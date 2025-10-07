/*
 * Programming Assignment 02: lsv2.1.0
 * Feature 3: Recursive (-R)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

extern int errno;

void do_ls(const char *dir, int recursive);
void list_dir(const char *dir, int recursive);

int main(int argc, char const *argv[])
{
    int recursive = 0;
    int start_index = 1;

    // check if -R flag present
    if (argc > 1 && strcmp(argv[1], "-R") == 0) {
        recursive = 1;
        start_index = 2;
    }

    if (argc == start_index) {
        list_dir(".", recursive);
    } else {
        for (int i = start_index; i < argc; i++) {
            list_dir(argv[i], recursive);
            if (i < argc - 1) puts("");
        }
    }
    return 0;
}

void list_dir(const char *dir, int recursive)
{
    printf("Directory listing of %s:\n", dir);
    do_ls(dir, recursive);
    puts("");
}

void do_ls(const char *dir, int recursive)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.')
            continue;
        printf("%s\n", entry->d_name);
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);

    // Recursive part
    if (recursive) {
        dp = opendir(dir);
        if (dp == NULL) return;
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.')
                continue;

            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

            struct stat st;
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)) {
                printf("\n%s:\n", path);
                do_ls(path, recursive);
            }
        }
        closedir(dp);
    }

}
