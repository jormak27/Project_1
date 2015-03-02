/* EE122 Client Program */
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 

// for screen blinking
# pragma comment (lib, "User32.lib")
# include <windows.h>
# include <stdlib.h> 

// for catching signal
# include <unistd.h>
# include <stdio.h>
# include <stdlib.h>
# include <signal.h>

// Define the function to be called when ctrl-c (SIGINT) 
// signal is sent to process
void (*signal (int sig, void (*func)(int)))(int);

void signal_callback_handler(int signum) {
  printf("Caught signal %d\n", signum);
  // Clean up and close stuff here
  printf("Magic time\n");
  // Terminate program
  exit(signum);
}

void error(char *msg)
{
   perror(msg);
   exit(0);
}

struct Inputs
{
  char *ip_addr;
  int portno;
  int blink_five;
};

void cli1(struct Inputs *userInput)
{

  int sockfd, portno, n, size;
  struct sockaddr_in serv_addr; 
  struct hostent *server; // in header file 

  // initializing variables
  HWND hwnd = GetConsoleWindow();
  FLASHWINFO flashInfo = { sizeof(FLASHWINFO), hwnd, FLASHW_ALL, 5, 100 };
  size = 1000;  
  char buffer[size];
  bzero(buffer, size); 

  // socket creation and binding
  sockfd = socket(AF_INET, SOCK_STREAM, 0);  
  if (0>sockfd){ error("ERROR Opening socket"); }
  server = gethostbyname(userInput -> ip_addr);
  if (NULL == server){ error("ERROR No host"); }
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  serv_addr.sin_port = htons(userInput -> portno);
  n = connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  if (0 < n) { error("ERROR connecting"); }

  // receiving
  while(1) {
    bzero(buffer, size);
    n = read(sockfd, buffer, size);
    printf("buffer: %s\n", buffer);
    if (n < 0) error("ERROR reading from socket");
    FlashWindowEx(&flashInfo); 
  }

}

void cli2(struct Inputs *userInput)
{
  // just copy from cli1, but modify such that
  // can buffer packets and blink and get every 5 
  // seconds
}


int main(int argc, char *argv[]) // Three arguments provided:  client host port (host is your localhost if both processes are on your own machine)
{ 
  // Register signal and signal handler
  signal(SIGINT, signal_callback_handler);


  if (argc > 4) {
    error("Incorrect input arguments.\n Usage: ./proj1_client <address> <port> [ -b ]");
  }

  struct Inputs userInput;
  if (argc == 4) {
    char *flag = argv[3];
    char *c = &flag[1]; // why like this I don't know
    printf("character is : %s\n", c);
    int n = strcmp("b", c);
    if (n == 0 ) {
      userInput.blink_five = 1;
    } else {
      userInput.blink_five = 0;
    }
  } 

  userInput.ip_addr = argv[1];   // float or int?  
  userInput.portno = atoi(argv[2]);

  if (userInput.blink_five == 0) {
    cli1(&userInput);
  } else {
    cli2(&userInput);
  }
  printf("blink_five is : %d\n", userInput.blink_five);
}