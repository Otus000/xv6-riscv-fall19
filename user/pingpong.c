#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main()
{
    int parent_fd[2];
    int child_fd[2];
    pipe(parent_fd);
    pipe(child_fd);
    char buf[512];

    int rc = fork();
    if (rc < 0) {
        printf("\n fork failed");
        exit();
    }
    else if (rc == 0){
        close(parent_fd[1]);
        close(child_fd[0]);
        if (read(parent_fd[0], buf, sizeof buf)) {
            close(parent_fd[0]);
            write(child_fd[1], "pong\n", 5);
            close(child_fd[1]);
            printf("received ");
            printf(buf);
        }
    }
    else {
        close(child_fd[1]);
        close(parent_fd[0]);
        write(parent_fd[1], "ping\n", 5);
        close(parent_fd[1]);
        if (read(child_fd[0], buf, sizeof buf)) {
            printf("received ");
            printf(buf);
            close(child_fd[0]);
        }
    }
    exit();
}