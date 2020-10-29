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
    }
    else if (rc == 0){
        if (read(parent_fd[0], buf, sizeof buf)) {
            write(child_fd[1], "pong\n", 5);
            printf("received ");
            printf(buf);
            close(parent_fd[0]);
        }
    }
    else {
        write(parent_fd[1], "ping\n", 5);
        if (read(child_fd[0], buf, sizeof buf)) {
            printf("received ");
            printf(buf);
            close(parent_fd[1]);
            close(parent_fd[0]);
            close(child_fd[1]);
        }
    }
    exit();

}