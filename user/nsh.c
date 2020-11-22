#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"
#define MAXARGS 10
#define MAXCMD 10
#define EXEC 0
#define REDIR 1
#define PIPE 2

// 要执行的指令
typedef struct cmd{
    int type;           // 指令种类 EXEC普通指令 REDIR重定向指令 PIPE管道指令
    char *argv[MAXARGS];// 指令参数 EXEC有效
    struct cmd *cmd;    // 子指令 REDIR有效
    char *file;         // 文件名 REDIR有效
    int model;          // 打开文件方式 REDIR有效
    int fd;             // 要关闭的句柄 REDIR有效
    struct cmd *left;   // 管道左端指令 PIPE有效
    struct cmd *right;  // 管道右端指令 PIPE有效
}Cmd;

// 错误信息
void panic(char *s) {
    fprintf(2, "%s\n", s);
    exit(-1);
}
// fork
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
//ps指向下一token开头或es
//返回token的种类
int gettoken(char **ps, char *es, char **q, char **eq)
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

// 申请的cmd空间
Cmd cmds[MAXCMD];
int cmd_cnt = 0;

void init_cmd (void) {
    Cmd *p = cmds;
    memset(p, 0, MAXCMD*sizeof(Cmd));
}

void setRedircmd (Cmd* cmd, Cmd* childcmd, char *file, int model, int fd) {
    cmd->type = REDIR;
    cmd->cmd = childcmd;
    cmd->file = file;
    cmd->model = model;
    cmd->fd = fd;
}

void showCmd(Cmd* cmd) {
    printf("--showCmd--\n");
    if (cmd->type == EXEC) {
        printf("exec\n");
        int argc = 0;
        while (cmd->argv[argc]) {
            printf("%d  #%s#\n", argc, cmd->argv[argc]);
            argc++;
        }
    }
    else if (cmd->type == REDIR) {
        printf("redir\n");
        printf("file #%s#\n", cmd->file);
        showCmd(cmd->cmd);
    }
    else if (cmd->type == PIPE) {
        printf("left\n");
        showCmd(cmd->left);
        printf("right\n");
        showCmd(cmd->right);
    }
    printf("-----------\n");
    return;
}
// 设置EXECcmd 将参数填入argv
Cmd* parseexec(char *s, Cmd* cmd) {
    cmd->type = EXEC;
    char *p = s;
    char *ep = s + strlen(s);
    char *q = s;
    char *eq = s;
    int argc = 0;
    while (*p) {
        gettoken(&p, ep, &q, &eq);  // 获取一个参数
        *eq = 0;
        cmd->argv[argc++] = q;
        if(argc >= MAXARGS)
            panic("too many args");
    }
    cmd->argv[argc] = 0;
    // showCmd(cmd);
    return cmd;
}
// 检查是否有重定向 有则设置 否则设置普通cmd
Cmd* parseredir(char *start, char *end, Cmd *cmd) {
    char *p = start;
    char *q = start;
    char *eq = start; 
    while (*p) {
        if (*p == '<') {
            *p++ = 0;
            if (cmd->cmd != 0) { // 第二次重定向
                //选择一个空的cmd并设置之前的重定向cmd为其子cmd
                // printf("second redir\n");
                gettoken(&p, end, &q, &eq); //读取文件名
                *eq = 0;
                setRedircmd(&cmds[cmd_cnt], cmd, q, O_RDONLY, 0);
                // showCmd(&cmds[cmd_cnt]);
                cmd_cnt++;
                return &cmds[cmd_cnt-1];
            }
            // 第一次重定向
            // 选取一个空的cmd作为执行指令的cmd，并设置其为子cmd
            // printf("first redir\n");
            parseexec(start, &cmds[cmd_cnt]); 
            gettoken(&p, end, &q, &eq); //读取文件名
            *eq = 0;
            setRedircmd(cmd, &cmds[cmd_cnt], q, O_RDONLY, 0);
            // showCmd(cmd);
            cmd_cnt++;
        }
        else if (*p == '>') {
            *p++ = 0;
            if (cmd->cmd != 0) { // 第二次重定向
            //选择一个空的cmd并设置之前的重定向cmd为其子cmd
                // printf("second redir\n");
                gettoken(&p, end, &q, &eq); //读取文件名
                *eq = 0;
                setRedircmd(&cmds[cmd_cnt], cmd, q, O_CREATE | O_WRONLY, 1); 
                // showCmd(cmd);
                cmd_cnt++;
                return &cmds[cmd_cnt-1];
            }
            // printf("first redir\n");
            // 第一次重定向
            // 选取一个空的cmd作为执行指令的cmd，并设置其为子cmd
            parseexec(start, &cmds[cmd_cnt]);
            gettoken(&p, end, &q, &eq); //读取文件名
            *eq = 0;
            setRedircmd(cmd, &cmds[cmd_cnt], q, O_CREATE | O_WRONLY, 1);
            cmd_cnt++;
            // showCmd(cmd);
        }
        else p++;
    }
    // 无重定向则设置cmd为普通cmd
    if (cmd->cmd == 0)
        parseexec(start, cmd);
    return cmd;
}
// 检查是否有管道 有则设置 否则检查是否有重定向
Cmd *parsecmd(char *start, char *end) {
    init_cmd();
    char *p = start;
    Cmd *cmd = &cmds[cmd_cnt++];
    while (*p) {
        if (*p == '|') { // 有管道
            *p = 0;
            cmd->type = PIPE;
            cmd->left = &cmds[cmd_cnt++];
            cmd->right = &cmds[cmd_cnt++];
            parseredir(start, p, cmd->left);
            parseredir(p+1, end, cmd->right);
            // showCmd(cmd);
            return cmd;
        }
        p++;
    }
    return parseredir(start, end, cmd);;
}

void runcmd(Cmd *cmd) {
    int p[2];
    switch (cmd->type) {
        case EXEC:
            // fprintf(2, "run exec\n------\n");
            if (cmd->argv[0] == 0) exit(-1);
            exec(cmd->argv[0], cmd->argv);
            fprintf(2, "exec %s failed\n", cmd->argv[0]);
            break;
        case REDIR:
            // fprintf(2, "run redir\n------\n");
            close(cmd->fd);
            if(open(cmd->file, cmd->model) < 0)
                panic("open file failed");
            runcmd(cmd->cmd);
            break;
        case PIPE:
            if(pipe(p) < 0)
                panic("pipe");
            // printf("run pipe\n------\n");
            if(fork1() == 0){
                close(1);
                dup(p[1]);
                close(p[0]);
                close(p[1]);
                runcmd(cmd->left);
            }
            if(fork1() == 0){
                close(0);
                dup(p[0]);
                close(p[0]);
                close(p[1]);
                runcmd(cmd->right);
            }
            close(p[0]);
            close(p[1]);
            wait(0);
            wait(0);
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
    static char buf[10001];
    char *p = buf;
    char *ep = buf;
    while (getcmd(buf, sizeof(buf)) >= 0) {
        p = buf;
        while (*p) {
            ep = p;
            // 将buf在换行符处分开，每次执行一行
            while (*ep && *ep != '\n') ep++;    
            *ep = 0;
            if (fork1() == 0) {
                runcmd(parsecmd(p, ep));
            }
            wait(0);
            p = ep + 1;
        }
    }
    exit(0);
}