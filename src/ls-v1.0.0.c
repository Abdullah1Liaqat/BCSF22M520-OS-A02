/*
 * Programming Assignment 02: lsv2.0.0
 * Features implemented:
 *   1. Default column layout (down then across)
 *   2. -l long listing format
 * 
 * Compile: make
 * Run:     ./bin/ls
 *          ./bin/ls -l
 *          ./bin/ls /etc
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
#include <sys/ioctl.h>
#include <termios.h>

extern int errno;

/* Function Prototypes */
void do_ls(const char *dir, int long_listing);
void print_file_info(const char *path, const char *filename);

/* ────────────────────────────────────────────── */
int main(int argc, char const *argv[])
{
    int long_listing = 0;
    const char *dirs[64];
    int dir_count = 0;

    /* Parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0)
            long_listing = 1;
        else
            dirs[dir_count++] = argv[i];
    }

    if (dir_count == 0)
        dirs[dir_count++] = ".";

    for (int i = 0; i < dir_count; i++) {
        if (dir_count > 1)
            printf("Directory listing of %s:\n", dirs[i]);
        do_ls(dirs[i], long_listing);
        if (dir_count > 1)
            puts("");
    }

    return 0;
}

/* ────────────────────────────────────────────── */
void do_ls(const char *dir, int long_listing)
{
    struct dirent *entry;
    DIR *dp = opendir(dir);
    if (!dp) {
        fprintf(stderr, "Cannot open directory: %s\n", dir);
        return;
    }

    /* ─── Long Listing ─── */
    if (long_listing) {
        errno = 0;
        while ((entry = readdir(dp)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            print_file_info(dir, entry->d_name);
        }
        if (errno != 0) perror("readdir failed");
        closedir(dp);
        return;
    }

    /* ─── Column Layout ─── */
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    int term_width = ws.ws_col ? ws.ws_col : 80;

    char *names[1024];
    int count = 0, maxlen = 0;
    while ((entry = readdir(dp)) != NULL) {
        if (entry->d_name[0] == '.') continue;
        names[count] = strdup(entry->d_name);
        int len = strlen(entry->d_name);
        if (len > maxlen) maxlen = len;
        count++;
    }
    closedir(dp);

    int colwidth = maxlen + 2;
    int cols = term_width / colwidth;
    if (cols < 1) cols = 1;
    int rows = (count + cols - 1) / cols;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            int i = c * rows + r;
            if (i < count)
                printf("%-*s", colwidth, names[i]);
        }
        putchar('\n');
    }

    for (int i = 0; i < count; i++)
        free(names[i]);
}

/* ────────────────────────────────────────────── */
void print_file_info(const char *path, const char *filename)
{
    char fullpath[1024];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    struct stat info;
    if (stat(fullpath, &info) == -1) {
        perror("stat");
        return;
    }

    /* File type and permissions */
    printf((S_ISDIR(info.st_mode)) ? "d" : "-");
    printf((info.st_mode & S_IRUSR) ? "r" : "-");
    printf((info.st_mode & S_IWUSR) ? "w" : "-");
    printf((info.st_mode & S_IXUSR) ? "x" : "-");
    printf((info.st_mode & S_IRGRP) ? "r" : "-");
    printf((info.st_mode & S_IWGRP) ? "w" : "-");
    printf((info.st_mode & S_IXGRP) ? "x" : "-");
    printf((info.st_mode & S_IROTH) ? "r" : "-");
    printf((info.st_mode & S_IWOTH) ? "w" : "-");
    printf((info.st_mode & S_IXOTH) ? "x" : "-");

    /* Links, owner, group, size, time, name */
    struct passwd *pw = getpwuid(info.st_uid);
    struct group  *gr = getgrgid(info.st_gid);
    char *time_str = ctime(&info.st_mtime);
    time_str[strlen(time_str) - 1] = '\0';

    printf(" %2ld %-8s %-8s %8ld %s %s\n",
           (long)info.st_nlink,
           pw ? pw->pw_name : "?",
           gr ? gr->gr_name : "?",
           (long)info.st_size,
           time_str,
           filename);
}

