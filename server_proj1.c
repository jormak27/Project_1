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

# define TRUE 1
# define FALSE 0

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

		while(fgets(buffer, (userInput -> packet_size),fp) != NULL) {
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
    	printf("client port number: %d\n", ntohs(cli_addr.sin_port) );
		printf("client buffer: %s\n", buffer);

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

void Mode_2(struct Inputs *userInput){

	// will make header and buffer and then append to data

	int sockfd, clilen, n, recsize;
	struct sockaddr_in serv_addr, cli_addr; 
	// malloc or hard code
	//char buffer[userInput -> packet_size], header[8];
	char buffer[500], header[8];
	char data[528], port[2];
	int so_broadcast;

	memset(buffer, 0, 500);
	memset(header, 0, 8);
	memset(data, 0, 528);
	memset(port, 0, 2);
	
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  
	if (0>sockfd){
		error("ERROR Opening socket");
	}

	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;  
	serv_addr.sin_port = htons(userInput -> portno);
	n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (0>n){
		error("ERROR: Error on binding");
	}
	clilen = sizeof(cli_addr); 

	//struct iphdr *iph = (struct iphdr *)buffer; 
	struct udphdr *udph = (struct udphdr *) (header); //+ sizeof(struct iphdr));

	//UDP header
    udph -> uh_sport = serv_addr.sin_port;
    udph-> uh_ulen = htons(8 + sizeof(buffer));
    udph-> uh_sum = 0; 
	// still need to fill in destination port for udp header

	// open file
	FILE *fp; 
	fp = fopen(userInput -> filename, "r");  //This assumes that the file is in the same directory in which we're runing this program 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}
	while(1) {
		rewind(fp); // puts file pointer back to beginning		

		// do we need ip and udp header
// Use recvmsg() with the msg[] buffers initialized so that the first 
// one receives the IP header, then the second one will only contain 
// data.
// raw_sendmsg() ?

		/* http://www.tagwith.com/question_440319_recvfrom-with-raw-sockets-get-just-the-data
But of course, when calling recvfrom() on a raw socket, 
you are given the raw IP datagram, while the sockaddr struct 
is not filled in.
		*/

		// for receiving the IP header is always included in the packet!!
		// which is the 20 extra bytes !!!

		// if doesn't work, try recvmsg with msg[] already made!
		// http://man7.org/linux/man-pages/man2/recvmsg.2.html
		// http://pubs.opengroup.org/onlinepubs/009695399/functions/recvmsg.html
		// microsoft FTW!!
		// http://developer.nokia.com/community/wiki/Open_C_Sockets:_recv,_recvfrom,_recvmsg_methods

		recsize = recvfrom(sockfd, (void *)data, 528, 0, (struct sockaddr *)&cli_addr, &clilen);
		if (recsize < 0) {
      		error("ERROR: recvfrom failed");
		}

		// memcpy out ip header possibly and fill in cli_addr to send to!!
		// so it doesn't get send to everyone:
		// so possibly because cli_addr not filled is in broadcast mode!!
		// wait but my sendto even goes to itself?? and echoes everywhere

		//printf("received size: %d\n", recsize);
		printf("client buffer: %s\n", data+28); // shows up as connecting!!!!!
		// information is in form ip header(20) + udp header(8) + buffer
		// so information is data + 28!!!
		if(strcmp("connecting", data+28) == 0) {
		// set destination port as given by client 
		memcpy(port, data+20, 2);
		uint16_t client_portno = ntohs(*port); // got the port number of client
		printf("client source port number: %d\n", client_portno);
		//memcpy(port, data+22, 2);
		//uint16_t dest_portno = ntohs(*port);
		//printf("client destination port number: %d\n", dest_portno);		
		udph -> uh_dport = client_portno;
		//printf("server source port %d\n", udph->uh_sport);
		//printf("server dest port %d\n", udph->uh_dport); 	

		bzero(data, 528);
		memcpy(data, header, 8);
		bzero(buffer, 500); 
		printf("About to send...\n");

		//memcpy(port, data, 2);
		//dest_portno = ntohs(*port);
		//printf("server source port number: %d\n", dest_portno);	
		//memcpy(port, data+2, 2);
		//client_portno = ntohs(*port);
		//printf("server destination port number: %d\n", client_portno);	

		//put in ip header to send?

		// try regular textfile and a binary file

		while(fgets(buffer, 500, fp) != NULL) {
		// weird because picks up its own packets too!
		//while(fgets(buffer, (userInput -> packet_size),fp) != NULL) {
			printf("SENDING %s\n\n",buffer);
			memcpy(data+8, buffer, 500);
			//n = sendto(sockfd, buffer, userInput -> packet_size,0,(struct sockaddr *)&cli_addr, clilen);
			// fill in cli_addr
			n = sendto(sockfd, data, 508,0,(struct sockaddr *)&cli_addr, clilen);
			if (n < 0) error("ERROR sending datagram");
			//printf("n: %d\n", n);
			usleep(userInput -> packet_delay);
		} 
		//End terminator does not show up!!! Maybe still firing others and never shows this
		memset(data+8, 0, 1000);
		memcpy(data+8, "End", sizeof("End"));
		//int terminator_size = 3;
		//char terminator[3]="End";
		//memset(buffer, 0, 500);
		//memcpy(buffer, header, 8);
		//memcpy(buffer+8, "End", 3);
		printf("SENDING %s\n\n", data+8);
		n = sendto(sockfd, data, 508, 0,(struct sockaddr *)&cli_addr, clilen);
		if (n < 0) error("ERROR sending terminator to the socket.");
	 }
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