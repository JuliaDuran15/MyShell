#ifndef LS_H
#define LS_H

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

void clone_ls(char **args) {
    int long_format = 0;
    int all_files = 0;

    for (int i = 1; args[i] != NULL; i++) {
        if (strcmp(args[i], "-l") == 0) {
            long_format = 1;
        } else if (strcmp(args[i], "-a") == 0) {
            all_files = 1;
        }
    }

    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("ls");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!all_files && entry->d_name[0] == '.') {
            continue;
        }

        if (long_format) {
            struct stat st;
            stat(entry->d_name, &st);
            printf("%ld %s\n", st.st_size, entry->d_name); // Simplificado, pode-se adicionar mais detalhes
        } else {
            printf("%s  ", entry->d_name);
        }
    }
    printf("\n");

    closedir(dir);
}

#endif // LS_H
