#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "cat.h"
#include "ls.h"

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_RESET "\033[0m"
#define ANSI_COLOR_BLUE "\x1B[01;36m"

#define MAX_LENGTH 1024  // Definindo o comprimento máximo para os comandos
#define MAX_PATHS 64     // Número máximo de caminhos no vetor de busca
#define DELIMS " \t\r\n" // Delimitadores para separar os tokens dos comandos

char **search_paths; // Vetor para armazenar os caminhos de busca de executáveis
int num_paths = 0;   // Contador para o número de caminhos registrados
volatile sig_atomic_t exit_flag = 0;

void cd(char *path) {
    if (chdir(path) != 0) {
        perror("ERROR: cd failed");
    }
}

void append_path(char *path) {
    if (num_paths < MAX_PATHS) {
        if (num_paths > 0) {
            strcat(search_paths[num_paths - 1], ":");
        }
        search_paths[num_paths++] = strdup(path);
    } else {
        fprintf(stderr, "Limite máximo de caminhos alcançado. Não é possível adicionar mais caminhos.\n");
    }
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
    char **args = malloc(MAX_PATHS * sizeof(char *));
    int argc = 0;
    char *token = strtok(cmd, DELIMS);

    while (token != NULL && argc < MAX_PATHS) {
        args[argc++] = strdup(token);
        token = strtok(NULL, DELIMS);
    }
    args[argc] = NULL;

    if (args[0] == NULL) return; // Comando vazio
    if (strcmp(args[0], "exit") == 0) {
        exit_flag = 1; // set exit flag
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
    char **commands = malloc(MAX_PATHS * sizeof(char *));
    int n_commands = 0;
    char *token = strtok(cmd_line, "&");

    while (token != NULL && n_commands < MAX_PATHS) {
        commands[n_commands++] = strdup(token);
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
    char *initialPath = "/bin";

    search_paths = malloc(MAX_PATHS * sizeof(char *));
    search_paths[0] = strdup(initialPath);
    num_paths = 1;

    while (!exit_flag) {
        printf(ANSI_COLOR_BLUE "myshell> " ANSI_COLOR_RESET);
        if (!fgets(line, sizeof(line), stdin)) break;

        pid_t pid;
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(1);
        }

        if (pid == 0) { // child process
            process_command(line);
            exit(0);
        } else { // parent process
            // wait for the child process to finish
            wait(NULL);
        }
    }
    // Não roda comandos em paralelo, fora do while

    // if (strchr(line, '&') != NULL) {
    //     // Executa comandos em paralelo se houver '&' na linha de comando
    //     execute_commands_concurrently(line);
    // }

    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }
    free(search_paths);
    
    return 0;
}
