#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char **argv) 
{
    if (argc < 2){
        printf("Sleep needs two arguments\n");
        exit();
    } else if (argc > 2)
    {
        printf("Too many arguments\n");
        printf("Sleep needs two arguments\n");
        exit();
    }

    char *c = argv[1];
    while (*c != '\0') {
        if (*c < '0' || *c > '9') {
            printf("Invalid input\n");
            printf("Please input numbers\n");
            exit();
        }
        c++;
    }

    int n;
    n = atoi(argv[1]);
    sleep(n);

    exit();
}