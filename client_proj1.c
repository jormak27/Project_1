/* EE122 Client Program */
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <netinet/udp.h>
# include <netinet/ip.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 
# include <time.h>
# include <netinet/in_systm.h>

/* for clock_gettime */
# include <stdio.h>  /* for printf */
# include <stdint.h> /* for uint64 definition */
# include <stdlib.h> /* for exit() definition */
# include <time.h> /* for clock_gettime */

# define BILLION 1000000000L

# define TRUE 1
# define FALSE 0

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
  char buffer[size];

  /* declarations for clock_gettime */
  long long unsigned int diff;
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
    diff = diff * .000000001;
    fprintf(fp_stat, "elapsed time of packet = %llu seconds\n\n", (long long unsigned int) diff);
    printf("elapsed time of packet = %llu seconds\n\n", (long long unsigned int) diff);
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); // if doing this, "End" is not written, but last packet "End" is timed
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
  diff = diff * .000000001;
  printf("elapsed time of connection = %llu seconds\n", (long long unsigned int) diff);

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
    diff = diff * .000000001;
    fprintf(fp_stat, "elapsed time of packet = %llu seconds\n", (long long unsigned int) diff);
    printf("elapsed time of packet = %llu seconds\n", (long long unsigned int) diff);
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
  diff = diff * .000000001;
  printf("elapsed time of connection = %llu seconds\n", (long long unsigned int) diff);
}

void Mode_2(struct Inputs *userInput)
{
  
  // possibly header is not coming through correctly??
  int sockfd, portno, n, recsize;
  struct sockaddr_in serv_addr, cli_addr; 
  int server_len = sizeof(serv_addr);
  struct hostent *server; 
  int size = 1000; 
  char buffer[size];
  int so_broadcast;
  
  // trying to add make its udp header!!
  char header[8], data[size + 8];
  memset(buffer, 0, size);
  memset(header, 0, 8);

  /* declarations for clock_gettime */
  uint64_t diff;
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start); /* mark start time */

  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  //sockfd is socket file descriptor.  This is just returns an integer.  
  //sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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

  bzero ((char*) &cli_addr, sizeof(cli_addr));
  cli_addr.sin_family = AF_INET;   
  cli_addr.sin_addr.s_addr = INADDR_ANY;  
  cli_addr.sin_port = htons(55555);
  n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
  if (0>n){
    error("ERROR: Error on binding");
  }
/*
  so_broadcast = TRUE; 
  n = setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &so_broadcast, sizeof(so_broadcast));
  if(n<0) {
    error("ERROR setsockopt did not work");
  }
*/
  // file to write to
  FILE *fp; 
  fp = fopen(userInput -> recv_file, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write to was opened \n");
  if (NULL==fp){
    error("ERROR: File did not open ");
  }

  FILE *fp_stat; 
  fp_stat = fopen(userInput -> stats_filename, "w");  // This assumes that the file is in the same directory in which we're runing this program 
  printf("file to write stats to was opened \n");
  if (NULL==fp_stat) error("ERROR: File did not open ");

    //UDP header
    struct udphdr *udph = (struct udphdr *) (header);
    udph -> uh_sport = cli_addr.sin_port; // tried to bind - left as zero and it worked!!
    udph -> uh_dport = serv_addr.sin_port;
    udph-> uh_ulen = htons(8 + strlen("connecting")); 
    udph-> uh_sum = 0; 

    memcpy(data, header, 8);
    memcpy(data+8, "connecting", sizeof("connecting"));
  
  n = sendto(sockfd, data, sizeof(data),0,(struct sockaddr *)&serv_addr, server_len);
  //n = sendto(sockfd, "connecting", sizeof("connecting"), 0,(struct sockaddr *)&serv_addr, server_len);
  printf("send initial packet\n");
  
  memset(buffer, 0, 1000);
  memset(data, 0, 1008);
  // receiving

  while(1) {
    struct timespec start_in_loop, end_in_loop;
    clock_gettime(CLOCK_MONOTONIC, &start_in_loop);  
    recsize = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &server_len); // void * ??? 
    if (recsize < 0) {
          error("ERROR: recvfrom failed in client");
    }

    // should probably add ip address to confirm!!!
    // ????
    printf("BUFFER %s\n", buffer+28);
    if(strcmp("connecting", buffer+28) != 0) {

    // client only works on first time through.
/*    char port[2];
    memcpy(port, buffer+20, 2);
    uint16_t recv_portno = ntohs(*port); 
    printf("received source port number: %d\n", recv_portno);
    memcpy(port, buffer+22, 2);
    uint16_t dest_portno = ntohs(*port);
    printf("received destination port number: %d\n", dest_portno);
*/
    // ip address?
    // source address are byte 12-15 - maybe have to go by this?
      clock_gettime(CLOCK_MONOTONIC, &end_in_loop);

      // for stats text file
      diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + end_in_loop.tv_nsec - start_in_loop.tv_nsec;
      fprintf(fp_stat, "elapsed time of packet = %llu nanoseconds\n", (long long unsigned int) diff);
      //printf("elapsed time of packet = %llu nanoseconds\n", (long long unsigned int) diff);
      if (strcmp("End", buffer+28) == 0) break;
      // client is not picking "End"!!!
      fputs(buffer+28, fp);  
      bzero(buffer, size);
      }  
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