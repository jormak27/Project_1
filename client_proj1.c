# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 

void error(char *msg)
{
   perror(msg);
   exit(0);
}

struct Inputs
{
  int mode;
  char *ip_addr;
  int portno;
  char *recv_file;
  char *stats_filename;
};

void Mode_0(struct Inputs *userInput)
{

  int sockfd, portno, n;
  struct sockaddr_in serv_addr; 
  struct hostent *server; // in header file 
  int size = 2000;  // what if pass in this - so assume client knows
  // note that when I tried to send at 10,000 bytes and 5,000 byes
  // connection reset by peer error.
  // also if I put size at 3000, get open_stackdumpfile
  char *buffer;


  sockfd = socket(AF_INET, SOCK_STREAM, 0);  //sockfd is socket file descriptor.  This is just returns an integer.  
  if (0>sockfd){
    error("ERROR Opening socket");
  }
  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server){
    error("ERROR No host");  
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  FILE *fp; 
  fp = fopen(userInput -> recv_file, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file was opened \n");
  if (NULL==fp){
    error("ERROR: File did not open ");
  }
  //bzero(buffer, sizeof(buffer)); // need to bzero buffer before it works!!!
  bzero(buffer, size);
  n = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (0 < n) {
    error("ERROR connecting");
  }
  //printf("Finished connecting");

  // receiving


/* 1. just having multiple reads */
/*
  n = read(sockfd, buffer, size);
  if (n < 0) error("ERROR inital reading from socket");
  //n = read(sockfd, buffer, sizeof(buffer));
  //if (n < 0) error("ERROR inital reading from socket");
  //printf("%s\n", buffer);
  fputs(buffer, fp); 
  //fprintf(fp, "%s", buffer ); // both work
  bzero(buffer, size);
  n = read(sockfd, buffer, size);
  if (n < 0) error("ERROR reading from socket");
  //printf("buffer: %s\n", buffer);
  fputs(buffer, fp);
  bzero(buffer, size);
  n = read(sockfd, buffer, size);
  if (n < 0) error("ERROR reading from socket");
  printf("buffer: %s\n", buffer);
  if (strcmp("End", buffer) == 0) printf("Success"); */

/* Final Solution that works!!! */
/* 5. Making it terminate on "End" */ 
while(1) {
  bzero(buffer, size);
  n = read(sockfd, buffer, size);
  //printf("buffer: %s\n", buffer);
  if (n < 0) error("ERROR inital reading from socket");
  if (strcmp("End", buffer) == 0) break;
  fputs(buffer, fp);  
}

/* 4. Trying to send some message that says "End" */
/*  n = read(sockfd, buffer, size);
  printf("%s\n", buffer);
  if (buffer == "End") {
    printf("here");  
  } */
  
/* 3. trying to use read() as condition */
/*  while(n > 0) { // condition should be something else
  //while(fprintf(fp, "%s", buffer) > 0) {
  // fputs(buffer, fp);
  // while(feof(fp) != 0) {
    printf("inside the while loop");
    printf("buffer: %s\n", buffer);
    fputs(buffer, fp); 
    n = read(sockfd, buffer, size);
    if (n < 0) error("ERROR reading from socket");
    } */

/* 2. trying to use fputs as condition */
/*
  while(fputs(buffer, fp) > 0) {
    n = read(sockfd, buffer, size);
    if (n < 0) error("ERROR reading from socket");
  } */

/*  n = read(sockfd, buffer, sizeof(buffer));
  if (n < 0) error("ERROR reading from socket");
  fputs(buffer, fp); */
 // ended up writing twice */

  printf("wrote to the file");

/*
  printf("User input: \n\n%d %s %d %s %s \n", 
  userInput -> mode,
  userInput -> ip_addr,
  userInput -> portno,
  userInput -> recv_file, 
  userInput -> stats_filename); */

}



int main(int argc, char *argv[]) // Three arguments provided:  client host port (host is your localhost if both processes are on your own machine)
{
  if (argc != 6) {
    error("Not enough input arguments.\n Usage: ./proj1_client <mode> <server_address> <port> <received_filenam> <stats_filename>");
  }
  
  struct Inputs userInput;

  userInput.mode = atoi(argv[1]);
  userInput.ip_addr = argv[2];   // float or int?  
  userInput.portno = atoi(argv[3]);
  userInput.recv_file = argv[4];
  userInput.stats_filename = argv[5];

  if (0==userInput.mode){
    Mode_0(&userInput);
  }

}