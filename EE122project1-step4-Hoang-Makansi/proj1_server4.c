/* 
EE122 Project 1
Phase 4 Server Program
*/
# include <stdio.h>
# include <sys/types.h>
# include <sys/socket.h>
# include <netinet/in.h>
# include <stdlib.h> 
# include <string.h>
# include <strings.h>
# include <netdb.h> 
# include <unistd.h>

# define QUEUE 10 

/* only needs to service one client */
void error(char *msg){
	perror(msg);
	exit(1);
}

/* structure for holding the user inputs */
typedef struct Inputs {
	int portno;
	int MAX_NUM;
} Inputs;

void serv(struct Inputs *userInput){

	int sockfd, serv_len, cli_len, check, max, size;
	struct sockaddr_in serv_addr, cli_addr;
	char *buffer;

	size = 200;
	buffer = (char *)malloc(size);

	/* initializing stuffs */
	max = userInput -> MAX_NUM;
	cli_len = sizeof(cli_addr); 
	serv_len = sizeof(serv_addr);

	/* socket making and binding */
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);  
	if (sockfd < 0) error("ERROR Opening socket"); 

	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;  
	serv_addr.sin_port = htons(userInput -> portno);  
	check = bind(sockfd, (struct sockaddr *) &serv_addr, serv_len);
	if (check < 0) error("ERROR: Error on binding");  

	check = recvfrom(sockfd, buffer, size, 0,  (struct sockaddr *)&cli_addr, &cli_len);
	printf("Accepted a Client\n");
	if (strcmp("connecting", buffer) == 0) {
		while(1) { 
			memcpy(buffer, "EVERYTHING IS AWESOME. EVERYTHING IS COOL.", sizeof("EVERYTHING IS AWESOME. EVERYTHING IS COOL."));
			check = rand() % max + 1;
			sleep(check);
			check = sendto(sockfd, buffer, size, 0, (struct sockaddr *)&cli_addr, cli_len);
			if (check < 0) error("ERROR writing to the socket.");
			printf("Sent: %s\n", buffer);
		} 
	}

}

int main(int argc, char** argv) {
	
	Inputs userInput;

	if (argc != 2) error("Not enough input arguments.\n Usage: ./proj1_server4 <port>");
	userInput.portno = atoi(argv[1]);
	userInput.MAX_NUM = 10; /* want average of 5 seconds */
	serv(&userInput);
	return 0;
}