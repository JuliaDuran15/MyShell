#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_LENGTH 1024  // Definindo o comprimento máximo para os comandos
#define MAX_PATHS 64     // Número máximo de caminhos no vetor de busca
#define DELIMS " \t\r\n" // Delimitadores para separar os tokens dos comandos

char *search_paths[MAX_PATHS]; // Vetor para armazenar os caminhos de busca de executáveis
int num_paths = 0;             // Contador para o número de caminhos registrados

void cd(char *path) {
    if (chdir(path) != 0) {
        perror("cd failed");
    }
}

void set_path(char *paths[], int count) {
    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }
    num_paths = count;
    for (int i = 0; i < count; i++) {
        search_paths[i] = strdup(paths[i]);
    }
}

// Implementação simplificada do comando 'cat'
void fake_cat(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: cat <file> [> outfile]\n");
        return;
    }

    // Determinar se há redirecionamento
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
        perror("File opening failed");
        return;
    }

    // Configurar redirecionamento, se necessário
    int original_stdout = dup(STDOUT_FILENO);
    if (redirect) {
        int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1) {
            perror("Failed to open output file");
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

    // Restaurar saída padrão, se necessário
    if (redirect) {
        dup2(original_stdout, STDOUT_FILENO);
    }
    close(original_stdout);
}
// Implementação simplificada do comando 'ls'
void fake_ls(int argc, char *argv[]) {
    DIR *d;
    struct dirent *dir;
    d = opendir(".");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (argc == 1 || (argc > 1 && strcmp(argv[1], "-a") == 0) || dir->d_name[0] != '.') {
                if (argc > 1 && strcmp(argv[1], "-l") == 0) {
                    struct stat st;
                    stat(dir->d_name, &st);
                    printf("%ld %s\n", st.st_size, dir->d_name);
                } else {
                    printf("%s\n", dir->d_name);
                }
            }
        }
        closedir(d);
    } else {
        perror("Failed to open directory");
    }
}

void process_command(char *cmd) {
    char *args[MAX_PATHS];
    int argc = 0;
    char *token = strtok(cmd, DELIMS);

    while (token != NULL && argc < MAX_PATHS) {
        args[argc++] = token;
        token = strtok(NULL, DELIMS);
    }
    args[argc] = NULL;

    if (args[0] == NULL) return; // Comando vazio
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (argc > 1) {
            cd(args[1]);
        } else {
            fprintf(stderr, "cd: argument required\n");
        }
    } else if (strcmp(args[0], "path") == 0) {
        set_path(&args[1], argc - 1);
    } else if (strcmp(args[0], "cat") == 0 && argc > 1) {
        fake_cat(argc, args);
    } else if (strcmp(args[0], "ls") == 0) {
        fake_ls(argc, args);
    } else {
        fprintf(stderr, "Unsupported command: %s\n", args[0]);
    }
}

int main() {
    char line[MAX_LENGTH];
    char *initialPath = "/bin";

    search_paths[0] = strdup(initialPath);
    num_paths = 1;

    while (1) {
        printf("myshell> ");
        if (!fgets(line, sizeof(line), stdin)) break;
        process_command(line);
    }

    for (int i = 0; i < num_paths; i++) {
        free(search_paths[i]);
    }

    return 0;
}