#include <stdio.h>
#include <stdlib.h>
#include "./../include/compiler.h"

char* ReadFileData(char* path) {
    FILE* fp = fopen(path, "r");

    ASSERT(fp != NULL, "Failed to open file: '%s'", path);

    fseek(fp, 0, SEEK_END);
    size_t file_length = ftell(fp);
    rewind(fp);

    char* file_buffer = malloc((file_length+1)*sizeof(char));
    fread(file_buffer, file_length, 1, fp);
    file_buffer[file_length] = '\0';

    fclose(fp);

    return file_buffer;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return -1;
    }

    char* source = ReadFileData(argv[1]);

    printf("%s\n", source);

    free(source);

    return 0;
}
