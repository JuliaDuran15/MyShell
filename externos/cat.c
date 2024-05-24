#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define MAX_BUFFER_SIZE 1024
#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\033[0m"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <file> [> outfile]\n", argv[0]);
        return 1;
    }

    int redirect = 0;
    char *output_file = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ">") == 0 && (i + 1) < argc) {
            redirect = 1;
            output_file = argv[i + 1];
            argv[i] = NULL;  // Terminate argument list here for execv later
            break;
        }
    }

    const char *filepath = argv[1];
    FILE *file = fopen(filepath, "r");
    if (!file) {
        perror(ANSI_COLOR_RED "ERROR: " ANSI_COLOR_RESET "File opening failed");
        return 1;
    }

    // Configure redirection, if necessary
    int original_stdout = dup(STDOUT_FILENO);
    if (redirect) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror(ANSI_COLOR_RED "ERROR: " ANSI_COLOR_RESET "Failed to open output file");
            fclose(file);
            return 1;
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
        close(original_stdout);
    }

    return 0;
}
