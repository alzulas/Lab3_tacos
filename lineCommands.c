//lineCommands.c

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>


#define BLKSIZE 4096

#define MAX 10000
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];

struct stat mystat, *sp;

char *t1 = "xwrxwrxwrx-----";
char *t2 = "---------------";

void ls_file(char *fname)
{
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  char buff[1024];

  sp = &fstat;
//  printf("name=%s<br>", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
    printf("can't stat %s\n", fname);
    exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
    printf("%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
    printf("%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
    printf("%c",'l');

  for (i=8; i >= 0; i--){
   if (sp->st_mode & (1 << i))
    printf("%c", t1[i]);
   else
    printf("%c", t2[i]);
  }

  printf("%4d ",sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  printf("%s ",ftime);

  // print name
  printf("%s", basename(fname));
  bzero(buff, 1024);
  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
    if (readlink(fname, buff, 1024) != -1)// SYSCALL to read the linkname
       printf(" -> %s", buff);
  }
  printf("<br>");
}

int ls_dir(char *dname)
{
  struct dirent *dp;
  int fd, i;
  char buff[1024];

  printf("Directory Location: %s <br>", dname);
  fd = opendir(dname);
  while ((dp = readdir(fd)) != NULL)
  {
     if (dp->d_name!=NULL)
     {
	printf("%s <br>",dp->d_name);
     }
  }
  //printf("But it didn't print anything <br>");
  return 0;
}

makeDir(ENTRY entry)
{
	if(entry[1].value == NULL) exit(1);
    
    mkdir(entry[1].value, 0755);
    
    if (mkdir(entry[1].value, 0777) < 0){
        printf("errno=%d : %s\n", errno, strerror(errno));
    }
}
removeDir(ENTRY entry)
{
	if (entry[1].value == NULL) exit(1);
    rmdir(entry[1].value);
}
removeFile(ENTRY entry)
{
    if (entry[1].value == NULL) exit(1);
    remove(entry[1].value);    
}
catFile(ENTRY entry)
{  
    if (entry[1].name == NULL) exit(1);
    fd = open(entry[1].value, O_RDONLY);
    if (fd < 0) exit(2);
    while (n = read(fd, buff, 1024))
    {
      for (i = 0; i < n; i++)
      {
        if(buff[i] == "\n")
          putchar("\r");
        else
          putchar(buff[i]);
      }
    }
    printf("\n");
    close(fd);
}
/*  else if (strcmp(entry[0].value, "cp")==0)
  {
    if (entry[2].name == NULL) exit(1);

    fd = open(entry[1].value, O_RDONLY);
    if(fd < 0) exit(2);
    gd = open(entry[2].value, O_WRONLY|O_CREAT);
    if (gd < 0) exit(3);
    total = 0;    

    printf("\n");
    while ((n = read(fd, buff, BLKSIZE))!=0)
    {
      write(gd, buff, n);
      total += n;
    }
    printf("total = %d<p>", total);
    close(fd);
    close(gd);

  }*/
listDirectory(ENTRY entry)
{
    if (entry[1].value == NULL)
      s = "./";
    else
      s = entry[1].value;

    sp = &mystat;
     if ((r = lstat(s, sp)) < 0)
    {
      getcwd(cwd, 1024);
      strcpy(name, cwd);
      strcat(name, "/");
    }
    else 
    {
      strcpy(name, s);
      if (s[0] != '/'){ // name is relative : get CWD path
        getcwd(cwd, 1024);
        strcpy(name, cwd); 
        strcat(name, "/"); 
        strcat(name, s);
      }
    }
    if (S_ISDIR(sp->st_mode))
      ls_dir(name);
    else
      ls_file(name);
}

changeDirectory(myargv)
{
	if (myargv[1]==NULL)
	{
		chdir(HOME);
	} //set myargv1 to $HOME
	chdir(myargv[1]);
}

