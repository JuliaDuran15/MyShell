#ifndef CAT_H
#define CAT_H

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\033[0m"

void clone_cat(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, "Usage: cat <file> [> outfile]\n");
        return;
    }

    // Determine if there is redirection
    int redirect = 0;
    char *output_file = NULL;
    for (int i = 1; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0 && (i + 1) < MAX_BUFFER_SIZE) {
            redirect = 1;
            output_file = args[i + 1];
            break;
        }
    }

    const char *filepath = args[1];
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror(ANSI_COLOR_RED "ERROR: " ANSI_COLOR_RESET " File opening failed");
        return;
    }

    // Configure redirection, if necessary
    int original_stdout = dup(STDOUT_FILENO);
    if (redirect) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror(ANSI_COLOR_RED "ERROR: " ANSI_COLOR_RESET "Failed to open output file");
            fclose(file);
            return;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    char buffer[MAX_BUFFER_SIZE];
    while (fgets(buffer, sizeof(buffer), file)) {
        printf("%s", buffer);
    }

    fclose(file);

    // Restore standard output, if necessary
    if (redirect) {
        dup2(original_stdout, STDOUT_FILENO);
    }
    close(original_stdout);
}

#endif // CAT_H
