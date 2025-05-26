#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

void find(char *path, char *target) {
    char buf[512], *p;
    int fd;
    struct dirent de;
    struct stat st;

    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }
    if (fstat(fd, &st) < 0) {
        fprintf(2, "cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch (st.type) {
    // 当前处理的目录项是文件，一律从这个分支走
    case T_FILE:
        if (strcmp(path + strlen(path) - strlen(target), target) == 0) {
            printf("%s\n", path);
        }
        break;

    // 当前处理的目录项是目录
    case T_DIR:
        if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
            printf("find: path too long\n");
            break;
        }
        strcpy(buf, path);
        p = buf + strlen(buf);
        *p++ = '/';

        // 对于该目录项下的每一项，依次进行处理，p固定好位置很关键
        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0) {
                continue;
            }
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            if (stat(buf, &st) < 0) {
                printf("find: cannot stat %s\n", buf);
                continue;
            }

            if (strcmp(buf + strlen(buf) - 2, "/.") != 0
                && strcmp(buf + strlen(buf) - 3, "/..") != 0) {
                find(buf, target);
            }
        }
        break;
    }

    close(fd);
}

int main(int argc, char **argv) {
    if (argc < 3) {
        exit(0);
    }

    char target[512];
    target[0] = '/';
    strcpy(target, argv[2]);
    find(argv[1], target);

    exit(0);
}
