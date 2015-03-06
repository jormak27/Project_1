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
    if (strcmp("End", buffer) == 0) break;
    fputs(buffer, fp); /* if doing this, "End" is not written, but last packet "End" is timed */
  }

  /* tells how big our file is */
  printf("Total size of file.txt = %ld bytes\n", ftell(fp));

  clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  printf("elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
  fprintf(fp_stat, "elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));

  check = fclose(fp_stat);
  if (check == EOF) error("Error: Failed to close stats file");
  check = fclose(fp);
  if (check == EOF) error("Error: Failed to close received file");

}

void Mode_1(struct Inputs *userInput)
{

  int sockfd, check, size, first_packet, serv_len;
  struct sockaddr_in serv_addr; 
  struct hostent *server; 
  char *buffer;
  FILE *fp, *fp_stat;
  /* for clocking between packets and the entire connection */
  struct timespec start_in_loop, end_in_loop, start, end;
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
    if (check < 0) error("Error: Recvfrom failed"); 
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
  
  clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  printf("elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
  fprintf(fp_stat, "elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));

  check = fclose(fp_stat);
  if (check == EOF) error("Error: Failed to close stats file");
  check = fclose(fp);
  if (check == EOF) error("Error: Failed to close received file");

} 


void Mode_2(struct Inputs *userInput)
{
  
  int sockfd, check, size, serv_len, first_packet;
  struct sockaddr_in serv_addr, cli_addr; 
  struct hostent *server; 
  char *buffer, header[8], *data;
  FILE *fp, *fp_stat; 
  uint64_t diff;
  struct timespec start_in_loop, end_in_loop, start, end;
  struct udphdr *udph;
  
  /* trying to add make its udp header!! */
  serv_len = sizeof(serv_addr);
  size = 1000; 
  buffer = (char *) malloc(size);
  if (buffer == NULL) error("Error: Failed to allocate buffer");
  data = (char *) malloc(size + 8);
  if (data == NULL) error("Error: Failed to allocate data");

  /* clean everything up */
  memset(buffer, 0, size);
  memset(header, 0, 8);
  memset(data, 0, size+8);

  clock_gettime(CLOCK_REALTIME, &start); /* mark start time */

  sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP); 
  if (sockfd < 0) error("ERROR Opening socket"); 

  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server) error("ERROR No host of that name");  

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  /* need to check this when we get to actual testing
   because could be working since we are on same host.
   let's see what happens when we actually test. */
  bzero ((char*) &cli_addr, sizeof(cli_addr));
  cli_addr.sin_family = AF_INET;   
  cli_addr.sin_addr.s_addr = INADDR_ANY;  
  cli_addr.sin_port = htons(55555);
  check = bind(sockfd, (struct sockaddr *) &serv_addr, serv_len);
  if (check < 0) error("ERROR: Error on binding");

  /* opening file to write received messages to */
  fp = fopen(userInput -> recv_file, "w");
  if (NULL==fp) error("ERROR: File did not open "); 

  /* opening file to write stats tov*/
  fp_stat = fopen(userInput -> stats_filename, "w");  
  if (NULL==fp_stat) error("ERROR: File did not open ");

  /* Constructing the UDP Header of our dummy packet */
  udph = (struct udphdr *) (header);
  udph -> uh_sport = 0;/*cli_addr.sin_port;..got rid of binding, left as zero and it worked!! */
  udph -> uh_dport = serv_addr.sin_port;
  udph-> uh_ulen = htons(8 + strlen("connecting")); 
  udph-> uh_sum = 0; 

  /* copying necessary information to our dummy packet 'data' */
  memcpy(data, header, 8);
  memcpy(data+8, "connecting", sizeof("connecting"));
  
  /* sending our dummy packet 'data' to the server so it knows where we are */
  check = sendto(sockfd, data, size+8,0,(struct sockaddr *)&serv_addr, serv_len);
  if (check < 0) error("Error: Dummy Packet failed to send");
  
  /* cleaning our buffer and data */
  memset(buffer, 0, 1000);
  memset(data, 0, 1008);
  
  /* While loop to begin receiving */
  first_packet = 1;
  while(1) {
    clock_gettime(CLOCK_REALTIME, &start_in_loop); 

    /* Receive a packet */
    check = recvfrom(sockfd, buffer, size, 0, (struct sockaddr *)&serv_addr, &serv_len); 
    if (check < 0) { error("ERROR: recvfrom failed in client"); }
    /* Only if the buffer we get is not our dummy packet write to our file
    Tragedy is that we don't have enough time to check ip addresses
    to ascertain from all broadcasts who our server was again
    FYI: source address are byte 12-15 of IP address */
    if(strcmp("connecting", buffer+28) != 0) {
      if (first_packet == 0) {
        clock_gettime(CLOCK_REALTIME, &end_in_loop);
        /* writing to stats text file */
        diff = BILLION * (end_in_loop.tv_sec - start_in_loop.tv_sec) + (end_in_loop.tv_nsec - start_in_loop.tv_nsec);
        fprintf(fp_stat, "Time between packets = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
        printf("Time between packets = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
       } else {
        first_packet = 0;
       }

      clock_gettime(CLOCK_REALTIME, &start_in_loop); 
      if (strcmp("End", buffer+28) == 0) break;
      fputs(buffer+28, fp);  
      bzero(buffer, size);
      }  
    }

  /* tells how big our file is */
  printf("Total size of file.txt = %ld bytes\n", ftell(fp));

  clock_gettime(CLOCK_REALTIME, &end); /* mark the end time */

  diff = BILLION * (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec);
  printf("elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));
  fprintf(fp_stat, "elapsed time of connection = %lu nanoseconds or %d seconds (rounded)\n", (long unsigned int) diff, (unsigned int)(diff/BILLION));

  
  /* closing all our files */
  fclose(fp_stat);
  fclose(fp);
} 

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
    case 2:
      Mode_2(&userInput);
      break; 
    default:
      error("ERROR: Invalid Mode");
      break;
  }

  return 0;

}