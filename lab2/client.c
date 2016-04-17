/* client.c using TCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>

#define BUF_SIZE 1000
#define SIZE_BYTE 4
#define FILENAME_BYTE 20

/* client program called with host name where server is run */
int main(int argc, char *args[])
{
  int sock;                     /* initial socket descriptor */
  struct sockaddr_in sin_addr; /* structure for socket name 
                                 * setup */
  struct hostent *hp;
  
  if(argc != 4) {
    printf("Usage: ./ftpc <remote-IP> <remote-port> <local file>\n");
    exit(1);
  }
  
  /* initialize socket connection in unix domain */
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("error opening socket");
    exit(1);
  }

  hp = gethostbyname(args[1]);
  if(hp == 0) {
    fprintf(stderr, "%s: unknown host\n", args[1]);
    exit(2);
  }

  /* construct name of socket to send to */
  bcopy((void *)hp->h_addr, (void *)&sin_addr.sin_addr, hp->h_length);
  sin_addr.sin_family = AF_INET;
  sin_addr.sin_port = htons(atoi(args[2])); /* fixed by adding htons() */
  
  /* establish connection with server */
  if(connect(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
    close(sock);
    perror("error connecting stream socket");
    exit(1);
  }
  
  /* open the file */
  FILE *fi;
  if((fi = fopen(args[3],"r")) == NULL){
    perror("error finding local file to transfer");
    exit(1);
  }
  struct stat st;
  stat(args[3], &st);
  unsigned long filesize = (long)st.st_size;

  /* write file size & name to sock */
  char sdbuf[BUF_SIZE];
  int bytes = SIZE_BYTE;
  unsigned long Nsize = htonl(filesize);
  memcpy(sdbuf, &Nsize, bytes);
  if(send(sock, sdbuf, bytes, 0) < 0){
    perror("error sending file size");
    exit(1);
  }
  if(strlen(args[3])>20){
    printf("error: Please use filename length <= 20");
    exit(1);
  }  
  bytes = FILENAME_BYTE;
  memcpy(sdbuf,args[3],strlen(args[3])+1);
  if(send(sock, sdbuf, bytes, 0)<0){
    perror("error sending file name");
    exit(1);
  }
  
  /* read & send file */
  unsigned long remain = filesize;
  while(remain >= BUF_SIZE){
    if(fread(sdbuf, 1, BUF_SIZE, fi) != BUF_SIZE){
      printf("error file reading failure");
      exit(1);
    }
    if(send(sock, sdbuf, BUF_SIZE, 0)<0){
      perror("error sending file");
      exit(1);
    }
    remain -=BUF_SIZE;  
  }

  /* read & send the remaining bytes */
  if(fread(sdbuf, 1, remain, fi) != remain){
    printf("error file reading failure");
    exit(1);
  }
  if(send(sock, sdbuf, remain, 0) <0){
    perror("error sending file");
    exit(1);
  }
  fclose(fi);
  close(sock);
  return 0;
}

