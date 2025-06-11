#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <limits.h>

#define VERSION "0.1"
#define HELP_MESSAGE \
	"hcom " VERSION  \
	"	-h	--help:  shows this message" \
	"	-d	--debug: debug mode"

#define DEBUG_FLAG 1

#define IS_DEBUG(flags) (flags & DEBUG_FLAG)

#define BUF_INIT_SIZE 2048
#define OCC_SIZE CHAR_MAX
#define QUEUE_INIT_SIZE 2048
#define TREE_INIT_SIZE 32

typedef struct Node {
	char element;
	int weight;
	struct Node* left;
	struct Node* right;
} Node;

typedef struct InputData {
	int flags;
	char* filename;
} InputData;

typedef struct HuffTree {
	Node* root;
	int size;
} HuffTree;

typedef struct MinPriorityQueue {
	Node** elements;
	int size;
	int capacity;
} MinPriorityQueue;

void die(const char *fmt, ...);
FILE* open_file(const char* file);
int* count_occurences(const char* input);
int min_in_occ(int* occurences);
void _log_occurences(int* occurences);
Node* init_node(char element, int weight);
Node* init_internal_node(Node* left, Node* right);
MinPriorityQueue* init_queue();
MinPriorityQueue* add_occ_to_queue(
 MinPriorityQueue* queue, const char c, const int freq);
MinPriorityQueue* priority_sort(int* occurences);
void remove_node_in_queue(MinPriorityQueue* queue, int i);
void rearrange_queue(MinPriorityQueue* queue);
void min_heap_insert(MinPriorityQueue* queue, Node* node);
Node* get_internal_node_from_queue(MinPriorityQueue* queue);
HuffTree* init_tree();
int get_size_tree(HuffTree* tree);
HuffTree* build_huff_tree(MinPriorityQueue* queue);
Node* node_from_tree(HuffTree* tree, int i);
void _log_tree(HuffTree* tree);
void free_node(Node* node);
void free_queue(MinPriorityQueue* queue);
void free_tree(HuffTree* tree);
void process_input(int argc, char** argv, InputData* input);

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
	if (fp == NULL) {
		die("error: opening file");
	}

	return fp;
}

void _log_occurences(int* occurences) {
	for (int i = 0; i < OCC_SIZE; i++) {
		if (occurences[i] != 0) {
			printf("%c:%d\n", i, occurences[i]);
		}
	}
}

int* count_occurences(const char* input) {

	int length = strlen(input);
	int* occurences = calloc(OCC_SIZE, sizeof(int));

	for (int i = 0; i < length; i++) {
		occurences[(int)input[i]]++;
	}

	return occurences;
}

int min_in_occ(int* occurences) {
	int min = 0;

	// look for first character with occurence
	for (int i = 0; i < OCC_SIZE; i++) {
		if (occurences[i] > 0) {
			min = i;
			break;
		}
	}
	// occurences is empty
	if (min == 0) {
		return -1;
	}
	for (int i = min; i < OCC_SIZE; i++) {
		if ((occurences[i] < occurences[min]) && (occurences[i] > 0)) {
			min = i;
		}
	}

	return min;
}

Node* init_node(char element, int weight) {
	Node* node = malloc(sizeof(Node));
	node->element = element;
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

MinPriorityQueue* init_queue() {
	MinPriorityQueue* queue = malloc(sizeof(MinPriorityQueue));
	queue->elements = malloc(QUEUE_INIT_SIZE * sizeof(Node*));
	queue->size = 0;
	queue->capacity = QUEUE_INIT_SIZE;

	return queue;
}

MinPriorityQueue* add_occ_to_queue(MinPriorityQueue* queue, const char c, const int freq) {
	if (queue->size >= queue->capacity) {
		queue->capacity *= 2;
		queue->elements = realloc(queue->elements, queue->capacity * sizeof(Node*));
		if (!queue->elements) die("queue reallocation failed.");
	}

	queue->elements[queue->size] = init_node(c, freq);
	queue->size++;

	return queue;
}

MinPriorityQueue* priority_sort(int* occurences) {
	MinPriorityQueue* queue = init_queue();

	int min_in_queue = 0;
	while ((min_in_queue = min_in_occ(occurences)) > 0) {
		add_occ_to_queue(queue, (char)min_in_queue, occurences[min_in_queue]);
		occurences[min_in_queue] = 0;
	}

	return queue;
}

void remove_node_in_queue(MinPriorityQueue* queue, int i) {
	free(queue->elements[i]);
	for (int j = i; j < (queue->size - 1); j++) {
		queue->elements[j] = queue->elements[j+1];
	}
	// remove last element
	queue->elements[queue->size] = NULL;
	queue->size--;
}

void rearrange_queue(MinPriorityQueue* queue) {
	for (int i = 0; i < queue->size-1; i++) {
		if (queue->elements[i]->weight > queue->elements[i+1]->weight) {
			Node* tmp = queue->elements[i];
			queue->elements[i] = queue->elements[i+1];
			queue->elements[i+1] = tmp;
		}
	}
}

void min_heap_insert(MinPriorityQueue* queue, Node* node) {
    int i = queue->size++;
    queue->elements[i] = node;
    while (i > 0 && queue->elements[(i-1)/2]->weight > queue->elements[i]->weight) {
        Node* tmp = queue->elements[i];
        queue->elements[i] = queue->elements[(i-1)/2];
        queue->elements[(i-1)/2] = tmp;
        i = (i-1)/2;
    }
}

Node* get_internal_node_from_queue(MinPriorityQueue* queue) {
	Node* internal_node = init_internal_node(queue->elements[0], queue->elements[1]);
	remove_node_in_queue(queue, 0);
	free(queue->elements[0]);
	queue->elements[0] = internal_node;
	rearrange_queue(queue);

	return internal_node;
}

HuffTree* init_tree() {
	HuffTree* tree = calloc(1, sizeof(HuffTree));
	tree->size = 0;

	return tree;
}

int get_size_tree(HuffTree* tree) {
	return 0;
}

HuffTree* build_huff_tree(MinPriorityQueue* queue) {
	HuffTree* tree = init_tree();
	Node* internal_node;

	while (queue->size > 1) {
		internal_node = get_internal_node_from_queue(queue);
	}

	tree->root = init_node('\0', queue->elements[0]->weight + internal_node->weight);
	tree->root->left = queue->elements[0];
	tree->root->right = internal_node;
	tree->size = get_size_tree(tree);

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
		if (current_node->left != NULL) {
			printf("with children left (%c:%d)",
				current_node->left->element, current_node->left->weight);
		} if (current_node->right != NULL) {
			printf("and right (%c:%d)\n", 
				current_node->right->element, current_node->right->weight);
		}
	}
}

void free_node(Node* node) {
	if (node == NULL) return;
	node->element = '\0';
	node->weight = 0;
	if (node->right != NULL) {
		free_node(node->right);
	}
	if (node->left != NULL) {
		free_node(node->left);
	}
	free(node);
}

void free_queue(MinPriorityQueue* queue) {
	for (int i = 0; i < queue->size; i++) {
		free(queue->elements[i]);
	}
	queue->size = 0;
	queue->capacity = 0;
	free(queue);
}

void free_tree(HuffTree* tree) {
	free_node(tree->root);
	tree->size = 0;
	free(tree);
}

void process_input(int argc, char** argv, InputData* input) {
	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
			input->flags = input->flags || DEBUG_FLAG;
		} else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			die(HELP_MESSAGE);
		} else {
			input->filename = argv[i];
		}
	}
}

int main(int argc, char** argv) {
	InputData* input_data = malloc(sizeof(InputData));
	input_data->flags = 0;
	input_data->filename = "";

	if (argc >= 2) {
		process_input(argc, argv, input_data);
		if (IS_DEBUG(input_data->flags)) {
			printf("%s", input_data->filename);
		}
	}
	if (argc == 1 || strlen(input_data->filename) == 0) {
		die("which file?");
	}

	FILE* fp = open_file(input_data->filename);
	char* buffer = calloc(BUF_INIT_SIZE, sizeof(char));
	MinPriorityQueue* queue;
	HuffTree* tree;
	int* occurences;

	while (fread(buffer, 1, BUF_INIT_SIZE, fp) > 0) {
		occurences = count_occurences(buffer);
		if (IS_DEBUG(input_data->flags)) _log_occurences(occurences);
		queue = priority_sort(occurences);
		tree = build_huff_tree(queue);
		_log_tree(tree);
	}

	fclose(fp);
	free(occurences);
	free(buffer);
	free_queue(queue);
	free_tree(tree);

	return 0;
}
