// ls.h
#ifndef LS_H
#define LS_H

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

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
        perror("ERROR: Failed to open directory");
    }
}

#endif // LS_H