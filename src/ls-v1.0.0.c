/*
 * Programming Assignment 02: lsv2.4.0
 * Features:
 *   - Normal listing
 *   - Recursive (-R)
 *   - Show hidden files (-a)
 *   - Alphabetical sorting (default)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

extern int errno;

void do_ls(const char *dir, int recursive, int show_all);
void list_dir(const char *dir, int recursive, int show_all);
int compare(const void *a, const void *b);

int main(int argc, char const *argv[])
{
    int recursive = 0;
    int show_all = 0;
    int start_index = 1;

    // Parse flags (-a, -R)
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'R')) recursive = 1;
            if (strchr(argv[i], 'a')) show_all = 1;
            start_index++;
        } else break;
    }

    // No paths -> current directory
    if (argc == 1 || start_index == argc) {
        list_dir(".", recursive, show_all);
    } else {
        for (int i = start_index; i < argc; i++) {
            list_dir(argv[i], recursive, show_all);
            if (i < argc - 1) puts("");
        }
    }

    return 0;
}

void list_dir(const char *dir, int recursive, int show_all)
{
    printf("Directory listing of %s:\n", dir);
    do_ls(dir, recursive, show_all);
    puts("");
}

int compare(const void *a, const void *b)
{
    const char **strA = (const char **)a;
    const char **strB = (const char **)b;
    return strcmp(*strA, *strB);
}

void do_ls(const char *dir, int recursive, int show_all)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    // Collect entries
    char *names[1024];
    int count = 0;
    errno = 0;

    while ((entry = readdir(dp)) != NULL) {
        if (!show_all && entry->d_name[0] == '.')
            continue;
        names[count] = strdup(entry->d_name);
        count++;
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);

    // Sort alphabetically
    qsort(names, count, sizeof(char *), compare);

    // Print sorted names
    for (int i = 0; i < count; i++) {
        printf("%s\n", names[i]);
        free(names[i]);
    }

    // Recursive part
    if (recursive) {
        dp = opendir(dir);
        if (dp == NULL) return;
        while ((entry = readdir(dp)) != NULL) {
            if (!show_all && entry->d_name[0] == '.')
                continue;

            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

            struct stat st;
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)
                && strcmp(entry->d_name, ".") != 0
                && strcmp(entry->d_name, "..") != 0) {
                printf("\n%s:\n", path);
                do_ls(path, recursive, show_all);
            }
        }
        closedir(dp);
    }
}
