/* EE122 Client Program */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* The client program is to connect to the specified server on the specified port, 
receive a transferred file using the specified mode, and to save the given file to disk 
at the specified location. When running the client, it should attempt to connect to 
the server, retrieve the file, and exit. The client should also print out statistics 
information of how long the connection lasted (in seconds) and how large the received 
file is. In addition to the received file itself, the 
client is required to export a second file listing the time-delay (in seconds) between each 
packet received from the server. */

/*
	Usage: ./proj1_client <mode> <server_address> <port> <received_filename> <stats_filename>

	<mode> Either 0, 1, or 2.  This value indicates what type of connection to establish with the server.  
		It is assumed that the same mode is specified for both the server and the client.

	<address> The IP address of the server to connect to.
	
	<port> The port number of the server to connect to.  
		Use the same number here as was used to initialize the server.

	<filename> Where to store the file on the local machine after it has been transferred from the server.

	<stats_filename> Where to write the file containing statistics information 
		about the received packets from the server.  This should be a text file, 
		where each line indicates the delay between successive packets, as a floating-point number in seconds.

	Example: This call open a client to connect to the server located at 128.32.47.83 
		using datagram sockets (UDP) on port 33122, and will retrieve the file foo.txt 
		and write out statistics info to stats.txt :
	./proj1_client 1 128.32.47.83 33122 bar.txt stats.txt

*/

int main(int argc, char** argv) {

}
