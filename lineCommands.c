//lineCommands.c
//A. Leah Zulas and Mackenzie Meade
//Lab 3
//K.C. Wang 

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>


#define BLKSIZE 4096

#define MAX 256
typedef struct {
    char *name;
    char *value;
} ENTRY;

ENTRY entry[MAX];

struct stat mystat, *sp;

char *t1 = "xwrxwrxwrx-----";
char *t2 = "---------------";

void ls_file(char *fname) //The K.C. Code for ls
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

  printf("%4d ",(int)sp->st_nlink);
  printf("%4d ",sp->st_gid);
  printf("%4d ",sp->st_uid);
  printf("%8d ",(int)sp->st_size);

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
  printf("\n");
}

int ls_dir(char *dname) //Zulas code for ls dir
{
  struct dirent *dp;
  DIR* fd;
  int i;
  char buff[1024];

  printf("Directory Location: %s \n", dname); // print the dir location
  fd = opendir(dname); //open the dir into fd
  while ((dp = readdir((DIR *)fd)) != NULL) //while not the end of the dir
  {
    if (dp->d_name!=NULL) //if the name isn't null
    {
		printf("%s \n",dp->d_name); //print the name of that item in the directory
    }
  }
  return 0;
}

void makeDir(char *tokens[32]) //mkdir
{
	if(!tokens[1]) exit(1); //Make sure there is a name to make the dir
    if (mkdir(tokens[1], 0755) < 0){ //unless it's already made, then don't make dir.
        printf("errno=%d : %s\n", errno, strerror(errno)); //instead error
    }   
}
void removeDir(char *tokens[32]) //rmdir
{
	if (!tokens[1]) exit(1); //if no name of dir, exit.
    rmdir(tokens[1]); //otherwise, remove dir
}
void removeFile(char* tokens[32]) //rm
{
    if (!tokens[1]) exit(1);  //if no name, exit
    remove(tokens[1]); //remove the file
}
void catFile(char *tokens[32]) //cat used by client
{  
  int fd, n, i;
  char buff[4096];
  if (!tokens[1]) exit(1); //if no name of thing to cat, exit
  fd = open(tokens[1], O_RDONLY); //open file name, read only
  if (fd < 0) exit(2); //if the file is empty, exit
  while (n = read(fd, buff, 1024)) //so long as there is still stuff to read, read
  {
    for (i = 0; i < n; i++) //for look to print char to the screen
    {
      //if(buff[i] == '\n') //switch \n to \r
        //putchar('\r');
      //else
        putchar(buff[i]);
    }
  }
  printf("\n");
  close(fd); //close file
}

char* catFile2(char *tokens[32]) //cat used by server
{  
  int fd, n, i, charCount;
  char buff[4096];
  char* s;
  charCount = 0;
  s = malloc(sizeof(char));
  if (!tokens[1]) { sprintf(s, "Cannot open file\n"); return s; } //if no name of thing to cat, exit
  fd = open(tokens[1], O_RDONLY); //open file name, read only
  if (fd < 0) { sprintf(s, "Cannot open file\n"); return s; } //if the file is empty, exit
  while (n = read(fd, buff, 256)) //so long as there is still stuff to read, read
  {
    charCount += 256;
    s = realloc(s, sizeof(char) * charCount);
    for (i = 0; i < n; i++) //for loop to print char to the screen
    {
      //if(buff[i] == '\n') //switch \n to \r
        //putchar('\r');
      //else
        sprintf(s + strlen(s), "%c", buff[i]);
    }
  }
  sprintf(s + strlen(s), "\n");
  close(fd); //close file
  return s;
}

void copy (char *tokens[32])
  {
  	int n, total, fd, gd;
  	char buff[4096];

    if (!tokens[1]) exit(1);

    fd = open(tokens[1], O_RDONLY);
    if(fd < 0) exit(2);
    gd = open(tokens[2], O_WRONLY|O_CREAT);
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

}
void listDirectory(char *tokens[32]) //ls code by K.C.  ls run by client
{
	char cwd[1024];
	char name[1024];
	char *s;
	int r;

    if (!tokens[1])
      s = "./";
    else
      s = tokens[1];

    sp = &mystat;
     if ((r = lstat(s, sp)) < 0) //added an ls without a directory
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

void changeDirectory(char *tokens[32]) //cd
{
	char *HOME, *envp;
	int i;
	if (!tokens[1]) //if you just said cd
	{
		tokens[1]=getenv("HOME"); //get environmental pointer
	} //set myargv1 to $HOME
	  chdir(tokens[1]); //else, go to where you asked.
}

char* printDirectory(char *tokens[32]) // pwd
{
	char* cwd = malloc (sizeof(char) * 1024);
	getcwd(cwd, 1024);  //figure out the directory
	printf ("Directory = %s \n", cwd); //print it
	return cwd;
}

char* ls_dir2(char* dname)	// used by server, builds a string
{
  struct dirent *dp;
  DIR* fd;
  int i, charCount;
  char buff[1024];
  char* s = malloc (sizeof(char) * 256);
  charCount = 256;

  sprintf(s, "Directory Location: %s \n", dname); // print the dir location
  fd = opendir(dname); //open the dir into fd
  while ((dp = readdir((DIR *)fd)) != NULL) //while not the end of the dir
  {
    if (dp->d_name!=NULL) //if the name isn't null
    {
		while (strlen(dp->d_name) + strlen(s) > charCount)
		{
			charCount += 256;
			s = realloc(s, sizeof(char) * charCount);
		}
		sprintf(s + strlen(s), "%s \n",dp->d_name); //print the name of that item in the directory
    }
  }
  return s;
}

char* ls_file2(char* fname)	// used by server, builds a string
{
  struct stat fstat, *sp;
  int r, i, charCount;
  char ftime[64];
  char buff[1024];
  char* s = malloc(sizeof(char) * 256);
  charCount = 256;

  sp = &fstat;
//  printf("name=%s<br>", fname); getchar();

  if ( (r = lstat(fname, &fstat)) < 0){
    sprintf(s, "can't stat %s\n", fname);
    return s;
  }

  if ((sp->st_mode & 0xF000) == 0x8000)
    sprintf(s, "%c",'-');
  if ((sp->st_mode & 0xF000) == 0x4000)
    sprintf(s + strlen(s), "%c",'d');
  if ((sp->st_mode & 0xF000) == 0xA000)
    sprintf(s + strlen(s), "%c",'l');

  for (i=8; i >= 0; i--){
   if (sp->st_mode & (1 << i))
    sprintf(s + strlen(s), "%c", t1[i]);
   else
    sprintf(s + strlen(s), "%c", t2[i]);
  }

  sprintf(s + strlen(s), "%4d ",(int)sp->st_nlink);
  sprintf(s + strlen(s), "%4d ",sp->st_gid);
  sprintf(s + strlen(s), "%4d ",sp->st_uid);
  sprintf(s + strlen(s), "%8d ",(int)sp->st_size);

  // print time
  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime)-1] = 0;
  sprintf(s + strlen(s), "%s ",ftime);

  // print name
  while (strlen(basename(fname)) + strlen(s) > charCount)
  {
    charCount += 256;
    s = realloc(s, sizeof(char) * charCount);
  }
  sprintf(s + strlen(s), "%s", basename(fname));
  bzero(buff, 1024);
  // print -> linkname if it's a symbolic file
  if ((sp->st_mode & 0xF000)== 0xA000){ // YOU FINISH THIS PART
    if (readlink(fname, buff, 1024) != -1)// SYSCALL to read the linkname
    {
      while (strlen(buff) + strlen(s) + 4 > charCount)
      {
        charCount += 256;
        s = realloc(s, sizeof(char) * charCount);
      }
      sprintf(s + strlen(s), " -> %s", buff);
    }
  }
  while (1 + strlen(s) > charCount)
  {
    charCount += 256;
    s = realloc(s, sizeof(char) * charCount);
  }
  sprintf(s + strlen(s), "\n");

  return s;
}

char* listDirectory2(char* tokens[32])	// ls used by server
{
	char cwd[1024];
	char name[1024];
	char *s;
	char* res;
	int r;

    if (!tokens[1])
      s = "./";
    else
      s = tokens[1];

    sp = &mystat;
     if ((r = lstat(s, sp)) < 0) //added an ls without a directory
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
      res = ls_dir2(name);
    else
      res = ls_file2(name);

    return res;
}

// create functions fileToString, stringToFile to read/write from files?