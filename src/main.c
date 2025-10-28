#include <stdio.h>
#include <stdlib.h>
#include "./../include/compiler.h"

size_t GetFileLength(char* path) {
    FILE* fp = fopen(path, "r");

    ASSERT(fp != NULL, "Failed to open file: '%s'", path);

    fseek(fp, 0, SEEK_END);
    size_t file_length = ftell(fp) + 1; // For the "\0"
    rewind(fp);

    fclose(fp);

    return file_length;
}

void GetFileData(char* path, char* dest, size_t file_length) {
    FILE* fp = fopen(path, "r");

    ASSERT(fp != NULL, "Failed to open file: '%s'", path);

    fread(dest, file_length, 1, fp);

    fclose(fp);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return -1;
    }

    size_t file_length = GetFileLength(argv[1]);
    char source[file_length];
    GetFileData(argv[1], source, file_length);

    printf("%s\n", source);

    return 0;
}
