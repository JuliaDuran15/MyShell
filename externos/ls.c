#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int long_format = 0;
    int all_files = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            long_format = 1;
        } else if (strcmp(argv[i], "-a") == 0) {
            all_files = 1;
        }
    }

    DIR *dir = opendir(".");
    if (dir == NULL) {
        perror("Failed to open directory");
        return 1;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (!all_files && entry->d_name[0] == '.') {
            continue;
        }

        struct stat fileStat;
        if (stat(entry->d_name, &fileStat) != -1) {
            if (long_format) {
                // Print type and permissions
                printf((S_ISDIR(fileStat.st_mode)) ? "d" : "-");
                printf((fileStat.st_mode & S_IRUSR) ? "r" : "-");
                printf((fileStat.st_mode & S_IWUSR) ? "w" : "-");
                printf((fileStat.st_mode & S_IXUSR) ? "x" : "-");
                printf((fileStat.st_mode & S_IRGRP) ? "r" : "-");
                printf((fileStat.st_mode & S_IWGRP) ? "w" : "-");
                printf((fileStat.st_mode & S_IXGRP) ? "x" : "-");
                printf((fileStat.st_mode & S_IROTH) ? "r" : "-");
                printf((fileStat.st_mode & S_IWOTH) ? "w" : "-");
                printf((fileStat.st_mode & S_IXOTH) ? "x" : "-");

                // Print number
            // Print user and group names
            struct passwd *pwd = getpwuid(fileStat.st_uid);
            struct group *grp = getgrgid(fileStat.st_gid);
            printf(" %s %s", pwd->pw_name, grp->gr_name);

            // Print file size
            printf(" %8ld", (long)fileStat.st_size);

            // Format and print modification time
            char timebuf[80];
            strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", localtime(&fileStat.st_mtime));
            printf(" %s", timebuf);

            // Print file name with color
            printf("\033[1;32m %s\033[0m\n", entry->d_name);
        } else {
            printf("%s  ", entry->d_name);
        }
    } else {
        perror("stat failed");
    }
}
printf("\n");

closedir(dir);
return 0;
}