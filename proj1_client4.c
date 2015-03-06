/* 
EE122 Project 1
Phase 4 Client Program
*/
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 

/* for screen blinking */
# include <windows.h>
# include <winuser.h>
# include <stdlib.h> 

/* for catching signal */
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>

/* for timeout and select */
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>


typedef struct Inputs
{
  char *ip_addr;
  int portno;
  int blink_five;
} Inputs;

/* Define the function to be called when ctrl-c (SIGINT) 
 signal is sent to process */
void (*signal (int sig, void (*func)(int)))(int);

void signal_callback_handler(int signum) {
  /* Clean up and close stuff here */
  printf("ctrl-c detected. Closing client.\n");
  /* Terminate program */
  exit(signum);
}

void error(char *msg)
{
   perror(msg);
   exit(0);
}

void cli1(struct Inputs *userInput)
{
  int sockfd, check, size, serv_len;
  struct sockaddr_in serv_addr; 
  struct hostent *server; 
  char *buffer;
  HWND hwnd;
  FLASHWINFO flashInfo;

  /* initializing variables */
  hwnd = GetConsoleWindow();
  flashInfo.cbSize = sizeof(FLASHWINFO);
  flashInfo.hwnd = hwnd;
  flashInfo.dwFlags = FLASHW_ALL;
  flashInfo.uCount = 5;
  flashInfo.dwTimeout = 100;

  size = 200;  
  buffer = (char *) malloc(200);
  if (buffer == NULL) error("Error: Failed to allocate memory for client buffer");
  bzero(buffer, size); 
  serv_len = sizeof(serv_addr);

  /* socket creation and binding */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
  if (sockfd < 0) error("ERROR Opening socket"); 
  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server) error("ERROR No host");

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  check = sendto(sockfd, "connecting", size, 0, (struct sockaddr *)&serv_addr, serv_len);
  if (check < 0) error("Error: Sending inital packet");

  /* receiving */
  while(1) {
    bzero(buffer, size);
    check = recvfrom(sockfd, buffer, size, 0, (struct sockaddr *)&serv_addr, &serv_len); 
    if (check < 0) error("ERROR: recvfrom failed in client"); 
    printf("RECEIVED: %s\n", buffer);
    FlashWindowEx(&flashInfo); 
  }

}

void cli2(struct Inputs *userInput)
{
  int sockfd, check, size, serv_len;
  struct sockaddr_in serv_addr; 
  struct hostent *server; 
  char *buffer;
  struct timeval tv;
  HWND hwnd; 
  fd_set readfds;
  FLASHWINFO flashInfo;

  /* initializing variables */
  hwnd = GetConsoleWindow();
  flashInfo.cbSize = sizeof(FLASHWINFO);
  flashInfo.hwnd = hwnd;
  flashInfo.dwFlags = FLASHW_ALL;
  flashInfo.uCount = 5;
  flashInfo.dwTimeout = 100;
  size = 200;  
  buffer = (char *) malloc(200);
  if (buffer == NULL) error("Error: Failed to allocate memory for client buffer");
  bzero(buffer, size); 
  serv_len = sizeof(serv_addr); 

  /* socket creation and binding */
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
  if (sockfd < 0) error("ERROR Opening socket"); 
  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server) error("ERROR No host");

  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);

  check = sendto(sockfd, "connecting", size, 0, (struct sockaddr *)&serv_addr, serv_len);
  if (check < 0) error("Error: Sending inital packet"); 

  /* clear the set ahead of time */
  FD_ZERO(&readfds); 
  /* add descriptor */
  FD_SET(sockfd, &readfds); 
  /* write in tv */
  tv.tv_sec = 5;
  tv.tv_usec = 0; 

  /* receiving */
  while(1) {
    bzero(buffer, size);
    check = select(sockfd+1, &readfds, NULL, NULL, &tv);
    if(check < 0 ) {
      error("Error: error occured in select()");
    } else if (check == 0) { /* reached our timeout */
      printf("Did not receive anything in the 5 second period\n");
    } else {
      if (FD_ISSET(sockfd, &readfds)) {
        check = recvfrom(sockfd, buffer, size, 0, (struct sockaddr *)&serv_addr, &serv_len); 
        if (check < 0) error("ERROR: recvfrom failed in client"); 
        printf("RECEIVED: %s\n", buffer);
        FlashWindowEx(&flashInfo); 
      }
    } 
  /* clear the set ahead of time */
  FD_ZERO(&readfds); 
  /* add descriptor */
  FD_SET(sockfd, &readfds);
  /* write in tv */
  }
  printf("Client closed.\n"); 
} 


int main(int argc, char *argv[]) 
{ 
  Inputs userInput;
  char *flag, *c;
  int compare;
  /* Register signal and signal handler */
  signal(SIGINT, signal_callback_handler);

  if (argc > 4) {
    error("Incorrect input arguments.\n Usage: ./proj1_client <address> <port> [ -b ]");
  }
  if (argc == 4) {
    flag = argv[3];
    c = &flag[1];
    compare = strcmp("b", c);
    if (compare == 0 ) {
      userInput.blink_five = 1;
    } else {
      userInput.blink_five = 0;
    }
  } 

  userInput.ip_addr = argv[1];  
  userInput.portno = atoi(argv[2]);

  if (userInput.blink_five == 0) {
    cli1(&userInput);
  } else {
    cli2(&userInput);
  } 
  return 0;
}