/* EE122 Server Program */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>


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

int main(int argc, char** argv) {
	// recall command line arguments by doing argv[0]....argv[4]


}
