//**************************** ECHO CLIENT CODE **************************
// The echo client client.c
//A. Leah Zulas and Mackenzie Meade
//Lab 3
//K.C. Wang 

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

int sock, r, count, length;
int SERVER_IP, SERVER_PORT; 

// clinet initialization code

int client_init(char *argv[])
{
  printf("======= client init ==========\n");

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
  //printf("hostname=%s  ", hp->h_name);

  //printf("IP=%s  ", inet_ntoa(SERVER_IP));
  printf("PORT=%d\n", SERVER_PORT);
  printf("---------------------------------------------------------\n");

  printf("========= init done ==========\n");
}

int main(int argc, char *argv[ ])
{
  int n, d;
  char line[MAX], ans[MAX];
  char *hostname;
  char token[32][64];
  char buf[MAX];
  char *linept, *myargv[32];
  FILE* fp;
  struct stat fstat, *sp;

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
    else if (strcmp(myargv[0], "cat") == 0)
    {
	// special case: send command to server, wait for server to reply with size of result
	// then, receive and print output in blocks of MAX bytes
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      
      // wait for reply, ans will contain a string representing the size of the result
      n = read(sock, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
      
      // client then reads and printf()s [size] bytes
      // use int length for SIZE, int count for count
      length = strtol(ans, (char**)NULL, 10);
      printf("length=%d\n", length);
      count = 0;
      while (count < length)
      {
        n = read(sock, ans, MAX);
 	printf("%s", ans);
	count += MAX;
      }
    }
    else if (strcmp(myargv[0], "lls") == 0)
    {
      listDirectory(myargv);
      linept = "List Printed from Client\n";
    }
    else if (strcmp(myargv[0], "ls") == 0)
    {
	// special case: send command to server, wait for server to reply with size of result
	// then, receive and print output in blocks of MAX bytes
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);
      
      // wait for reply, ans will contain a string representing the size of the result
      n = read(sock, ans, MAX);
      printf("client: read  n=%d bytes; echo=(%s)\n",n, ans);
      
      // client then reads and printf()s [size] bytes
      // use int length for SIZE, int count for count
      // strtol() to convert from string to int
      length = strtol(ans, (char**)NULL, 10);
	    printf("length=%d\n", length);
      count = 0;
      while (count < length)
      {
        n = read(sock, ans, MAX);
 	      printf("%s", ans);
	      count += MAX;
      }
    }
    else if (strcmp(myargv[0], "lcd") == 0)
    {
      char cwd[1024];
      changeDirectory(myargv);
      getcwd(cwd, 1024);
      printf("New Directory is %s\n", cwd); 
      linept = cwd;
    }
    else if (strcmp(myargv[0], "lpwd") == 0)
    {
      /*char cwd[1024];
      getcwd(cwd, 1024);  //figure out the directory
      printf ("Directory = %s \n", cwd); //print it
      linept = cwd;*/
      linept = printDirectory(myargv);
    }
    else if (strcmp(myargv[0], "get") == 0)
    {
      n = write(sock, line, MAX);
      printf("client: wrote n=%d bytes; line=(%s)\n", n, line);

      // response is SIZE of file
      n = read(sock, ans, MAX);
      // server responds "BAD" if either file was not specified or file open failed
      if (strcmp(ans, "BAD") == 0)
      {
        printf("server says get failed\n");
      }
      else
      {
        fp = open(myargv[1], O_WRONLY | O_CREAT, 0744);	// open a file for writing
        length = strtol(ans, (char**)NULL, 10);	// find length of file from server response
        printf("length=%d\n", length);	// print it
        count = 0;
        while (count < length)		// read from the server and write to the file
        {
          n = read(sock, ans, MAX);
	        printf("client: read n=%d bytes\n", n);
          write(fp, ans, n);
	        count += n;
        }
      }
    }
    else if (strcmp(myargv[0], "put") == 0)
    {
    	n = write(sock, line, MAX);	// tell server what we're doing
    	// THIS IS A DIRECT COPY OF SERVER GET
    	// send "BAD" to server if no arg was specified or file open failed
    	if (!myargv[1]) { n = write(sock, "BAD", MAX); }
      fp = open(myargv[1], O_RDONLY);
    	if (fp < 0) { n = write(sock, "BAD", MAX); }
    	sp = &fstat;
      if ( (d = lstat(myargv[1], &fstat)) < 0)
      {
         printf("can't stat %s\n", myargv[1]); 
         n = write(sock, "BAD", MAX);
      }
    	length = sp->st_size;
    	snprintf(line, MAX, "%d", length);
    	// write size to server
    	n = write(sock, line, MAX);
    	printf("client: wrote n=%d bytes; ECHO=[%s]\n", n, line);
    	// write file string to server
	
    	while(d = read(fp, buf, MAX))
    	{
    	  n = write(sock, buf, MAX);
    	  printf("client: wrote n=%d bytes to server\n", n);
    	}
    }
    else if ((strcmp(myargv[0], "quit") == 0) || (strcmp(myargv[0], "exit") == 0))	// changed so "quit" or "exit" will work
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
