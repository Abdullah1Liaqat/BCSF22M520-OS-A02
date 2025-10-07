/*
 * Programming Assignment 02: lsv2.6.0
 * Features:
 *   - Normal listing
 *   - Recursive (-R)
 *   - Show hidden files (-a)
 *   - Alphabetical sorting (default)
 *   - Sort by file size (-S)
 *   - Sort by modification time (-t)
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <time.h>

extern int errno;

typedef struct {
    char name[256];
    off_t size;
    time_t mtime;
} FileInfo;

void do_ls(const char *dir, int recursive, int show_all, int sort_by_size, int sort_by_time);
void list_dir(const char *dir, int recursive, int show_all, int sort_by_size, int sort_by_time);
int compare_alpha(const void *a, const void *b);
int compare_size(const void *a, const void *b);
int compare_time(const void *a, const void *b);

int main(int argc, char const *argv[])
{
    int recursive = 0;
    int show_all = 0;
    int sort_by_size = 0;
    int sort_by_time = 0;
    int start_index = 1;

    // Parse all flags
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (strchr(argv[i], 'R')) recursive = 1;
            if (strchr(argv[i], 'a')) show_all = 1;
            if (strchr(argv[i], 'S')) sort_by_size = 1;
            if (strchr(argv[i], 't')) sort_by_time = 1;
            start_index++;
        } else break;
    }

    if (argc == 1 || start_index == argc) {
        list_dir(".", recursive, show_all, sort_by_size, sort_by_time);
    } else {
        for (int i = start_index; i < argc; i++) {
            list_dir(argv[i], recursive, show_all, sort_by_size, sort_by_time);
            if (i < argc - 1) puts("");
        }
    }

    return 0;
}

void list_dir(const char *dir, int recursive, int show_all, int sort_by_size, int sort_by_time)
{
    printf("Directory listing of %s:\n", dir);
    do_ls(dir, recursive, show_all, sort_by_size, sort_by_time);
    puts("");
}

int compare_alpha(const void *a, const void *b)
{
    const FileInfo *fa = (const FileInfo *)a;
    const FileInfo *fb = (const FileInfo *)b;
    return strcmp(fa->name, fb->name);
}

int compare_size(const void *a, const void *b)
{
    const FileInfo *fa = (const FileInfo *)a;
    const FileInfo *fb = (const FileInfo *)b;
    if (fb->size > fa->size) return 1;
    else if (fb->size < fa->size) return -1;
    else return strcmp(fa->name, fb->name);
}

int compare_time(const void *a, const void *b)
{
    const FileInfo *fa = (const FileInfo *)a;
    const FileInfo *fb = (const FileInfo *)b;
    if (fb->mtime > fa->mtime) return 1;
    else if (fb->mtime < fa->mtime) return -1;
    else return strcmp(fa->name, fb->name);
}

void do_ls(const char *dir, int recursive, int show_all, int sort_by_size, int sort_by_time)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    FileInfo files[1024];
    int count = 0;
    errno = 0;

    while ((entry = readdir(dp)) != NULL) {
        if (!show_all && entry->d_name[0] == '.')
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dir, entry->d_name);

        struct stat st;
        if (stat(path, &st) == 0) {
            strncpy(files[count].name, entry->d_name, sizeof(files[count].name) - 1);
            files[count].size = st.st_size;
            files[count].mtime = st.st_mtime;
            count++;
        }
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);

    // Sort by time > size > name priority
    if (sort_by_time)
        qsort(files, count, sizeof(FileInfo), compare_time);
    else if (sort_by_size)
        qsort(files, count, sizeof(FileInfo), compare_size);
    else
        qsort(files, count, sizeof(FileInfo), compare_alpha);

    // Print
    for (int i = 0; i < count; i++) {
        struct tm *tm_info = localtime(&files[i].mtime);
        char timebuf[64];
        strftime(timebuf, sizeof(timebuf), "%b %d %H:%M", tm_info);
        printf("%-30s %8ld bytes   %s\n", files[i].name, (long)files[i].size, timebuf);
    }

    // Recursive
    if (recursive) {
        for (int i = 0; i < count; i++) {
            char path[1024];
            snprintf(path, sizeof(path), "%s/%s", dir, files[i].name);

            struct stat st;
            if (stat(path, &st) == 0 && S_ISDIR(st.st_mode)
                && strcmp(files[i].name, ".") != 0
                && strcmp(files[i].name, "..") != 0
                && strcmp(files[i].name, ".git") != 0) {
                printf("\n%s:\n", path);
                do_ls(path, recursive, show_all, sort_by_size, sort_by_time);
            }
        }
    }
}
