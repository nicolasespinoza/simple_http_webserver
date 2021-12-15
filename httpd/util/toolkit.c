#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include "toolkit.h"
#include "array_list.h"

struct arraylist* split(char* string, char* delimiter) {
    struct arraylist* split_words = array_list_new(sizeof(char*));

    char* string_copy = strdup(string); // duplicates string dynamically
    char* start = string_copy;
    char* found = NULL;
    while ((found = strsep(&string_copy, delimiter)) != NULL) {
        array_list_add_to_end(split_words, strdup(found)); // NOTE: found uses strdup here
    }
    free(start);
    free(found);

    return split_words;
}

int string_to_int(char* value, int should_be_positive) {
    char* value_copy = strdup(value);

    if (strcmp(value, "0") == 0) {
        return 0;
    }

    char* filler;
    long converted = strtol(value_copy, &filler, 10);

    if (converted == 0) {
        printf("[%s] is not a valid integer, exiting", value);
        exit(-1);
    }

    if (should_be_positive && converted <= 0) {
        printf("[%s] is not a valid positive integer, exiting", value);
        exit(-1);
    }

    free(value_copy);
    return converted;
}

int contains_double_dot(char* string) {
    char* result;
    result = strstr(string, "..");
    return result ? 1 : 0;
}

int get_file_size(char* file_name) {
    struct stat stats;
    if (stat(file_name, &stats) == 0) {
        return stats.st_size;
    }

    return -1;
}

void print_file_contents(FILE* file) {
    char* line_buffer = NULL;
    size_t line_buffer_size = 0;
    ssize_t line_size;

    line_size = getline(&line_buffer, &line_buffer_size, file); // first line of the file
    while (line_size >= 0) {
        printf("%s", line_buffer);
        line_size = getline(&line_buffer, &line_buffer_size, file); // next line
    }

    free(line_buffer);
}
