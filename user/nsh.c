#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#define MAXARGS 10
#define EXEC 0
#define REDIR 1
#define PIPE 2

typedef struct execcmd {
    char *argv[MAXARGS];
}execcmd;
typedef struct redircmd {
    char *file;
    int model;
    int fd;
}redircmd;
typedef struct pipecmd {
    int type;
    struct cmd *left;
    struct cmd *right;
}pipecmd;

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
char whitespace[] = " \t\r\n\v";
char symbols[] = "<|>";
//扫描从ps到es的字符串
//将扫描到的token的起止位置保存在q和eq
//返回token的种类
int
gettoken(char **ps, char *es, char **q, char **eq)
{
  char *s;
  int ret;

  s = *ps;
  while(s < es && strchr(whitespace, *s))
    s++;
  if(q)
    *q = s;
  ret = *s;
  switch(*s){
  case 0:
    break;
  case '|':
  case '<':
  case '>':
    s++;
    break;
  default:
    ret = 'a';
    while(s < es && !strchr(whitespace, *s) && !strchr(symbols, *s))
      s++;
    break;
  }
  if(eq)
    *eq = s;
  while(s < es && strchr(whitespace, *s))
    s++;
  *ps = s;
  return ret;
}
void showExec(execcmd *ecmd) {
    printf("--exec--\n");
    int argc = 0;
    while (ecmd->argv[argc]) {
        printf("%d  #%s#\n", argc, ecmd->argv[argc]);
        argc++;
    }
    printf("-------\n");
    return;
}
//构造函数 
int type = EXEC;
execcmd ecmd;
redircmd rcmd[2];
int rcmd_cnt = 0;
void init_execcmd (void) {
    execcmd *p = &ecmd;
    memset(p, 0, sizeof(ecmd));
}
void setRedircmd (char *file, int model, int fd) {
    if (rcmd_cnt > 1) {
        panic("too many redirctions");
    }
    redircmd *p = &rcmd[rcmd_cnt];
    memset(p, 0, sizeof(rcmd[0]));
    
    rcmd[rcmd_cnt].file = file;
    rcmd[rcmd_cnt].model = model;
    rcmd[rcmd_cnt++].fd = fd;
}

void parseexec(char *s) {
    char *p = s;
    char *ep = s + strlen(s);
    char *q = s;
    char *eq = s;
    int argc = 0;
    while (*p) {
        gettoken(&p, ep, &q, &eq);
        *eq = 0;
        ecmd.argv[argc++] = q;
    }
    ecmd.argv[argc] = 0;
    // showExec(&ecmd);
    return;
}
void parsedir(char *start, char *end) {
    char *p = start;
    char *q = start;
    char *eq = start; 
    while (*p) {
        if (*p == '<') {
            *p++ = 0;
            if (rcmd_cnt == 0)
                parseexec(start);
            gettoken(&p, end, &q, &eq); //读取文件名
            if (!strchr(whitespace, *eq)) panic("syntax");
            *eq = 0;
            setRedircmd(q, O_RDONLY, 0);
            type = REDIR;
        }
        else if (*p == '>') {
            *p++ = 0;
            if (rcmd_cnt == 0)
                parseexec(start);
            gettoken(&p, end, &q, &eq);
            if (!strchr(whitespace, *eq)) panic("syntax");
            *eq = 0;
            setRedircmd(q, O_CREATE | O_WRONLY, 1);
            type = REDIR;
        }
        else p++;
    }
    if (type == EXEC) parseexec(start);
    return;
}
void parsepipe(char *start, char *end) {

}
void parsecmd(char *s) {
    char *es = s + strlen(s);
    char *p = s;
    while (*p) {
        if (*p == '|') {
            parsepipe(s, es);
            type = PIPE;
        }
        p++;
    }
    parsedir(s, es);
    return;
}
void runRedir(int cnt) {
    close(rcmd[cnt].fd);
    if(open(rcmd[cnt].file, rcmd[cnt].model) < 0){
      panic("open file failed");
    }
    if (cnt == 1) runRedir(0);
    exec(ecmd.argv[0], ecmd.argv);
    return;
}
void runcmd(void) {
    switch (type) {
        case EXEC:
            // printf("run exec\n------\n");
            exec(ecmd.argv[0], ecmd.argv);
            break;
        case REDIR:
            // printf("run redir\n------\n");
            runRedir(--rcmd_cnt);
            break;

    }
    // printf("------\n");
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
            parsecmd(buf);
            runcmd();
        }
        wait(0);
    }
    exit(0);
}