//**************************** ECHO CLIENT CODE **************************
// The echo client client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netdb.h>
#include "lineCommands.c"

#define MAX 256

// Define variables
struct hostent *hp;              
struct sockaddr_in  server_addr; 

int sock, r;
int SERVER_IP, SERVER_PORT; 

// clinet initialization code

int client_init(char *argv[])
{
  printf("======= clinet init ==========\n");

  printf("1 : get server info\n");
  hp = gethostbyname(argv[1]);
  if (hp==0){
     printf("unknown host %s\n", argv[1]);
     exit(1);
  }

  SERVER_IP   = *(long *)hp->h_addr;
  SERVER_PORT = atoi(argv[2]);

  printf("2 : create a TCP socket\n");
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock<0){
     printf("socket call failed\n");
     exit(2);
  }

  printf("3 : fill server_addr with server's IP and PORT#\n");
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = SERVER_IP;
  server_addr.sin_port = htons(SERVER_PORT);

  // Connect to server
  printf("4 : connecting to server ....\n");
  r = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (r < 0){
     printf("connect failed\n");
     exit(1);
  }

  printf("5 : connected OK to \007\n"); 
  printf("---------------------------------------------------------\n");
  printf("hostname=%s  IP=%s  PORT=%d\n", hp->h_name, inet_ntoa(SERVER_IP), SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

int main(int argc, char *argv[ ])
{
  int n;
  char line[MAX], ans[MAX];
  char *hostname;
  char token[32][64];
  char *linept, *myargv[32];

  if (argc < 3){
     printf("Usage : client ServerName SeverPort\n");
     exit(1);
  }

  client_init(argv);
   
  printf("********  processing loop  *********\n");
  while (1){
    printf("input a line : ");
    bzero(line, MAX);                // zero out line[ ]
    fgets(line, MAX, stdin);         // get a line (end with \n) from stdin

    line[strlen(line)-1] = 0;        // kill \n at end
    if (line[0]==0)                  // exit if NULL line
       exit(0);

    linept = line;

    bzero(myargv, 32);

    int i = 0;
    while (*linept)
    {
      sscanf(linept, "%s", token[i]); //tokenize line
      linept+=strlen(token[i])+1;
      myargv[i] = token[i];
      i++;
    }
        
    if (strcmp(myargv[0], "lmkdir") == 0)
    {
      printf("made it in mkdir\n");
      makeDir(myargv);
      linept = "Directory Made on Client\n";
    }
    else if (strcmp(myargv[0], "lrmdir") == 0)
    {
      removeDir(myargv);
      linept = "Directory Removed on Client\n";
    }
    else if (strcmp(myargv[0], "lrm") == 0)
    {
      removeFile(myargv);
      linept = "File removed on Client\n";
    }
    else if (strcmp(myargv[0], "lcat") == 0) 
    {
      catFile(myargv);
      linept = "File concatinated on Client\n";
    }
    else if (strcmp(myargv[0], "lls") == 0) //should also send stuff to the client, doesn't ls current directory well
    {
      listDirectory(myargv);
      linept = "List Printed from Client\n";
    }
    else if (strcmp(myargv[0], "lcd") == 0) //doesn't work right.
    {
      char cwd[1024];
      changeDirectory(myargv);
      getcwd(cwd, 1024);
      printf("New Directory is %s\n", cwd); 
      linept = cwd;
    }
    else if (strcmp(myargv[0], "lpwd") == 0)
    {
      char cwd[1024];
      getcwd(cwd, 1024);  //figure out the directory
      printf ("Directory = %s \n", cwd); //print it
      linept = cwd;
    }
    else if (strcmp(myargv[0], "exit") == 0)
    {
      exit(1);
    }
    else 
    {
    // Send ENTIRE line to server
    n = write(sock, line, MAX);
    printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

    // Read a line from sock and show it
    n = read(sock, ans, MAX);
    printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
    }
  }
}

