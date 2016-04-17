/* server.c using TCP */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 1000
#define SIZE_BYTE 4
#define FILENAME_BYTE 20

/* server program called with no argument */
int main(int argc,char **args){
  if (argc != 2) {
    printf("Usage: ./ftps <local-port>\n");
    return -1;
  }
  int sock;                     /* initial socket descriptor */
  int msgsock;                  /* accepted socket descriptor,
                                 * each client connection has a
                                 * unique socket descriptor*/  
  struct sockaddr_in sin_addr; /* structure for socket name setup */
  char recvbuf[BUF_SIZE];               /* buffer for holding read data */

  printf("TCP server waiting for remote connection from clients ...\n");
 
  /*initialize socket connection in unix domain*/
  if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0){
    perror("error opening TCP socket");
    exit(1);
  }
  
  /* construct name of socket to send to */
  sin_addr.sin_family = AF_INET;
  sin_addr.sin_addr.s_addr = INADDR_ANY;
  sin_addr.sin_port = htons(atoi(args[1]));

  /* bind socket name to socket */
  if(bind(sock, (struct sockaddr *)&sin_addr, sizeof(struct sockaddr_in)) < 0) {
    perror("error binding stream socket");
    exit(1);
  }
  
  /* listen for socket connection and set max opened socket connetions to 5 */
  listen(sock, 5);
  
  /* accept a (1) connection in socket msgsocket */ 
  if((msgsock = accept(sock, (struct sockaddr *)NULL, (int *)NULL)) == -1) { 
    perror("error connecting stream socket");

    exit(1);
  } 
  /* generate storage directory */
  char dir[4]="ftp";
  if(mkdir(dir,0777)== -1){
    int err = errno;
    if(err != EEXIST){             
      perror("error create storage directory");
      exit(1);
    }  
  }
  
  /* put all zeros in buffer (clear) */
  bzero(recvbuf,BUF_SIZE);
  
  /* recieve bytes for filesize */
  int bytes = SIZE_BYTE;
  unsigned long filesize;
  if(recv(msgsock,recvbuf,bytes,MSG_WAITALL) < SIZE_BYTE){
    printf("error sending file size");
    exit(1);
  }      
  memcpy(&filesize,recvbuf,bytes);
  filesize = ntohl(filesize);

  /* recieve bytes for filename */
  bytes = FILENAME_BYTE;
  char *filename;
  bzero(recvbuf,BUF_SIZE);
  if(recv(msgsock, recvbuf, bytes,MSG_WAITALL) < FILENAME_BYTE){
    printf("error sending file name");
    exit(1);
  }  
  filename = malloc(strlen(recvbuf) + strlen(dir) + 2); 
  strcpy(filename, dir);
  strcat(filename, "/");
  strcat(filename, recvbuf);
  
  /* open the file */
  FILE *fo;
  if((fo = fopen(filename,"w")) == NULL){
    perror("error creating storage file");
    exit(1);
  }
  
  /* receive & write file */
  unsigned long remain = filesize;
  bzero(recvbuf,BUF_SIZE);
  int recvbytes;
  while(remain >0 ){
    recvbytes = (remain< BUF_SIZE)?remain:BUF_SIZE;
    bytes = recv(msgsock, recvbuf, recvbytes, 0);
    if(bytes == -1 ){
      perror("error recieving file");
      exit(1);
    }	
    if(bytes == 0){
      fprintf(stderr,"error: Connection lost");	
      exit(1);		
    }
    remain -= bytes;
    if (fwrite(recvbuf,1,bytes,fo)!=bytes){
      fprintf(stderr,"error: File write error");
      exit(1);
    }
  }
  fclose(fo);
  
  /* close all connections and remove socket file */
  close(msgsock);
  close(sock);
}

