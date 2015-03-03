/* EE122 Server Program */
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 
# include <time.h>

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

/*
    Generic checksum calculation function
*/
unsigned short csum(unsigned short *ptr,int nbytes) 
{
    register long sum;
    unsigned short oddbyte;
    register short answer;
 
    sum=0;
    while(nbytes>1) {
        sum+=*ptr++;
        nbytes-=2;
    }
    if(nbytes==1) {
        oddbyte=0;
        *((u_char*)&oddbyte)=*(u_char*)ptr;
        sum+=oddbyte;
    }
 
    sum = (sum>>16)+(sum & 0xffff);
    sum = sum + (sum>>16);
    answer=(short)~sum;
     
    return(answer);
}


// The IP header's structure
struct ipheader {
	unsigned char      iph_ihl:5, iph_ver:4;
	unsigned char      iph_tos;
	unsigned short int iph_len;
	unsigned short int iph_ident;
	unsigned char      iph_flag;
	unsigned short int iph_offset;
	unsigned char      iph_ttl;
	unsigned char      iph_protocol;
	unsigned short int iph_chksum;
	unsigned int       iph_sourceip;
	unsigned int       iph_destip;
};

// UDP header's structure
struct udpheader {
	unsigned short int udph_srcport;
 	unsigned short int udph_destport;
 	unsigned short int udph_len
; 	unsigned short int udph_chksum;
};

void Mode_2(struct Inputs *userInput){

	int sockfd, clilen, n, recsize;
	struct sockaddr_in serv_addr, cli_addr; 
	char buffer[userInput -> packet_size], *data; //let's see what happens
	// in other example buffer = datagram

	// making socket!
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);    
	if (0>sockfd){
		error("ERROR Opening socket");
	}

	// so packet = ethernet header + ip header + udp header + data
	// ethernet header provided by OS kernal and don't construct it

	// that means we have to have data = buffer + ipheader + tcpheader

	// zero out packet buffer
	memset(buffer, 0, userInput->packet_size);

	// filling in server stuffs
	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;   //assigns the socktet to IP address (INADDR_ANY). // WHAT TO PUT HERE FOR IP ADDRESS??? 
	serv_addr.sin_port = htons(userInput -> portno);  // returns port number converted (host byte order to network short byte order)
	n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (0>n){
		error("ERROR: Error on binding");
	}

	clilen = sizeof(cli_addr); 

while(1) {
	// open file
	FILE *fp; 
	fp = fopen(userInput -> filename, "r"); 
	printf("file was opened \n");
	if (NULL==fp){
		error("ERROR: File did not open ");
	}
	bzero(buffer, userInput->packet_size);
	recsize = recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &clilen);
	if (recsize < 0) {
    	error("ERROR: recvfrom failed");
	}
	printf("recvfrom successful from client\n");
	printf("%s\n", buffer);
	
	bzero(buffer, userInput->packet_size);
	// make headers
	struct ipheader *ip = (struct ipheader *) buffer;
	struct udpheader *udp = (struct udpheader *) (buffer + sizeof(struct ipheader));


	//Data part
    data = buffer + sizeof(struct ipheader) + sizeof(struct udpheader);
    strcpy(data, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");

	// filling in IP fields 
	ip->iph_ihl = 5;
	ip->iph_ver = 4;
	ip->iph_tos = 0;
	ip->iph_len = sizeof(struct ipheader) + sizeof(struct udpheader) + strlen(data);
	ip->iph_ident = htons(54321); // no idea
    ip->iph_offset = 0;
	ip->iph_ttl = 64; // hops
	ip->iph_protocol = 17; // UDP
    ip->iph_chksum = 0;      //Set to 0 before calculating checksum
	ip->iph_sourceip = serv_addr.sin_addr.s_addr; 
	ip->iph_destip = cli_addr.sin_addr.s_addr;

	// filling in UDP fields 
	udp->udph_srcport = serv_addr.sin_port;
	udp->udph_destport = cli_addr.sin_port;
	udp->udph_len = htons( sizeof(struct udpheader));
	udp->udph_chksum = 0;

	ip->iph_chksum = csum((unsigned short *)data, ip->iph_len);

	// declaring that we are going to use our own headers!
	int one = 1;
	const int *val = &one;
	if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0) {
		error("setsockopt() error");
	} 
	printf("setsockopt() is OK.\n");
		
	//bzero(buffer, userInput -> packet_size); // will this clear our header?? - yes I think so!!!
	printf("About to send...\n");
	//printf("%d\n", ip->iph_chksum); // checksum is coming out to be zero!!! - possibly why it is wrong??
	//printf("%llu\n", ip->iph_ver);	// something is happening around here!!! iph_ver comes out as zero!! after i take out bzero!
	//printf("%d\n", udp->udph_len);
	// yeah fgets is rewriting our fields and what not!!!1
	//if (fgets(buffer, (userInput -> packet_size),fp) != NULL) {
		//printf("%d\n", ip->iph_chksum); // checksum is coming out to be zero!!! - possibly why it is wrong??
		//printf("%llu\n", ip->iph_ver);

	// using the 'ABCD....'
	// also wondering why we send buffer, but not data, but length is everything together!
		printf("this is buffer: %s\n\n",data);
		n = sendto(sockfd, data, ip->iph_len,0,(struct sockaddr *)&cli_addr, clilen);
		if (n < 0) error("ERROR sending datagram");
		usleep(userInput -> packet_delay);
	//} 

    printf("sent message\n");
/*    	
	while(1) {
		rewind(fp); // puts file pointer back to beginning		
		
		recsize = recvfrom(sockfd, (void *)buffer, sizeof(buffer), 0, (struct sockaddr *)&cli_addr, &clilen);
		if (recsize < 0) {
      		error("ERROR: recvfrom failed");
		}
		printf("recvfrom successful from client\n");

		// filling in rest of headers and ip checksum
		ip->iph_destip = cli_addr.sin_addr.s_addr;
	    udp->udph_destport = cli_addr.sin_port;
		ip->iph_chksum = csum((unsigned short *)buffer, sizeof(struct ipheader) + sizeof(struct udpheader));
		
		if(setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0)
		{
			error("setsockopt() error");
		}
		else
		printf("setsockopt() is OK.\n");

		//bzero(buffer, userInput -> packet_size); // will this clear our header?? - yes I think so!!!
		printf("About to send...\n");
		printf("%d\n", ip->iph_chksum); // checksum is coming out to be zero!!! - possibly why it is wrong??
		printf("%llu\n", ip->iph_ver);	// something is happening around here!!! iph_ver comes out as zero!! after i take out bzero!
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
    	printf("sent message\n");
    }

*/
    close(fp);
    }

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