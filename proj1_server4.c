/* EE122 Server Program */
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

void error(char *msg){
	perror(msg);
	exit(1);
}

// structure for holding the user inputs 
struct Inputs {
	int portno;
	int MAX_NUM;
};

void serv(struct Inputs *userInput){

	int sockfd, newsockfd, clilen, n, max;
	struct sockaddr_in serv_addr, cli_addr;
	char buffer[200] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	// initializing stuffs
	max = userInput -> MAX_NUM;

	// socket making and binding
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  
	if (0>sockfd){ error("ERROR Opening socket"); }
	bzero ((char*) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;   
	serv_addr.sin_addr.s_addr = INADDR_ANY;  
	serv_addr.sin_port = htons(userInput -> portno);  
	n = bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (0>n){ error("ERROR: Error on binding"); }
	listen(sockfd, QUEUE);
	clilen = sizeof(cli_addr); 

	while(1) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);   
		if (newsockfd < 0) { error("ERROR on accept"); }
		printf("Accepted Connection\n");
		while(1) { 
			// how to know when client is done
			// so idea is that client sends something when ctrl+c to exit and server gets it
			printf("%s\n", buffer);
			// so to get random number from 0 to 10
			n = rand() % max + 1;
			sleep(n);
			n = write(newsockfd, buffer, 200);
			if (n < 0) error("ERROR writing to the socket.");

			// how to know when client disconnects???
			// so basically need to poll when over?
			/*
			http://linux.die.net/man/7/tcp

			http://tldp.org/HOWTO/TCP-Keepalive-HOWTO/overview.html
			http://lkml.iu.edu/hypermail/linux/kernel/0106.1/1154.html
			http://ltxfaq.custhelp.com/app/answers/detail/a_id/1512/~/tcp-keepalives-explained
			*/
		} 
    }
}


int main(int argc, char** argv) {
	if (argc != 2) {
		error("Not enough input arguments.\n Usage: ./proj1_server4 <port>");
	}

	struct Inputs userInput;
	userInput.portno = atoi(argv[1]);
	userInput.MAX_NUM = 10; // want average of 5 seconds
	serv(&userInput);

	return 0;
}