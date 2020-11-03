#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char *fmtname(char* path) {
  static char buf[DIRSIZ+1];
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

  // Return name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf+strlen(p), '\0', DIRSIZ-strlen(p));
  return buf;
}

void
find(char *path, char *name) {
  char buf[512], *p;
  int fd;
  struct dirent de;
  struct stat st;
  
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %s\n", path);
    return;
  }
  
  if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat1 %s\n", path);
    close(fd);
    return;
  }
  //匹配
  if (strcmp(name, fmtname(path)) == 0) {
      printf("%s\n", path);
  }
  // switch (st.type) {
    // case T_FILE:
    //     // printf("name ##%s##\n", name);
    //     // printf("path ##%s##\n", fmtname(path));
    //     // printf("no match\n");
    //     break;
    // case T_DIR:

  //在dir中查找
  if (st.type == T_DIR) {
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      printf("find: path too long\n");
      return;
    }

    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';
    // printf("find in ##%s##\n", buf);
    //对当前dir中的所有文件
    while(read(fd, &de, sizeof(de)) == sizeof(de)){
      if (de.inum == 0) {
        continue;
      }
      // printf("name ##%s##\n", de.name);
      memmove(p, de.name, DIRSIZ);
      p[DIRSIZ] = 0;
      if(stat(buf, &st) < 0) {
        printf("find: cannot stat2 %s\n", buf);
        continue;
      }

      //递归查找 .和.. 以外的文件
      if (strcmp(fmtname(buf), ".\0") != 0 
        && strcmp(fmtname(buf), "..\0") != 0){
        // printf("find ##%s## ##%s##\n", buf, fmtname(buf));
        find(buf, name);
      }
    }
  }
  close(fd);
  return;
}

int
main(int argc, char *argv[])
{
  if(argc < 2){
    exit();
  }
  else if (argc < 3) {
    find(".", argv[1]);
  }
  else {
    find (argv[1], argv[2]);
  }
  exit();
}
