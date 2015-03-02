/* EE122 Client Program */
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 
# include <time.h>

/* for clock_gettime */
# include <stdio.h>  /* for printf */
# include <stdint.h> /* for uint64 definition */
# include <stdlib.h> /* for exit() definition */
# include <time.h> /* for clock_gettime */

# define BILLION 1000000000L

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
  int size = 1000;  // what if pass in this - so assume client knows
  // note that when I tried to send at 10,000 bytes and 5,000 byes
  // connection reset by peer error.
  // also if I put size at 3000, get open_stackdumpfile
  char buffer[size];

  /* wipe our stat file */
  // possible w/o memory dying out?

  /* declarations for clock_gettime */
  uint64_t diff;
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */

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

  // file to write to
  FILE *fp; 
  fp = fopen(userInput -> recv_file, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write to was opened \n");
  if (NULL==fp){
    error("ERROR: File did not open ");
  }

  bzero(buffer, size); // why is I take this out, stack dump

  n = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (0 < n) {
    error("ERROR connecting");
  }
  printf("Finished connecting\n");

  FILE *fp_stat; 
  fp_stat = fopen(userInput -> stats_filename, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write stats to was opened \n");
  if (NULL==fp_stat) error("ERROR: File did not open ");

  // receiving
  while(1) {
    struct timespec start_in_loop, end_in_loop;
    bzero(buffer, size);
    clock_gettime(CLOCK_MONOTONIC, &start_in_loop);  
    n = read(sockfd, buffer, size);
    printf("buffer: %s\n", buffer);
    if (n < 0) error("ERROR inital reading from socket"); 
    clock_gettime(CLOCK_MONOTONIC, &end_in_loop);

    // for stats text file
    diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + end_in_loop.tv_nsec - start_in_loop.tv_nsec;
    fprintf(fp_stat, "elapsed time of packet = %llu nanoseconds\n\n", (long long unsigned int) diff);
    printf("elapsed time of packet = %llu nanoseconds\n\n", (long long unsigned int) diff);
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); // if doing this, "End" is not written, but last packet "End" is timed
    //printf("condition: %d\n", condition);
  }

  // tells how big our file is
  int len = ftell(fp); // tells where our file pointer is in file
  printf("Total size of file.txt = %d bytes\n\n", len);
  
  fclose(fp_stat);
  fclose(fp);


  printf("wrote to the file and closed it.\n");

  /* now will try POSIX-standard clock_gettime function - someone said is not ansi C*/ 
  /* finishing clock_gettime */
  clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  printf("elapsed time of connection = %llu nanoseconds\n", (long long unsigned int) diff);

}

void Mode_1(struct Inputs *userInput)
{

  int sockfd, portno, n, recsize;
  struct sockaddr_in serv_addr; 
  int server_len = sizeof(serv_addr);
  struct hostent *server; // in header file 
  int size = 1000; 
  char buffer[size];

  /* declarations for clock_gettime */
  uint64_t diff;
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //sockfd is socket file descriptor.  This is just returns an integer.  
  if (0>sockfd){
    error("ERROR Opening socket");
  }
  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server){
    error("ERROR No host of that name");  
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  // file to write to
  FILE *fp; 
  fp = fopen(userInput -> recv_file, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write to was opened \n");
  if (NULL==fp){
    error("ERROR: File did not open ");
  }

  bzero(buffer, size); // why is I take this out, stack dump

  FILE *fp_stat; 
  fp_stat = fopen(userInput -> stats_filename, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write stats to was opened \n");
  if (NULL==fp_stat) error("ERROR: File did not open ");

  n = sendto(sockfd,"connecting",sizeof("connecting"),0,(struct sockaddr *)&serv_addr, server_len);

  // receiving
  while(1) {
    struct timespec start_in_loop, end_in_loop;
    bzero(buffer, size);
    clock_gettime(CLOCK_MONOTONIC, &start_in_loop);  

    recsize = recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &server_len); // void * ??? 
    if (recsize < 0) {
          error("ERROR: recvfrom failed in client");
    }

    printf("buffer: %s\n", buffer); 
    clock_gettime(CLOCK_MONOTONIC, &end_in_loop);

    // for stats text file
    diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + end_in_loop.tv_nsec - start_in_loop.tv_nsec;
    fprintf(fp_stat, "elapsed time of packet = %llu nanoseconds\n", (long long unsigned int) diff);
    printf("elapsed time of packet = %llu nanoseconds\n", (long long unsigned int) diff);
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); // if doing this, "End" is not written, but last packet "End" is timed
    //printf("condition: %d\n", condition);
  }

  // tells how big our file is
  int len = ftell(fp); // tells where our file pointer is in file
  printf("Total size of file.txt = %d bytes\n", len);
  
  fclose(fp_stat);
  fclose(fp);

  printf("wrote to the file and closed it.\n");

  /* finishing clock_gettime */
  clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  printf("elapsed time of connection = %llu nanoseconds\n", (long long unsigned int) diff);
}

void Mode_2(struct Inputs *userInput)
{

  int sockfd, portno, n, recsize;
  struct sockaddr_in serv_addr; 
  int server_len = sizeof(serv_addr);
  struct hostent *server; // in header file 
  int size = 1000; 
  char buffer[size];

  /* declarations for clock_gettime */
  uint64_t diff;
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */

  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  //sockfd is socket file descriptor.  This is just returns an integer.  
  if (0>sockfd){
    error("ERROR Opening socket");
  }
  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server){
    error("ERROR No host of that name");  
  }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  // file to write to
  FILE *fp; 
  fp = fopen(userInput -> recv_file, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write to was opened \n");
  if (NULL==fp){
    error("ERROR: File did not open ");
  }

  bzero(buffer, size); // why is I take this out, stack dump

  FILE *fp_stat; 
  fp_stat = fopen(userInput -> stats_filename, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write stats to was opened \n");
  if (NULL==fp_stat) error("ERROR: File did not open ");

  n = sendto(sockfd,"connecting",sizeof("connecting"),0,(struct sockaddr *)&serv_addr, server_len);
  printf("send initial packet\n");
  // receiving
  while(1) {
    struct timespec start_in_loop, end_in_loop;
    bzero(buffer, size);
    clock_gettime(CLOCK_MONOTONIC, &start_in_loop);  
    printf("in while loop\n");
    recsize = recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &server_len); // void * ??? 
    if (recsize < 0) {
          error("ERROR: recvfrom failed in client");
    }

    printf("buffer: %s\n", buffer); 
    clock_gettime(CLOCK_MONOTONIC, &end_in_loop);

    // for stats text file
    diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + end_in_loop.tv_nsec - start_in_loop.tv_nsec;
    fprintf(fp_stat, "elapsed time of packet = %llu nanoseconds\n", (long long unsigned int) diff);
    printf("elapsed time of packet = %llu nanoseconds\n", (long long unsigned int) diff);
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); // if doing this, "End" is not written, but last packet "End" is timed
    //printf("condition: %d\n", condition);
  }

  // tells how big our file is
  int len = ftell(fp); // tells where our file pointer is in file
  printf("Total size of file.txt = %d bytes\n", len);
  
  fclose(fp_stat);
  fclose(fp);

  printf("wrote to the file and closed it.\n");

  /* finishing clock_gettime */
  clock_gettime(CLOCK_MONOTONIC, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + end.tv_nsec - start.tv_nsec;
  printf("elapsed time of connection = %llu nanoseconds\n", (long long unsigned int) diff);
}

int main(int argc, char *argv[]) // Three arguments provided:  client host port (host is your localhost if both processes are on your own machine)
{ 
  if (argc != 6) {
    error("Not enough input arguments.\n Usage: ./proj1_client <mode> <server_address> <port> <received_filenam> <stats_filename>");
  }
  
  struct Inputs userInput;

  userInput.mode = atoi(argv[1]);
  userInput.ip_addr = argv[2];   // float or int?  
  userInput.portno = atoi(argv[3]);
  userInput.recv_file = argv[4];
  userInput.stats_filename = argv[5];

  switch(userInput.mode){

  case 0: 
    Mode_0(&userInput);
    break;
  case 1: 
    Mode_1(&userInput);
    break;
  case 2:
    Mode_2(&userInput);
    break;
  default:
    error("ERROR: Invalid Mode");
    break;
  }

}