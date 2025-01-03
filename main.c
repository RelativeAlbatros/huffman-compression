#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define BUF_INIT_SIZE 128
#define OCCURENCES_SIZE 128

typedef struct {
	char element;
	int freq;
} LeafNode;

typedef struct {
	LeafNode* leafnodes;
	int size;
	int capacity;
} HuffTree;

void die(const char *fmt, ...);
void count_freq(int* occurences, const char* buffer);
char get_max(int* occurences, int size);
HuffTree priority_sort(int* occurences, int size);
void _log_occurences(int* occurences);
void _log_tree(HuffTree* tree);

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

void count_freq(int* occurences, const char* buffer) {
	int length = strlen(buffer);

	for (int i = 0; i < length; i++) {
		occurences[(int)buffer[i]]++;
	}
}

char get_max(int* occurences, int size) {
	int largest = 0;

	for (int i = 0; i < size; i++) {
		if (occurences[i] > occurences[largest]) {
			largest = i;
		}
	}

	return (char)largest;
}

HuffTree priority_sort(int* occurences, int size) {
	LeafNode* array = malloc(size * sizeof(LeafNode));
	HuffTree tree = (HuffTree) {array, 0, size};

	for (int i = 0; i < size; i++) {
		char maxfreqchar = get_max(occurences, size);
		if (occurences[(int)maxfreqchar] == 0)
			return tree;

		tree.leafnodes[i] = (LeafNode) {maxfreqchar, occurences[(int)maxfreqchar]};
		tree.size++;
		occurences[(int)maxfreqchar] = 0; // remove char from occurences after add to array
	}

	return tree;
}

void _log_occurences(int* occurences) {
	for (int i = 0; i < OCCURENCES_SIZE; i++) {
		if (occurences[i] != 0) {
			printf("%c:%d\n", i, occurences[i]);
		}
	}
}

void _log_tree(HuffTree* tree) {
	for (int i = 0; i < tree->size; i++) {
		printf("%c:%d\n", tree->leafnodes[i].element, tree->leafnodes[i].freq);
	}
}

int main(int argc, char** argv) {
	int* occurences = calloc(OCCURENCES_SIZE*sizeof(int), sizeof(int));
	int buf_size = BUF_INIT_SIZE;
	char* buffer = malloc(buf_size);

	if (argc == 1) {
		die("which file?");
	}

	FILE* fp;
	if (access(argv[1], F_OK) == 0) {
		fp = fopen(argv[1], "r");
	} else {
		die("error: file not found");
	}

	while (fread(buffer, 1, buf_size, fp)) {
		count_freq(occurences, buffer);
		HuffTree tree = priority_sort(occurences, OCCURENCES_SIZE);
		_log_tree(&tree);
		free(tree.leafnodes);
	}

	free(occurences);
	free(buffer);
	return 0;
}
