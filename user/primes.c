#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main()
{
    char buf_now[64];
    char buf_next[64];
    int i = 0;
    int j = 0;
    int num = 34;
    
    int fd[2];
    pipe(fd);

    int rc = fork();
    if (rc < 0) {
        printf("fork failed\n");
        exit();
    }
    else if (rc > 0) {
        for (i = 0; i < num; i++) {
            buf_now[i] = i + 2;
            // printf("%d ", buf_now[i]);
        }
        
        close(fd[0]);
        write(fd[1], buf_now, num);
        close(fd[1]);
        wait();
    } 
    else {
        while (rc == 0) {
            close(fd[1]);
            num = read(fd[0], buf_now, sizeof buf_now);
            // printf("\n\npid%d received ", getpid());
            // printf("\nnum %d", num);
            close(fd[0]);
            printf("prime %d\n", buf_now[0]);
            j = 0;
            for (i = 1; i < num; i++) {
                if (buf_now[i] % buf_now[0] != 0) {
                    buf_next[j] = buf_now[i];
                    j++;
                }
                // printf("%d ", buf_now[i]);
            }
            if (j == 0) exit();
            num = j;
            
            pipe(fd);
            write(fd[1], buf_next, num);
            rc = fork();
            wait();
        }
    }
    exit();
}