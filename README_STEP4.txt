EE122 Project 1 Step 4 README
=============================

Group Members: Jordan Makansi (jasm001@bucknell.edu)
			   Gary Hoang (gqhoang@berkeley.edu)

Platform: Windows 32-bit

Compilation instructions
=========================
To compile the server and client run:

gcc -W -Wall -Wextra -ansi -pedantic proj1_server4.c -o proj1_server4.exe
gcc -W -Wall -Wextra -ansi -pedantic proj1_client4.c -o proj1_client4.exe

Depending on which version of gcc you have, you may receive compilation warnings of "implicit declaration of functions" so and so. 
The code should still run just fine.

How To Run
==========
to run the server: ./server_proj1.exe <port>
to run the client: ./client_proj1.exe <address> <port> [ -b ]
	-b flag is optional and enables consumption of client at 5 second intervals