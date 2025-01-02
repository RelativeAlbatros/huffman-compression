#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>

#define BUF_INIT_SIZE 128
#define OCCURENCES_SIZE 256

void die(const char *fmt, ...);
char* dump_file(FILE* fp);

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

void _log_occurences(int* occurences, int size) {
	for (int i = 0; i < size; i++) {
		if (occurences[i] != 0) {
			printf("%c:%d\n", i, occurences[i]);
		}
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
	}

	_log_occurences(occurences, OCCURENCES_SIZE);
	free(occurences);
	free(buffer);
	return 0;
}
