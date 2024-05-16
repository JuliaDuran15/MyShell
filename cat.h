// cat.c
#ifndef CAT_H
#define CAT_H
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

void fake_cat(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: cat <file> [> outfile]\n");
        return;
    }

    // Determine if there is redirection
    int redirect = 0;
    char *output_file = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 && (i + 1) < argc) {
            redirect = 1;
            output_file = argv[i + 1];
            break;
        }
    }

    const char *filepath = argv[1];
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror("ERROR: File opening failed");
        return;
    }

    // Configure redirection, if necessary
    int original_stdout = dup(STDOUT_FILENO);
    if (redirect) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("ERROR: Failed to open output file");
            fclose(file);
            return;
        }
        dup2(fd, STDOUT_FILENO);
        close(fd);
    }

    char buffer[1024];
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
// Path: MyShell/ls.h