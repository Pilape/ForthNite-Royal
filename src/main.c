#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/compiler.h"
#include "../include/lexer.h"
#include "../include/codegen.h"

static char* ReadFileData(const char* path) {
    FILE* fp = fopen(path, "r");

    ASSERT_FORMAT(fp != NULL, "Failed to open file: '%s'", path);

    fseek(fp, 0, SEEK_END);
    size_t file_length = ftell(fp);
    rewind(fp);

    char* file_buffer = malloc((file_length+1)*sizeof(char));
    ASSERT_FORMAT(file_buffer != NULL, "Could not allocate memory for filebuffer when loading file: '%s'", path);

    fread(file_buffer, file_length, 1, fp);
    file_buffer[file_length] = '\0';

    fclose(fp);

    return file_buffer;
}

static void RemoveFileExtension(char* file) {
    char* end = file + strlen(file);

    while (end > file && *end != '.') {
        --end;
    }

    if (end > file) {
        *end = '\0';
    }
}

static void WriteOutputFile(const char* path, Rom* code) {
    char output_path[strlen(path)+5]; // + '.rom\0'
    strcpy(output_path, path);
    RemoveFileExtension(output_path);
    strcat(output_path, ".rom\0");

    FILE* fp = fopen(output_path, "w");

    ASSERT_FORMAT(fp != NULL, "Failed to create file: '%s'", path);

    for (int i=0; i<code->size; i++) {
        fprintf(fp, "%02x ", code->data[i]);
        if (i % 16 == 15) fprintf(fp, "\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Not enough arguments\n");
        return -1;
    }

    char* source = ReadFileData(argv[1]);
    TokenList tokens = Scan(source);
    free(source);
    Rom output_code = { 0 };
    GenerateCode(&tokens, &output_code);
    free(tokens.data);

    WriteOutputFile(argv[1], &output_code);

    return 0;
}
