#define _XOPEN_SOURCE 500

#include <poll.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <ftw.h>
#include "logger.h"

char *directories[4096];
int count = 0;
char *mainDirectory;

// NOTE: I am managing by default only subdirectories that go from level 0 to level 2
// when initializing the program, because there are some directories very big that can lead
// to a segmentation core dumped when trying to store them (Example: /home/user directory),
// but if you create directories of higher level within the terminal they will be monitored
// (level 3, level 4, level 5, etc.)
static int display_info(const char *fpath, const struct stat *sb,
                        int tflag, struct FTW *ftwbuf)
{
    if (tflag == FTW_D && ftwbuf->level < 3)
    {
        directories[count] = malloc(sizeof(char) * 1024);
        snprintf(directories[count++], 1024, "%s", fpath);
    }

    return 0;
}

char *traducePath(int *wd, int wd2, const char name[])
{
    char *path;
    path = malloc(sizeof(char) * 1024);

    for (int i = 0; i < count; i++)
    {
        if (wd[i] == wd2)
        {
            snprintf(path, 1024, "%s", directories[i]);
            break;
        }
    }

    char *path2;
    path2 = malloc(sizeof(char) * 1024);
    int index = 0;
    strcpy(path2, "");

    for (int i = strlen(mainDirectory) + 1;
         i < strlen(path);
         i++)
    {
        path2[index++] = path[i];
    }
    if (!(strlen(mainDirectory) == strlen(path)))
    {
        path2[index++] = '/';
        path2[index] = '\0';
    }
    return path2;
}

int typeOfFileSystemObject(uint32_t mask, const char name[], const char action[], int *wd, int wd2)
{
    if (mask & IN_ISDIR)
    {
        infof("- [Directory - %s] - %s%s", action, traducePath(wd, wd2, name), name);
        return 1;
    }
    else
    {
        infof("- [File - %s] - %s%s", action, traducePath(wd, wd2, name), name);
        return 0;
    }
}

void addWatcher(int *wd, int wd2, const char name[], int fd)
{
    for (int i = 0; i < count; i++)
    {
        if (wd[i] == wd2)
        {
            directories[count] = malloc(sizeof(char) * 1024);
            snprintf(directories[count], 1024, "%s/%s", directories[i], name);
            wd[count] = inotify_add_watch(fd, directories[count], IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
            if (wd[count] < 0)
            {
                errorf("Couldn't add watch to directory");
                exit(EXIT_FAILURE);
            }
            count++;
            break;
        }
    }
}

static void handle_events(int fd, int *wd)
{
    char buf[4096]
        __attribute__((aligned(__alignof__(struct inotify_event))));
    const struct inotify_event *event;
    ssize_t len;

    while (1)
    {
        len = read(fd, buf, sizeof(buf));
        if (len == -1 && errno != EAGAIN)
        {
            errorf("read");
            exit(EXIT_FAILURE);
        }

        if (len <= 0)
        {
            break;
        }

        for (char *ptr = buf; ptr < buf + len;
             ptr += sizeof(struct inotify_event) + event->len)
        {
            event = (const struct inotify_event *)ptr;

            if (event->len)
            {
                if (event->mask & IN_CREATE)
                {
                    int flag = typeOfFileSystemObject(event->mask, event->name, "Create", wd, event->wd);
                    if (flag)
                    {
                        addWatcher(wd, event->wd, event->name, fd);
                    }
                    infof("\n");
                }
                if (event->mask & IN_DELETE)
                {
                    // By default when a directory is deleted, inotify removes the watcher;
                    typeOfFileSystemObject(event->mask, event->name, "Removal", wd, event->wd);
                    infof("\n");
                }
                if (event->mask & IN_MOVED_FROM)
                {
                    typeOfFileSystemObject(event->mask, event->name, "Rename", wd, event->wd);
                }
                if (event->mask & IN_MOVED_TO)
                {
                    infof(" -> %s%s\n", traducePath(wd, event->wd, event->name), event->name);
                }
            }
        }
    }
}

int main(int argc, char *argv[])
{
    // Validation of arguments
    if (argc < 2)
    {
        errorf("Error: Usage: %s PATH\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    mainDirectory = malloc(sizeof(char) * 1024);
    snprintf(mainDirectory, 1024, "%s", argv[1]);

    // Getting all subdirectories and files to read
    int flags = 0;
    flags |= FTW_PHYS;

    if (nftw(argv[1], display_info, 20, flags) == -1)
    {
        errorf("Error in nftw");
        exit(EXIT_FAILURE);
    }

    // Managing directories and files through inotify
    char buf;
    int fd, poll_num;
    int *wd;
    nfds_t nfds;
    struct pollfd fds[2];

    fd = inotify_init1(IN_NONBLOCK);
    if (fd < 0)
    {
        errorf("inotify_init1");
        exit(EXIT_FAILURE);
    }

    wd = calloc(4096, sizeof(int));
    if (wd == NULL)
    {
        errorf("calloc");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < count; i++)
    {
        wd[i] = inotify_add_watch(fd, directories[i], IN_CREATE | IN_DELETE | IN_MOVED_FROM | IN_MOVED_TO);
        if (wd[i] < 0)
        {
            errorf("Couldn't add watch to directory");
            exit(EXIT_FAILURE);
        }
    }

    nfds = 2;

    fds[0].fd = STDIN_FILENO;
    fds[0].events = POLLIN;

    fds[1].fd = fd;
    fds[1].events = POLLIN;

    infof("Starting File/Directory %s\n", argv[1]);
    infof("-----------------------------------------------------\n");
    while (1)
    {
        poll_num = poll(fds, nfds, -1);

        if (poll_num == -1)
        {
            if (errno == EINTR)
            {
                continue;
            }
            errorf("poll");
            exit(EXIT_FAILURE);
        }

        if (poll_num > 0)
        {
            if (fds[0].revents & POLLIN)
            {
                while (read(STDIN_FILENO, &buf, 1) > 0 && buf != '\n')
                {
                    continue;
                }
                break;
            }

            if (fds[1].revents & POLLIN)
            {
                handle_events(fd, wd);
            }
        }
    }

    close(fd);
    free(wd);
    exit(EXIT_SUCCESS);
}