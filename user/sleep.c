#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char **argv) 
{
    if (argc < 2){
        printf("\nSleep needs two arguments\n\n");
        exit();
    } else if (argc > 2)
    {
        printf("\nToo many arguments\n");
        printf("\nSleep needs two arguments\n\n");
        exit();
    }

    char *c = argv[1];
    while (*c != '\0') {
        if (*c < '0' || *c > '9') {
            printf("\nInvalid input\n");
            printf("\nPlease input numbers\n\n");
            exit();
        }
        c++;
    }

    int n;
    n = atoi(argv[1]);
    sleep(n);

    exit();
}