#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"
#include "kernel/fs.h"

void showArgv(int argc, char **argv) {
    int i;
    for( i=0; i < argc; i++ )
        printf("%d：#%s#\n",i+1, argv[i] ); 
    return;
}

int main(int argc, char **argv) {
    char buf[MAXARG];   
    // printf("\n1 argv\n");
    // showArgv(argc, argv); 
    int i = 0;
    char *p, *q;

    //每个参数向前一个位置，形成新的参数列表
    for (i = 0; i < argc - 1; i++) {
        memmove(argv[i], argv[i + 1], strlen(argv[i + 1]));
        if (strlen(argv[i]) > strlen(argv[i + 1]))
            memset(argv[i] + strlen(argv[i + 1]), '\0', strlen(argv[i]) - strlen(argv[i + 1]));
    }
    memset(argv[i], '\0', strlen(argv[i]));     //原先的最后一个参数设为空
    // printf("\n2 argv\n");
    // showArgv(argc, argv); 

    
    while (read(0, buf, MAXARG)) { //等待读入
        // printf("buf \n #%s#\n", buf);
        p = buf;
        q = buf;
        while (*p) { //扫描读入的字串
            if (*p == '\n') { //将遇到的换行符替换为0
                memset(p, '\0', 1);
            }
            if (*p == '\0') { //遇到0则将此之前的字串作为参数加入参数表，由子进程执行
                if (fork() == 0) {
                    memmove(argv[i], q, strlen(q));
                    if (strlen(argv[i]) > strlen(q))
                        memset(argv[i] + strlen(q), '\0', strlen(argv[i]) - strlen(q));
                    argv[++i] = 0;
                    // printf("\n1 argv\n");
                    // showArgv(argc, argv); 
                    exec(argv[0], argv);
                    printf("exec failed\n");
                    exit();
                }
                else {
                    wait();
                    q = p;
                    q++;
                } 
            }
            p++;    //若是执行换行符前的参数，p++后仍能扫描后续字串
        }
    } 
    exit();
}

//sh < xargstest.sh
//xargs echo