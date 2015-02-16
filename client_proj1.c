# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> // fixes the bcopy!! because hostent defined in here. For some reason was missing..

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
  struct hostent *server, *gethostbyname();  // in header file 
  // had to declare *gethostbyname() for it to work
  // although in problem example client given in book
  // no such declaration - is it because we used localhost?
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
  
  /* Fixed problem of bcopy because struct hostent not loaded in */
  /* check this: so I believe can do this alternatively, but should still with book.
  http://inst.eecs.berkeley.edu/~ee122/sp07/Socket%20Programming.pdf
  page 31 - instead of bcopy, they use*/
  //serv_addr.sin_addr.s_addr = inet_addr(userInput -> ip_addr);
  

  //printf("%s\n", (char *)server->h_addr); // to test, but it prints out @, but otherwise everything seems to go through
  bcopy((char *) server->h_addr, (char *) &serv_addr.sin_addr.s_addr, server->h_length);
  
  serv_addr.sin_port = htons(userInput -> portno);

  printf("User input: \n\n%d %s %d %s %s \n", 
  userInput -> mode,
  userInput -> ip_addr,
  userInput -> portno,
  userInput -> recv_file, 
  userInput -> stats_filename);

}



int main(int argc, char *argv[]) // Three arguments provided:  client host port (host is your localhost if both processes are on your own machine)
{
  if (argc != 6) {
    error("Not enough input arguments.\n Usage: ./proj1_server <mode> <server_address> <port> <received_filenam> <stats_filename>");
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