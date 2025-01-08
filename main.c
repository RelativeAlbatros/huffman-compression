#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define BUF_INIT_SIZE 2048
#define OCC_SIZE 128
#define QUEU_INIT_SIZE 2048
#define TREE_INIT_SIZE 32

typedef struct Node {
	char element;
	int weight;
	struct Node* left;
	struct Node* right;
} Node;

typedef struct HuffTree {
	Node* root;
	int size;
} HuffTree;

typedef struct MinPriorityQueu {
	Node** elements;
	int size;
	int capacity;
} MinPriorityQueu;

void die(const char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);

	if (fmt[0] && fmt[strlen(fmt)-1] == ':') {
		fputc(' ', stderr);
		perror(NULL);
	} else {
		fputc('\n', stderr);
	}

	exit(1);
}

FILE* open_file(const char* file) {
	FILE* fp = NULL;

	if (access(file, F_OK) == 0) {
		fp = fopen(file, "r");
	} else {
		die("error: file not found");
	}

	return fp;
}

int* count_occurences(const char* input) {
	int length = strlen(input);
	int* occurences = malloc(OCC_SIZE);

	for (int i = 0; i < length; i++) {
		occurences[(int)input[i]]++;
	}

	return occurences;
}

void _log_occurences(int* occurences) {
	for (int i = 0; i < OCC_SIZE; i++) {
		if (occurences[i] != 0) {
			printf("%c:%d\n", i, occurences[i]);
		}
	}
}

int min_in_occ(int* occurences) {
	int min = 0;

	for (int i = 0; i < OCC_SIZE; i++) {
		if (occurences[i] < occurences[min]) {
			min = i;
		}
	}

	return min;
}

Node* init_node(int weight) {
	Node* node = malloc(sizeof(Node));
	node->element = '\0';
	node->weight = weight;
	node->right = node->left = NULL;

	return node;
}

Node* init_internal_node(Node* left, Node* right) {
	Node* internal_node = malloc(sizeof(Node));
	internal_node->element = '\0';
	internal_node->weight = left->weight + right->weight;
	internal_node->left = left;
	internal_node->right = right;

	return internal_node;
}

HuffTree* init_tree() {
	HuffTree* tree = malloc(sizeof(Node*) + sizeof(int));
	tree->size = 0;

	return tree;
}

MinPriorityQueu* init_queu() {
	MinPriorityQueu* queu = malloc(sizeof(Node*) + 2 * sizeof(int));
	queu->elements = malloc(QUEU_INIT_SIZE * sizeof(Node));
	queu->size = 0;
	queu->capacity = QUEU_INIT_SIZE;

	return queu;
}

MinPriorityQueu* add_occ_to_queu(MinPriorityQueu* queu, const char c, const int freq) {
	if (queu->size >= queu->capacity) {
		queu->size *= 2;
		queu = realloc(queu, queu->size);
	}

	queu->elements[queu->size] = init_node(freq);
	queu->elements[queu->size]->element = c;
	queu->size++;

	return queu;
}

MinPriorityQueu* priority_sort(int* occurences) {
	MinPriorityQueu* queu = init_queu();

	int min_in_queu = 0;
	while ((min_in_queu = min_in_occ(occurences)) != 0) {
		add_occ_to_queu(queu, (char)min_in_queu, occurences[min_in_queu]);
	}

	return queu;
}

void remove_node_in_queu(MinPriorityQueu* queu, int i) {
	free(queu->elements[i]);
	for (int j = i; j < (queu->size - 1); j++) {
		queu->elements[j] = queu->elements[j+1];
	}
	// remove last element
	queu->elements[queu->size] = NULL;
	queu->size--;
}

void rearrange_queu(MinPriorityQueu* queu) {
	for (int i = 0; i < queu->size; i++) {
		if (queu->elements[i]->weight > queu->elements[i+1]->weight) {
			Node* tmp = queu->elements[i];
			queu->elements[i] = queu->elements[i+1];
			queu->elements[i+1] = tmp;
		}
	}
}

Node* get_internal_node_from_queu(MinPriorityQueu* queu) {
	Node* internal_node = init_internal_node(queu->elements[0], queu->elements[1]);
	remove_node_in_queu(queu, 0);
	free(queu->elements[0]);
	queu->elements[0] = internal_node;
	rearrange_queu(queu);

	return internal_node;
}

HuffTree* build_huff_tree(MinPriorityQueu* queu) {
	HuffTree* tree = init_tree();
	Node* internal_node;

	while (queu->size > 1) {
		internal_node = get_internal_node_from_queu(queu);
	}

	tree->root = init_node(queu->elements[0]->weight + internal_node->weight);
	tree->root->left = queu->elements[0];
	tree->root->right = internal_node;
	tree->size = get_size(tree);

	return tree;
}

// left of ith element is 2 * i + 1
// right of ith element is 2 * i + 2
// TODO:
Node* node_from_tree(HuffTree* tree, int i) {
	return tree->root->right;
}

void _log_tree(HuffTree* tree) {
	Node* current_node;
	for (int i = tree->size - 1; i >= 0; i++) {
		current_node = node_from_tree(tree, i);
		printf("internal node %d:%d\n", i, current_node->weight);
		printf("with children left (%c:%d) and right (%c:%d)\n", 
				current_node->left->element, current_node->left->weight,
				current_node->right->element, current_node->right->weight);
	}
}

void free_queu(MinPriorityQueu* queu) {
	for (int i = 0; i < queu->size; i++) {
		free(queu->elements[i]);
	}
	queu->size = 0;
	queu->capacity = 0;
	free(queu);
}

void free_tree(HuffTree* tree) {
	free(tree->root);
	tree->size = 0;
	free(tree);
}

int main(int argc, char** argv) {
	if (argc == 1) {
		die("which file?");
	}

	FILE* fp = open_file(argv[1]);
	char* buffer = calloc(BUF_INIT_SIZE * sizeof(char), sizeof(char));
	MinPriorityQueu* queu;
	HuffTree* tree;
	int* occurences;

	while (fread(buffer, 1, BUF_INIT_SIZE, fp) > 0) {
		occurences = count_occurences(buffer);
		queu = priority_sort(occurences);
		tree = build_huff_tree(queu);
		_log_tree(tree);
	}

	fclose(fp);
	free(occurences);
	free(buffer);
	free_queu(queu);
	free_tree(tree);

	return 0;
}
