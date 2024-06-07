
#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "kernel/fs.h"
#include "kernel/stat.h"
#include "user/user.h"

void find(char *path, char *target_file)
{
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0)
    {
        printf("ls: cannot open %s\n", path);
        exit(0);
    }

    if (fstat(fd, &st) < 0)
    {
        printf("ls: cannot stat %s\n", path);
        close(fd);
        exit(0);
    }

    switch (st.type)
    {
    case T_FILE: // file, check name
        /* code */
        if (strcmp(de.name, target_file) == 0)
        {
            printf("%s\n", buf);
        }
        exit(0);
    case T_DIR: // directory, recursion
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof buf)
        {
            
            printf("ls: path too long, %d \n", sizeof buf);
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de))
        {
            if (de.inum == 0 || strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
            {
                continue;
            }
            memmove(p, de.name, DIRSIZ);

            if (stat(buf, &st) < 0)
            {
                printf("ls: cannot stat %s\n", buf);
                continue;
            }
            if (st.type == T_DIR)
            {
                find(buf, target_file);
            }
            else if (st.type == T_FILE)
            {
                if (strcmp(de.name, target_file) == 0)
                {
                    printf("%s\n", buf);
                }
            }
        }
        break;
    }
    close(fd);
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("usage: find [directory] [target filename]\n");
        exit(0);
    }
    char *path = argv[1];
    char *target = argv[2];
    find(path, target);
    return 0;
}