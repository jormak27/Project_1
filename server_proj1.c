/* EE122 Server Program */
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 

# define QUEUE 10 

void error(char *msg){
	perror(msg);
	exit(1);
}

// structure for holding the user inputs 
struct Inputs
{
	int mode;
	int portno;
	char *filename;
	int packet_size;
	int packet_delay;
};

void Mode_0(struct Inputs *userInput){
	//print for error checking 
	// printf("%d %d %s %d %f \n", 
	// userInput -> mode,
	// userInput -> portno,
	// userInput -> filename,
	// userInput -> packet_size, 
	// userInput -> packet_delay);

	// assign a connection-oriented socket 
	int sockfd, newsockfd, clilen, n;
	struct sockaddr_in serv_addr, cli_addr; 
	char *buffer;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);  //sockfd is socket file descriptor.  This is just returns an integer.  
	if (0>sockfd){
		error("ERROR Opening socket");
	}
	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;   //assigns the socktet to IP address (INADDR_ANY). // WHAT TO PUT HERE FOR IP ADDRESS??? 
	serv_addr.sin_port = htons(userInput -> portno);  // returns port number converted (host byte order to network short byte order)
	n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	
	FILE *fp; 
	fp = fopen(userInput -> filename, "r");  // This assumes that the file is in the same directory in which we're runing this program 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}

	if (0>n){
		error("ERROR: Error on binding");
	}
	listen(sockfd, QUEUE);
	clilen = sizeof(cli_addr); 

	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);   // here, the process BLOCKS until a client connects to this server (on the port number specified by user input)
	if (newsockfd < 0)
		error("ERROR on accept");
	printf("Accepted Connection\n");
	// first get the file via fopen or something
	// then open the file to divide into 1000 byte chunks
	// send each chunk between delay 
	
	bzero(buffer, userInput -> packet_size);
/*	if (fgets(buffer, (userInput -> packet_size)+1,fp) != NULL) { // I don't know why we need the + 1 !!!
		printf("this is a buffer %s\n\n",buffer);
		n = write(newsockfd, buffer, userInput -> packet_size);
		if (n < 0) error("ERROR writing to the socket.");
		usleep(userInput -> packet_delay);
	}
	if (fgets(buffer, (userInput -> packet_size)+1,fp) != NULL) { // I don't know why we need the + 1 !!!
		printf("this is a buffer %s\n\n",buffer);
		n = write(newsockfd, buffer, userInput -> packet_size);
		if (n < 0) error("ERROR writing to the socket.");
		usleep(userInput -> packet_delay);
	} */

	while(fgets(buffer, (userInput -> packet_size)+1,fp) != NULL) {
		printf("this is a buffer %s\n\n",buffer);
		n = write(newsockfd, buffer, userInput -> packet_size);
		if (n < 0) error("ERROR writing to the socket.");
		usleep(userInput -> packet_delay);
	} 
	n = write(newsockfd, "End", 3);
	if (n < 0) error("ERROR writing to the socket.");

    printf("\nsent message.");
}

int main(int argc, char** argv) {
	// recall command line arguments by doing argv[0]....argv[4]
	if (argc != 6) {
		error("Not enough input arguments.\n Usage: ./proj1_server <mode> <port> <filename> <packet_size> <packet_delay>");
	}

	int mode, portno, packet_size;
	char *filename;
	float packet_delay;
	
	struct Inputs userInput;

	userInput.mode = atoi(argv[1]);
	userInput.portno = atoi(argv[2]);
	userInput.filename = argv[3];
	userInput.packet_size = atoi(argv[4]);
	userInput.packet_delay = atof(argv[5]);

	if (0==userInput.mode){
		Mode_0(&userInput);
	}

}


// void return_args(char **arguments){
// 	int i;
// 	arguments[1]


// }