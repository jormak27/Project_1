/* EE122 Server Program */
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

# define QUEUE 10 

void error(char *msg){
	perror(msg);
	exit(1);
}

// structure for holding the user inputs 
struct Inputs {
	int mode;
	int portno;
	char *filename;
	int packet_size;
	int packet_delay;
};

void Mode_0(struct Inputs *userInput){

	// assign a connection-oriented socket 
	int sockfd, newsockfd, clilen, n;
	struct sockaddr_in serv_addr, cli_addr; 
	char buffer[userInput -> packet_size];

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
	listen(sockfd, QUEUE);
	clilen = sizeof(cli_addr); 

	// open file
	FILE *fp; 
	fp = fopen(userInput -> filename, "r");  // This assumes that the file is in the same directory in which we're runing this program 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}

	while(1) {
		rewind(fp); // puts file pointer back to beginning
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);   // here, the process BLOCKS until a client connects to this server (on the port number specified by user input)
		if (newsockfd < 0)
			error("ERROR on accept");
		printf("Accepted Connection\n");
	
		bzero(buffer, userInput -> packet_size);

		while(fgets(buffer, (userInput -> packet_size)+1,fp) != NULL) {
			printf("this is a buffer %s\n\n",buffer);
			n = write(newsockfd, buffer, userInput -> packet_size);
			if (n < 0) error("ERROR writing to the socket.");
			usleep(userInput -> packet_delay);
		} 
		int terminator_size = 3;
		char *terminator="End";
		n = write(newsockfd, terminator, terminator_size);
		if (n < 0) error("ERROR writing terminator to the socket.");

    	printf("sent message\n"); //why does this not print out?
    }
    close(fp);
}

void Mode_1(struct Inputs *userInput){

	// assign a connection-oriented socket 
	int sockfd, clilen, n, recsize;
	struct sockaddr_in serv_addr, cli_addr; 
	char buffer[userInput -> packet_size];

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  //sockfd is socket file descriptor.  This is just returns an integer.  
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
	clilen = sizeof(cli_addr); 

	// open file
	FILE *fp; 
	fp = fopen(userInput -> filename, "r");  //This assumes that the file is in the same directory in which we're runing this program 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}

	while(1) {
		rewind(fp); // puts file pointer back to beginning		
		
		recsize = recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &clilen);
		if (recsize < 0) {
      		error("ERROR: recvfrom failed");
		}

		bzero(buffer, userInput -> packet_size);
		printf("About to send...\n");

		while(fgets(buffer, (userInput -> packet_size),fp) != NULL) {
			printf("this is a buffer %s\n\n",buffer);
			n = sendto(sockfd, buffer, userInput -> packet_size,0,(struct sockaddr *)&cli_addr, clilen);
			if (n < 0) error("ERROR sending datagram");
			usleep(userInput -> packet_delay);
		} 
		int terminator_size = 3;
		char *terminator="End";
		n = sendto(sockfd, terminator, terminator_size, 0,(struct sockaddr *)&cli_addr, clilen);
		if (n < 0) error("ERROR sending terminator to the socket.");
    	printf("sent message\n"); //why does this not print out?
    }
    close(fp);
}

unsigned short csum(unsigned short *buf, int nwords)
{   
    unsigned long sum;
    for(sum=0; nwords>0; nwords--)
            sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}



void Mode_2(struct Inputs *userInput){

	int sockfd, clilen, n, recsize;
	struct sockaddr_in serv_addr, cli_addr; 
	char buffer[userInput -> packet_size];
	
	memset(buffer, 0, userInput->packet_size);
	
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  //sockfd is socket file descriptor.  This is just returns an integer.  
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
	clilen = sizeof(cli_addr); 

	//struct iphdr *iph = (struct iphdr *)buffer; 
	struct udphdr *udph = (struct udphdr *) (buffer); //+ sizeof(struct iphdr));

	//UDP header
    udph -> uh_sport = serv_addr.sin_port;
    // destination port set in while loop 
    udph-> uh_ulen = htons(8 + strlen(buffer)); //tcp header sizeof
    udph-> uh_sum = 0; //leave checksum 0 now, filled later by pseudo header
	// still need to fill in destination port for udp header

	// open file
	FILE *fp; 
	fp = fopen(userInput -> filename, "r");  //This assumes that the file is in the same directory in which we're runing this program 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}
	int one = 1;
	const int *val = &one;

	while(1) {
		rewind(fp); // puts file pointer back to beginning		
		
		recsize = recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &clilen);
		if (recsize < 0) {
      		error("ERROR: recvfrom failed");
		}
		// set destination port as given by client 
		udph -> uh_dport = cli_addr.sin_port;

		printf("setsockopt() is OK.\n");

		bzero(buffer, userInput -> packet_size); // will this clear our header??
		printf("About to send...\n");

		while(fgets(buffer, (userInput -> packet_size),fp) != NULL) {
			printf("this is a buffer %s\n\n",buffer);
			n = sendto(sockfd, buffer, userInput -> packet_size,0,(struct sockaddr *)&cli_addr, clilen);
			if (n < 0) error("ERROR sending datagram");
			usleep(userInput -> packet_delay);
		} 
		int terminator_size = 3;
		char *terminator="End";
		n = sendto(sockfd, terminator, terminator_size, 0,(struct sockaddr *)&cli_addr, clilen);
		if (n < 0) error("ERROR sending terminator to the socket.");
    	printf("sent message\n"); //why does this not print out?
    }
    close(fp);

}

int main(int argc, char** argv) {
	// recall command line arguments by doing argv[0]....argv[4]
	if (argc != 6) {
		error("Not enough input arguments.\n Usage: ./proj1_server <mode> <port> <filename> <packet_size> <packet_delay>");
	}

	struct Inputs userInput;

	userInput.mode = atoi(argv[1]);
	userInput.portno = atoi(argv[2]);
	userInput.filename = argv[3];
	userInput.packet_size = atoi(argv[4]);
	userInput.packet_delay = atof(argv[5]);

	switch(userInput.mode){

	case 0: 
		Mode_0(&userInput);
	case 1: 
		Mode_1(&userInput);
	case 2: 
		Mode_2(&userInput);
	default:
		error("ERROR: Invalid Mode");
	}

	return 0;
}