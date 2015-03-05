/* 
EE122 Project 1 
================
Phase 1: Client Program 

Team Members
===============
Jordan Makansi
Gary Hoang

*/
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
# include <netinet/in_systm.h>
# include <signal.h>
# include <unistd.h>

/* for clock_gettime */
# include <stdint.h> /* for uint64 definition */
# include <time.h> /* for clock_gettime */

# define BILLION 1000000000L

/* clock_gettime method declarations */
int clock_gettime(clockid_t clk_id, struct timespec *tp);

void error(char *msg)
{
   perror(msg);
   exit(0);
}

typedef struct Inputs
{
  int mode;
  char *ip_addr;
  int portno;
  char *recv_file;
  char *stats_filename;
} Inputs;

void Mode_0(struct Inputs *userInput)
{

  int sockfd, check, size, serv_len, first_packet;
  struct sockaddr_in serv_addr; 
  struct hostent *server;
  FILE *fp, *fp_stat;
  /* for clocking between packets and the entire connection */
  struct timespec start_in_loop, end_in_loop, start, end; 
  uint64_t diff;
  char *buffer;

  size = 1000; 
  buffer = (char *) malloc(size);
  if (buffer == NULL) error("Error: Failed to allcate memory for client buffer");
  serv_len = sizeof(serv_addr);

  clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  /* sockfd is socket file descriptor. This is just returns an integer. */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);    
  if (sockfd < 0) error("ERROR Opening socket"); 

  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server) error("ERROR No host");
  
  /* fill in server struct */
  bzero((char *)&serv_addr, serv_len);
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  /* open our file to write to */
  fp = fopen(userInput -> recv_file, "w"); 
  if (NULL==fp) error("ERROR: File did not open "); 

  check = connect(sockfd, (struct sockaddr *)&serv_addr, serv_len);
  if (check < 0) error("ERROR: connecting");

  fp_stat = fopen(userInput -> stats_filename, "w"); 
  if (NULL == fp_stat) error("ERROR: File did not open ");

  first_packet = 1;
  /* receiving */
  while(1) {
    bzero(buffer, size);

    check = read(sockfd, buffer, size);
    if (check < 0) error("ERROR inital reading from socket");
    if (first_packet == 0) {
      clock_gettime(CLOCK_REALTIME, &end_in_loop);
      /* for stats text file */
      diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + (end_in_loop.tv_nsec - start_in_loop.tv_nsec);
      fprintf(fp_stat, "elapsed time between packets = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
      printf("elapsed time between packets = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
    } else {
      first_packet = 0;
    }

    clock_gettime(CLOCK_REALTIME, &start_in_loop);  
    printf("Packet received");
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); /* if doing this, "End" is not written, but last packet "End" is timed */
  }

  /* tells how big our file is */
  printf("Total size of file.txt = %ld bytes\n", ftell(fp));
  
  check = fclose(fp_stat);
  if (check == EOF) error("Error: Failed to close stats file");
  check = fclose(fp);
  if (check == EOF) error("Error: Failed to close received file");

  clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  printf("elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));

}


void Mode_1(struct Inputs *userInput)
{

  int sockfd, check, size, first_packet, serv_len;
  struct sockaddr_in serv_addr; 
  struct hostent *server; 
  char *buffer;
  FILE *fp, *fp_stat;
  /* for clocking between packets */
  struct timespec start_in_loop, end_in_loop;
  /* for timing the entire connection */
  struct timespec start, end; 
  uint64_t diff;

  /* intializing variables */
  size = 1000; 
  buffer = (char *) malloc(size);
  if (buffer == NULL) error("Error: Failed to allocate memory for client buffer");
  serv_len = sizeof(serv_addr);

  clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
  if (sockfd < 0) error("ERROR Opening socket");

  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server) error("ERROR No host of that name"); 

  bzero((char *)&serv_addr, serv_len);
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  /* file to write to */
  fp = fopen(userInput -> recv_file, "w"); 
  if (NULL==fp) error("ERROR: Received file did not open "); 

  fp_stat = fopen(userInput -> stats_filename, "w");  
  if (NULL==fp_stat) error("ERROR: Stats file did not open ");

  check = sendto(sockfd, "connecting", 20, 0, (struct sockaddr *)&serv_addr, serv_len);
  if (check < 0) error("Error: Sending inital packet");
  first_packet = 1;

  /* receiving */
  while(1) {
    bzero(buffer, size);
    check = recvfrom(sockfd, (void *)buffer, size, 0, (struct sockaddr *)&serv_addr, &serv_len); 
    if (check < 0) error("ERROR: recvfrom failed in client"); 
    if (first_packet == 0) {
      clock_gettime(CLOCK_REALTIME, &end_in_loop);
      /* writing to stats text file */
      diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + (end_in_loop.tv_nsec - start_in_loop.tv_nsec);
      fprintf(fp_stat, "elapsed time between packets = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
      printf("elapsed time between packets = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
    } else {
      first_packet = 0;
    }

    clock_gettime(CLOCK_REALTIME, &start_in_loop); 
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); /* if doing this, "End" is not written, but last packet "End" is timed */
  }

  printf("Total size of file.txt = %ld bytes\n", ftell(fp));
  
  check = fclose(fp_stat);
  if (check == EOF) error("Error: Failed to close stats file");
  check = fclose(fp);
  if (check == EOF) error("Error: Failed to close received file");

  clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  printf("elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
} 

/*
void Mode_2(struct Inputs *userInput)
{
  
  int sockfd, portno, n, recsize;
  struct sockaddr_in serv_addr, cli_addr; 
  int server_len = sizeof(serv_addr);
  struct hostent *server; 
  int size = 1000; 
  char buffer[size];
  FILE *fp, *fp_stat; 
  
  // trying to add make its udp header!!
  char header[8], data[size + 8];
  memset(buffer, 0, size);
  memset(header, 0, 8);
*/
  /* declarations for clock_gettime */
/*  uint64_t diff;
  struct timespec start, end;

  clock_gettime(CLOCK_MONOTONIC, &start); *//* mark start time */

 /* sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  //sockfd is socket file descriptor.  This is just returns an integer.  
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

  // need to check this when we get to actual testing
  // because could be working since we are on same host.
  // let's see what happens when we actually test. */
/*
  bzero ((char*) &cli_addr, sizeof(cli_addr));
  cli_addr.sin_family = AF_INET;   
  cli_addr.sin_addr.s_addr = INADDR_ANY;  
  cli_addr.sin_port = htons(55555);
  n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
  if (0>n){
    error("ERROR: Error on binding");
  } */

  /* opening file to write received messages to */
/*  fp = fopen(userInput -> recv_file, "w");
  if (NULL==fp) { error("ERROR: File did not open "); }

  // opening file to write stats to
  fp_stat = fopen(userInput -> stats_filename, "w");  
  if (NULL==fp_stat) { error("ERROR: File did not open "); }

  // Constructing the UDP Header of our dummy packet
  struct udphdr *udph = (struct udphdr *) (header);
  udph -> uh_sport = 0;//cli_addr.sin_port; //got rid of binding, left as zero and it worked!!
    // could possibly be that is on localhost, so did this.
  udph -> uh_dport = serv_addr.sin_port;
  udph-> uh_ulen = htons(8 + strlen("connecting")); 
  udph-> uh_sum = 0; 

  // copying necessary information to our dummy packet 'data'
  memcpy(data, header, 8);
  memcpy(data+8, "connecting", sizeof("connecting"));
  
  // sending our dummy packet 'data' to the server
  // so it knows where we are
  n = sendto(sockfd, data, sizeof(data),0,(struct sockaddr *)&serv_addr, server_len);
  printf("send initial packet\n");
  
  // cleaning our buffer and data
  memset(buffer, 0, 1000);
  memset(data, 0, 1008);
  
  // While loop to begin receiving
  while(1) {

    // Initializing our time
    struct timespec start_in_loop, end_in_loop;

    // Begin our clock
    clock_gettime(CLOCK_MONOTONIC, &start_in_loop); 

    // Receive a packet
    recsize = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serv_addr, &server_len); // void * ??? 
    if (recsize < 0) { error("ERROR: recvfrom failed in client"); }

    // Only if the buffer we get is not our dummy packet
    // write to our file
    // Tragedy is that we don't have enough time to check ip addresses
    // to ascertain from all broadcasts who our server was again
    // FYI: source address are byte 12-15 of IP address
    if(strcmp("connecting", buffer+28) != 0) {

      // finish clocking
      clock_gettime(CLOCK_MONOTONIC, &end_in_loop);

      // Get information about how long each packet was -- need to fix!
      diff = end_in_loop.tv_sec - start_in_loop.tv_sec + (end_in_loop.tv_nsec - start_in_loop.tv_nsec)/BILLION;
      // write our info to our stats file
      fprintf(fp_stat, "elapsed time of packet = %llu seconds\n", (long long unsigned int) diff);
      // if we received the End terminator, break
      if (strcmp("End", buffer+28) == 0) break;
      // else write to our file
      fputs(buffer+28, fp);  
      // begin again
      bzero(buffer, size);
      }  
    }


  // tells how big our file is
  printf("Total size of file.txt = %d bytes\n", ftell(fp));
  
  // closing all our files
  fclose(fp_stat);
  fclose(fp);*/

  /* finishing clock_gettime */
/*  clock_gettime(CLOCK_MONOTONIC, &end); *//* mark the end time */

/*  diff = end.tv_sec - start.tv_sec + (end.tv_nsec - start.tv_nsec)/BILLION;
  printf("elapsed time of connection = %llu seconds\n", (long long unsigned int) diff);

} */

int main(int argc, char *argv[]) 
{ 
  Inputs userInput; 

  if (argc != 6) {
    error("Not enough input arguments.\n Usage: ./proj1_client <mode> <server_address> <port> <received_filenam> <stats_filename>");
  }

  userInput.mode = atoi(argv[1]);
  userInput.ip_addr = argv[2]; 
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
 /*   case 2:
      Mode_2(&userInput);
      break; */
    default:
      error("ERROR: Invalid Mode");
      break;
  }

  return 0;

}