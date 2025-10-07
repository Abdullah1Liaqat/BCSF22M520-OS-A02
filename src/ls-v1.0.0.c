/*
 * Programming Assignment 02: lsv1.0.0
 * Version 1.1.0 — Added -l long listing feature
 */

#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>

extern int errno;

void do_ls(const char *dir, int long_listing);
void print_file_info(const char *path, const char *filename);

int main(int argc, char const *argv[])
{
    int long_listing = 0;
    const char *dirs[20];
    int dir_count = 0;

    // Parse arguments
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-l") == 0)
            long_listing = 1;
        else
            dirs[dir_count++] = argv[i];
    }

    if (dir_count == 0)
        dirs[dir_count++] = ".";

    for (int i = 0; i < dir_count; i++)
    {
        if (dir_count > 1)
            printf("Directory listing of %s:\n", dirs[i]);
        do_ls(dirs[i], long_listing);
        if (dir_count > 1)
            puts("");
    }

    return 0;
}

void do_ls(const char *dir, int long_listing)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    errno = 0;
    while ((entry = readdir(dp)) != NULL)
    {
        if (entry->d_name[0] == '.')
            continue;

        if (long_listing)
            print_file_info(dir, entry->d_name);
        else
            printf("%s\n", entry->d_name);
    }

    if (errno != 0)
        perror("readdir failed");

    closedir(dp);
}

void print_file_info(const char *path, const char *filename)
{
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    struct stat info;
    if (stat(fullpath, &info) == -1)
    {
        perror("stat");
        return;
    }

    // File type & permissions
    printf( (S_ISDIR(info.st_mode)) ? "d" : "-");
    printf( (info.st_mode & S_IRUSR) ? "r" : "-");
    printf( (info.st_mode & S_IWUSR) ? "w" : "-");
    printf( (info.st_mode & S_IXUSR) ? "x" : "-");
    printf( (info.st_mode & S_IRGRP) ? "r" : "-");
    printf( (info.st_mode & S_IWGRP) ? "w" : "-");
    printf( (info.st_mode & S_IXGRP) ? "x" : "-");
    printf( (info.st_mode & S_IROTH) ? "r" : "-");
    printf( (info.st_mode & S_IWOTH) ? "w" : "-");
    printf( (info.st_mode & S_IXOTH) ? "x" : "-");

    // Number of links, owner, group, size, time, name
    struct passwd *pw = getpwuid(info.st_uid);
    struct group  *gr = getgrgid(info.st_gid);
    char *time_str = ctime(&info.st_mtime);
    time_str[strlen(time_str) - 1] = '\0'; // remove newline

    printf(" %2ld %s %s %6ld %s %s\n",
           (long)info.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?",
           (long)info.st_size,
           time_str,
           filename);
}

