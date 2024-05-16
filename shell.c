#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "cat.h"
#include "ls.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_COLOR_BLUE "\x1B[01;36m"

#define MAX_LENGTH 1024  // Definindo o comprimento máximo para os comandos
#define DELIMS " \t\r\n" // Delimitadores para separar os tokens dos comandos

char **search_paths = NULL; // Vetor para armazenar os caminhos de busca de executáveis
int num_paths = 0;           // Contador para o número de caminhos registrados

void cd(char *path) {
    if (chdir(path) != 0) {
        perror("ERRROR: cd failed");
    }
}

void append_path(char *path) {
    if (num_paths == 0) {
        search_paths = malloc(sizeof(char *));
    } else {
        search_paths = realloc(search_paths, (num_paths + 1) * sizeof(char *));
    }

    if (!search_paths) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    if (num_paths > 0) {
        char *new_path = malloc(strlen(path) + 2); // Allocate space for ":" and null terminator
        if (!new_path) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        sprintf(new_path, "%s:", path);
        search_paths[num_paths] = new_path;
    } else {
        search_paths[num_paths] = strdup(path);
    }

    if (!search_paths[num_paths]) {
        fprintf(stderr, "Memory allocation failed\n");
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
        printf("%s", search_paths[i]);
    }
    printf("\n");
}

void process_command(char *cmd) {
    char **args = NULL;
    int argc = 0;
    char *token = strtok(cmd, DELIMS);

    args = malloc(MAX_LENGTH * sizeof(char *));
    if (!args) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    while (token != NULL) {
        args[argc] = strdup(token);
        if (!args[argc]) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        argc++;
        token = strtok(NULL, DELIMS);
    }
    args[argc] = NULL;

    if (args[0] == NULL) {
        free(args);
        return;
    }

    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (argc > 1) {
            cd(args[1]);
        } else {
            fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET " cd: argument required\n");
        }
    } else if (strcmp(args[0], "path") == 0) {
        if (argc == 1) {
            print_path(); // Se nenhum argumento for fornecido, imprime os caminhos
        } else {
            for (int i = 1; i < argc; i++) {
                append_path(args[i]);
            }
        }
    } else if (strcmp(args[0], "cat") == 0) {
        if (argc > 1) {
            fake_cat(argc, args);
        } else {
            fprintf(stderr, ANSI_COLOR_RED "ERR - " ANSI_COLOR_RESET " cat: argument required\n");
        }
    } else if (strcmp(args[0], "ls") == 0) {
        fake_ls(argc, args);
    } else {
        fprintf(stderr, ANSI_COLOR_RED "ERROR " ANSI_COLOR_RESET ": Unsupported command: %s\n", args[0]);
    }

    for (int i = 0; i < argc; i++) {
        free(args[i]);
    }
    free(args);
}

void execute_commands_concurrently(char *cmd_line) {
    char **commands = NULL;
    int n_commands = 0;
    char *token = strtok(cmd_line, "&");

    commands = malloc(MAX_LENGTH * sizeof(char *));
    if (!commands) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    while (token != NULL) {
        commands[n_commands] = strdup(token);
        if (!commands[n_commands]) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        n_commands++;
        token = strtok(NULL, "&");
    }

    for (int i = 0; i < n_commands; i++) {
        process_command(commands[i]);
        free(commands[i]);  // Liberar a memória alocada com strdup
    }

    free(commands);
}

int main() {
    char line[MAX_LENGTH];
    char *initialPath = "/bin:"; // /usr/bin:/usr/local/bin

    append_path(initialPath);

    while (1) {
        printf(ANSI_COLOR_BLUE "myshell> " ANSI_COLOR_RESET);
        if (!fgets(line, sizeof(line), stdin)) break;
        
        pid_t pid = fork();
        if (pid < 0) {
            fprintf(stderr, "Failed to fork\n");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // Processo filho
            if (strchr(line, '&') != NULL) {
                // Executa comandos em paralelo se houver '&' na linha de comando
                execute_commands_concurrently(line);
            } else {
                process_command(line);
            }
            exit(EXIT_SUCCESS);
        } else {
            // Processo pai
            waitpid(pid, NULL, 0);
        }
    }

    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }
    free(search_paths);

    return 0;
}
