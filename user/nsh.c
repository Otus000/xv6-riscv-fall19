#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#define MAXARGS 10
#define EXEC 0
#define REDIR 1
#define PIPE 2
struct cmd {
    int type;
};
struct execcmd {
    int type;
    char *argv[MAXARGS];
};
struct redircmd {

};
struct pipecmd {
    int type;
    struct cmd *left;
    struct cmd *right;
};

void panic(char *s) {
    fprintf(2, "%s\n", s);
    exit(-1);
}
int fork1(void) {
    int pid;
    pid = fork();
    if (pid < 0) {
        panic("fork");
    }
    return pid;
}
//构造函数 
struct cmd *execcmd (void) {
    struct execcmd cmd;
    struct execcmd *pcmd = &cmd;
    pcmd->type = EXEC;
    memset(pcmd, 0, sizeof(pcmd));
    return (struct cmd*) pcmd;
}

struct cmd *parsecmd(char *s) {
    struct execcmd *cmd;
    struct cmd *ret;
    ret = execcmd();
    cmd = (struct execcmd*)ret;
    cmd->type = EXEC;

    char *p = s;
    char *es = s + strlen(s);
    char *q = s;
    int argc = 0;
    while (p < es) {
        while (*q && *q != ' ' && *q != '\n') {
            q++;
        }
        *q = 0;
        cmd->argv[argc++] = p;
        p = ++q;
    }
    cmd->argv[argc] = 0;
    return (struct cmd*) cmd;
}


void runcmd(struct cmd *cmd) {
    struct execcmd *ecmd;
    switch (cmd->type) {
        case EXEC:
            ecmd = (struct execcmd*)cmd;
            exec(ecmd->argv[0], ecmd->argv);
    }
}

int getcmd(char *buf, int nbuf) {
    fprintf(2, "@ ");
    memset(buf, 0, nbuf);
    gets(buf, nbuf);
    if (buf[0] == 0) return -1;
    return 0;
}

int main(void) {
    static char buf[100];

    while (getcmd(buf, sizeof(buf)) >= 0) {
        if (fork1() == 0) {
            runcmd(parsecmd(buf));
        }
        wait(0);
    }
    exit(0);
}