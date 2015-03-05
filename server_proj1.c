/* 
EE122 Project 1 
================
Phase 1: Server Program 

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
# include <unistd.h>
# include <fcntl.h>

# define QUEUE 10 

/*
	Mode 0: Connection-oriented sockets
	Mode 1: Connectionless sockets
	Mode 2: Connectionless sockets with (UDP) checksum disabled
*/

void error(char *msg){
	perror(msg);
	exit(1);
}

/* Structure For Holding The User Inputs */ 
typedef struct Inputs {
	int mode;
	int portno;
	char *filename;
	int packet_size;
	int packet_delay;
}Inputs;

void Mode_0(struct Inputs *userInput){

	int sockfd, newsockfd,  serv_len, cli_len, check, terminator_size;
	struct sockaddr_in serv_addr, cli_addr; 
	char *buffer, terminator[3];
	FILE *fp;

	/* initialize known variables */
	cli_len = sizeof(cli_addr); 
	serv_len = sizeof(serv_addr);
	memcpy(terminator, "End", 3);
	terminator_size = strlen(terminator);

	buffer = (char *) malloc(userInput->packet_size);
	if (buffer == NULL) error("Failed to allocate memory for given packet size");

	/* make socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (sockfd < 0) error("ERROR Opening socket"); 

	/* fill in server struct */
	bzero ((char*) &serv_addr, serv_len);
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;   /* assigns the socktet to IP address (INADDR_ANY). */
	serv_addr.sin_port = htons(userInput -> portno);  /* returns port number converted (host byte order to network short byte order) */
	
	/* bind and listen */
	check = bind(sockfd, (struct sockaddr *) &serv_addr, serv_len);
	if (check < 0) error("ERROR: Error on binding"); 
	listen(sockfd, QUEUE);

	/* open file for reading */
	fp = fopen(userInput -> filename, "r");  
	if (NULL==fp) error("ERROR: File did not open ");

	printf("Server successfully opened and awaiting clients.\n");

	while(1) {
		rewind(fp); /* puts file pointer back to beginning */
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &cli_len); 
		if (newsockfd < 0) error("ERROR on accept"); 
	
		bzero(buffer, userInput -> packet_size);

		/* send our file */
		while(fgets(buffer, (userInput -> packet_size),fp) != NULL) {
			check = write(newsockfd, buffer, userInput -> packet_size);
			if (check < 0) {error("ERROR writing to the socket."); }
			usleep(userInput -> packet_delay);
		}

		/* send our terminator */
		check = write(newsockfd, terminator, terminator_size);
		if (check < 0) error("ERROR writing terminator to the socket.");

    	printf("Received a client and sent file successfully.\n");
    }
    check = fclose(fp);
    if (check == EOF) error("Error: Failed to close file.");
}

void Mode_1(struct Inputs *userInput){

	int sockfd, serv_len, cli_len, check, terminator_size;
	struct sockaddr_in serv_addr, cli_addr; 
	char *buffer, terminator[3];
	FILE *fp; 

	/* initialize known variables */
	cli_len = sizeof(cli_addr); 
	serv_len = sizeof(serv_addr);
	memcpy(terminator, "End", 3);
	terminator_size = strlen(terminator);

	buffer = (char *) malloc(userInput->packet_size);
	if (buffer == NULL) error("Failed to allocate memory for given packet size");

	sockfd = socket(AF_INET, SOCK_DGRAM, 0);   
	if (0>sockfd) error("ERROR Opening socket"); 

	bzero ((char*) &serv_addr, serv_len);
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY; 
	serv_addr.sin_port = htons(userInput -> portno);  
	check = bind(sockfd, (struct sockaddr *) &serv_addr, serv_len);
	if (check < 0) error("ERROR: Error on binding"); 

	/* open file */
	fp = fopen(userInput -> filename, "r"); 
	if (NULL==fp) error("ERROR: File did not open");

	printf("Server successfully opened and awaiting clients.\n");

	while(1) {
		rewind(fp); /* puts file pointer back to beginning */		
		
		check = recvfrom(sockfd, buffer, userInput->packet_size, 0, (struct sockaddr *)&cli_addr, &cli_len);
		if (check < 0) error("ERROR: recvfrom failed");
		/* only if it is our client, then send data */
    	if (strcmp("connecting", buffer) == 0) {

			bzero(buffer, userInput -> packet_size);

			while(fgets(buffer, (userInput -> packet_size),fp) != NULL) {
				check = sendto(sockfd, buffer, userInput -> packet_size,0,(struct sockaddr *)&cli_addr, cli_len);
				if (check < 0) error("ERROR sending datagram");
				usleep(userInput -> packet_delay);
			} 
			check = sendto(sockfd, terminator, terminator_size, 0,(struct sockaddr *)&cli_addr, cli_len);
			if (check < 0) error("ERROR sending terminator to the socket.");
    		printf("Received a client and sent file successfully.\n");
    	}
    }
    fclose(fp);
}

void Mode_2(struct Inputs *userInput){

	int sockfd, clilen, check;
	struct sockaddr_in serv_addr, cli_addr; 
	char *buffer, *data, header[8], port[2];
  	struct udphdr *udph;
  	FILE *fp;
  	uint16_t client_portno;

  	/* initialize variables */
	clilen = sizeof(cli_addr); 
	buffer = (char *) malloc(userInput-> packet_size);
	if (buffer == NULL) error("ERROR: Failed to allocate memory for buffer");
	data = (char *) malloc((userInput->packet_size)+20+8);
	if (data == NULL) error("ERROR: Failed to allocate memory for data");
	/* clean */
	memset(buffer, 0, userInput->packet_size);
	memset(header, 0, 8);
	memset(data, 0, (userInput->packet_size)+20+8);
	memset(port, 0, 2);
	
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);  
	if (sockfd < 0) error("ERROR Opening socket");

	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;  
	serv_addr.sin_port = htons(userInput -> portno);
	check = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (check < 0) error("ERROR: Error on binding");

	/* UDP header */
	udph = (struct udphdr *) (header); 
    udph -> uh_sport = serv_addr.sin_port;
    udph-> uh_ulen = htons(8 + sizeof(buffer));
    udph-> uh_sum = 0; 

	/* open file */
	fp = fopen(userInput -> filename, "r");
	if (NULL==fp) error("ERROR: File did not open ");

	printf("Server successfully opened and awaiting clients\n");

	while(1) {
	/*	 Use recvmsg() with the msg[] buffers initialized so that the first 
		 one receives the IP header, then the second one will only contain 
		 data.	
		 if doesn't work, try recvmsg with msg[] already made!
		 http://man7.org/linux/man-pages/man2/recvmsg.2.html
		 http://pubs.opengroup.org/onlinepubs/009695399/functions/recvmsg.html
		 microsoft FTW!!
		 http://developer.nokia.com/community/wiki/Open_C_Sockets:_recv,_recvfrom,_recvmsg_methods
		*/
		check = recvfrom(sockfd, (void *)data, 528, 0, (struct sockaddr *)&cli_addr, &clilen);
		if (check < 0) error("ERROR: recvfrom failed");
/*
		memcpy out ip header possibly and fill in cli_addr to send to!!
		so it doesn't get send to everyone:
		so possibly because cli_addr not filled is in broadcast mode!!
		wait but my sendto even goes to itself?? and echoes everywhere

		//printf("client buffer: %s\n", data+28); // shows up as connecting!!!!!
		// information is in form ip header(20) + udp header(8) + buffer
		// so message is data + 28!!!
*/
		if(strcmp("connecting", data+28) == 0) {
			rewind(fp); 	
			/* set destination port as given by client */
			memcpy(port, data+20, 2);
			client_portno = ntohs(*port); /* get the client port number	*/
			udph -> uh_dport = client_portno;

			bzero(data, (userInput->packet_size)+20+8);
			memcpy(data, header, 8);
			bzero(buffer, userInput->packet_size); 

			while(fgets(buffer, userInput->packet_size, fp) != NULL) {
			/* weird because picks up its own packets too! */
			/* while(fgets(buffer, (userInput -> packet_size),fp) != NULL) { */
				memcpy(data+8, buffer, userInput->packet_size);
				/* fill in cli_addr??? */
				check = sendto(sockfd, data, (userInput->packet_size)+8,0,(struct sockaddr *)&cli_addr, clilen);
				if (check < 0) error("ERROR sending datagram");
				usleep(userInput -> packet_delay);
			} 
			memset(data+8, 0, userInput->packet_size);
			memcpy(data+8, "End", sizeof("End"));
			check = sendto(sockfd, data, (userInput->packet_size)+8, 0,(struct sockaddr *)&cli_addr, clilen);
			if (check < 0) error("ERROR sending terminator to the socket.");
			memset(data, 0, (userInput->packet_size)+8);
			printf("Received a client and sent file successfully.\n");
	 	}
    }

    fclose(fp);

}

int main(int argc, char** argv) {
	Inputs userInput;
	
	if (argc != 6) {
		error("Not enough input arguments.\n Usage: ./proj1_server <mode> <port> <filename> <packet_size> <packet_delay>");
	}


	userInput.mode = atoi(argv[1]);
	userInput.portno = atoi(argv[2]);
	userInput.filename = argv[3];
	userInput.packet_size = atoi(argv[4]);
	userInput.packet_delay = atof(argv[5]);

	switch(userInput.mode){

	case 0: 
		Mode_0(&userInput);
		break;
	case 1: 
		Mode_1(&userInput);
		break;
	case 2: 
		Mode_2(&userInput); 
	default:
		error("ERROR: Invalid Mode");
		break;
	}

	return 0;
}