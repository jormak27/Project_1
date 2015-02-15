/* EE122 Server Program */
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>

# define QUEUE 10 

void error(char *msg){
	perror(msg);
	exit(1);
}


/* The server program is to transfer a specified file to any connecting clients.   
It only needs to connect to one client at a time, but it should be able to 
serve multiple clients in sequence without needing to be restarted. */
/*
	Usage: ./proj1_server <mode> <port> <filename> <packet_size> <packet_delay>
	
	<mode> Either 0, 1, or 2.  This value indicates what type of connection to establish with the server.
		   0 :  Connection-oriented sockets 
		   1 :  Connectionless sockets 
		   2 :  Connectionless sockets without checksum enabled

	<port> The port number to use for the server

	<filename> A path to a local file to host.  
			When a client connects to the server, it will be served this file 
			using the protocol specified by <mode>.

	<packet_size> The size of each packet to send, in bytes.  
			You will need to break the input file into packets of this size, 
			and send the packets separately through the network.

	<packet_delay> The server should sleep for this many seconds between sending packets.  
			Note that the value can be expressed as a floating point value.  
			For sleeping in fractions of a second, look at the man page for usleep().

	Example: The following call will specify the server to use datagram sockets (UDP) 
			 on port 33122, hosting a file called bar.txt in a folder named foo.  
			 The program will send the file in packets of size 1000 bytes, with zero delay between packets.
		./proj1_server 1 33122 foo/bar.txt 1000 0
*/

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
	
	if (0>n){
		error("ERROR: Error on binding");
	}
	n = listen(sockfd, QUEUE);
	clilen = sizeof(cli_addr); 
	
	FILE *fp; 
	fp = fopen(userInput -> filename, "r");  // This assumes that the file is in the same directory in which we're runing this program 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}
	fgets(buffer,userInput -> packet_size,fp);

	printf("this is a buffer %s\n\n",buffer);
	/// send buffer
	fgets(buffer,userInput -> packet_size,fp);

	printf("this is a buffer %s\n\n",buffer);

	printf("this is a buffer %s\n",buffer);
	


	newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);   // here, the process BLOCKS until a client connects to this server (on the port number specified by user input)
	if (newsockfd < 0)
		error("ERROR on accept");

	// first get the file via fopen or something
	// then open the file to divide into 1000 byte chunks
	// send each chunk between delay 


// 	n = write(newsockfd, "I got your message", 18);
// 	if (n<0) error("ERROR writing to the socket");
// 	return 0;
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