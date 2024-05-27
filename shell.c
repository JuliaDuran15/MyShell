#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <dirent.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_ARGS 100

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_COLOR_BLUE "\x1B[01;36m"

char **search_paths = NULL;
int num_paths = 0;

void execute_command(char *cmd, char **args);
void handle_internal_commands(char *cmd, char **args);
void handle_cd(char **args);
void handle_path(char **args);
void handle_exit();
void redirect_output(char **args);

void handle_clear();

void append_path(char *path);
void print_path();


int main(int argc, char *argv[]) {
    char input[MAX_BUFFER_SIZE];
    char *args[MAX_ARGS];
    char *token;
    int interactive = 1;
    FILE *input_file = stdin; // Default input is standard input

    // Check for a batch file argument
    if (argc > 1) {
        input_file = fopen(argv[1], "r");
        if (!input_file) {
            fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "Could not open file: %s\n", argv[1]);
            exit(EXIT_FAILURE);
        }
        interactive = 0;
    }


    while (1) {
        if (interactive) {
            printf(ANSI_COLOR_BLUE "myshell> " ANSI_COLOR_RESET);
            fflush(stdout);
        }

        if (fgets(input, MAX_BUFFER_SIZE, input_file) == NULL) {
            break;
        }

        // Remove newline character from input
        input[strcspn(input, "\n")] = 0;

        // Tokenize input
        int i = 0;
        token = strtok(input, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL;

        if (args[0] == NULL) {
            continue;
        }

        handle_internal_commands(args[0], args);
    }

    if (!interactive) {
        fclose(input_file);
    }

    return 0;
}

void handle_clear() {
    #ifdef _WIN32
    system("cls");
    #else
    system("clear");
    #endif
}

void handle_internal_commands(char *cmd, char **args) {
    if (strcmp(cmd, "exit") == 0) {
        handle_exit();
    } else if (strcmp(cmd, "cd") == 0) {
        handle_cd(args);
    } else if (strcmp(cmd, "path") == 0) {
        handle_path(args);
    }  else if (strcmp(cmd, "clear") == 0) {
        handle_clear();
    } else {
        execute_command(cmd, args);
    }
}

void handle_exit() {
    exit(0);
}

void handle_cd(char **args) {
    if (args[1] == NULL) {
        fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "cd: missing argument\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("cd");
        }
    }
}


void handle_path(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        append_path(args[i]);
    }
    print_path();
}

void append_path(char *path) {
    if (num_paths == 0) {
        search_paths = malloc(sizeof(char *));
    } else {
        search_paths = realloc(search_paths, (num_paths + 1) * sizeof(char *));
    }

    if (!search_paths) {
        fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    if (num_paths > 0) {
        char *new_path = malloc(strlen(path) + 2); // Allocate space for ":" and null terminator
        if (!new_path) {
            fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        sprintf(new_path, "%s", path);
        search_paths[num_paths] = new_path;
    } else {
        search_paths[num_paths] = strdup(path);
    }

    if (!search_paths[num_paths]) {
        fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    num_paths++;
}

void print_path() {
    printf("Caminhos de busca de executáveis:\n");
    for (int i = 0; i < num_paths; i++) {
        printf("%d: %s\n", i + 1, search_paths[i]);
    }

    printf("Caminho completo de busca de executáveis: ");
    for (int i = 0; i < num_paths; i++) {
        printf("%s ;", search_paths[i]);
    }
    printf("\n");
}


void execute_command(char *cmd, char **args) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        int executed = 0;
        int fd = -1; // File descriptor for redirection
        int i;

        // Check for output redirection
        for (i = 0; args[i] != NULL; i++) {
            if (strcmp(args[i], ">") == 0) {
                if (args[i + 1] != NULL) {
                    fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                    if (fd == -1) {
                        perror("open");
                        exit(EXIT_FAILURE);
                    }
                    // Redirect stdout to the file
                    if (dup2(fd, STDOUT_FILENO) == -1) {
                        perror("dup2");
                        exit(EXIT_FAILURE);
                    }
                    close(fd);

                    // Remove redirection operator and file name from arguments
                    args[i] = NULL;
                    break;
                } else {
                    fprintf(stderr, "Syntax error: expected file name after '>'\n");
                    exit(EXIT_FAILURE);
                }
            }
        }

        for (int j = 0; j < num_paths; j++) {
            char exec_path[MAX_BUFFER_SIZE];
            snprintf(exec_path, sizeof(exec_path), "%s/%s", search_paths[j], cmd);
            // Try to execute the command in the specified path
            execv(exec_path, args);
        }

        // If none of the paths succeeded, print an error
        if (!executed) {
            fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "Command not found: %s\n", cmd);
            exit(EXIT_FAILURE);
        }
    } else if (pid > 0) {
        // Parent process waits for the child to complete
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Return status: %d\n", WEXITSTATUS(status));
        }
    } else {
        // Fork failed
        perror("fork");
    }
}

// void redirect_output(char **args) {
//     int i;
//     for (i = 0; args[i] != NULL; i++) {
//         if (strcmp(args[i], ">") == 0) {
//             break;
//         }
//     }

//     if (args[i] == NULL || args[i + 1] == NULL) {
//         fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "Redirection syntax error\n");
//         return;
//     }

//     int fd = open(args[i + 1], O_CREAT | O_WRONLY | O_TRUNC, 0644);
//     if (fd == -1) {
//         perror(ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "open");
//         return;
//     }

//     args[i] = NULL; // Truncate args at redirection operator
//     pid_t pid = fork();
//     if (pid == 0) {
//         // Child process
//         dup2(fd, STDOUT_FILENO);
//         close(fd);
//         execvp(args[0], args);
//         perror(ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "execvp");
//         exit(1);
//     } else if (pid > 0) {
//         // Parent process
//         int status;
//         waitpid(pid, &status, 0);
//         close(fd);
//     } else {
//         // Fork failed
//         perror(ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET "fork");
//     }
// }
