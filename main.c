#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define BUF_INIT_SIZE 128
#define ARR_INIT_SIZE 32

typedef struct {
	char value;
	int freq;
} OccuringCharacter;

typedef struct {
	OccuringCharacter** characters;
	size_t size;
	size_t capacity;
} ArrayOccurence;

void die(const char *fmt, ...);
OccuringCharacter* init_occ_char(char value, int freq);
ArrayOccurence* init_array_occurence(size_t capacity);
void free_array_occurence(ArrayOccurence* arr);
void add_occ(ArrayOccurence* arr, OccuringCharacter* occ);
char* dump_file(FILE* fp);
ArrayOccurence* count_freq(const char* input);
void _log_array_table(const ArrayOccurence* arr);

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

OccuringCharacter* init_occ_char(char value, int freq) {
	OccuringCharacter* occ = malloc(sizeof(*occ));
	occ->value = value;
	occ->freq = freq;
	return occ;
}

ArrayOccurence* init_array_occurence(size_t capacity) {
    ArrayOccurence* arr = malloc(sizeof(*arr));
    arr->characters = (OccuringCharacter**)malloc(capacity * sizeof(OccuringCharacter*));
    arr->size = 0;
    arr->capacity = capacity;
    return arr;
}

void free_array_occurence(ArrayOccurence* arr) {
    free(arr->characters);
    arr->capacity = arr->size = 0;
}

void add_occ(ArrayOccurence* arr, OccuringCharacter* occ) {
    if (arr->size >= arr->capacity) {
        arr->capacity *= 2;
        arr->characters = realloc(arr->characters, arr->capacity*sizeof(OccuringCharacter*));
    }
	if (occ) {
		arr->characters[arr->size] = occ;
		arr->size++;
	} else {
		die("Error: adding null OccuringCharacter to ArrayOccurence");
	}
}

ArrayOccurence* count_freq(const char* input) {
	int length = strlen(input);
	OccuringCharacter* occ = init_occ_char('\0', 0);
	ArrayOccurence* arr = init_array_occurence(ARR_INIT_SIZE);

	for (int i = 0; i < length; i++) {
		if (input[i] == occ->value) {
			occ->freq++;
		} else {
			occ->value = input[i];
			occ->freq++;
			add_occ(arr, occ);
		}
	}
	return arr;
}

char* dump_file(FILE* fp) {
	char c;
	size_t size = BUF_INIT_SIZE;
	char* buffer = malloc(size);

	while ((c = getc(fp)) != EOF) {
		if (size >= strlen(buffer)) {
			size *= 2;
			buffer = realloc(buffer, size);
		}
		strncat(buffer, (char[2]){c, '\0'}, BUF_INIT_SIZE);
	}

	return buffer;
}

void _log_array_table(const ArrayOccurence* arr) {
	for (size_t i = 0; i < arr->size; i++) {
		fprintf(stdout, "%c: %d", arr->characters[i]->value, arr->characters[i]->freq);
	}
	fprintf(stdout, "\n");
}

int main(int argc, char** argv) {
	if (argc == 1) {
		die("which file?");
	}

	FILE* fp;
	if (access(argv[1], F_OK) == 0) {
		fp = fopen(argv[1], "r");
	} else {
		die("error: file not found");
	}

	char* buffer = dump_file(fp);
	ArrayOccurence* arr = count_freq(buffer);
	free(buffer);
	_log_array_table(arr);
	free_array_occurence(arr);

	return 0;
}
