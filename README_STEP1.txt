EE122 Project 1 Step 1 README
=============================

Group Members: Jordan Makansi (jasm001@bucknell.edu)
			   Gary Hoang (gqhoang@berkeley.edu)

Platform: Windows 32-bit

Compilation instructions
=========================
To compile the server and client run:

gcc -W -Wall -Wextra -ansi -pedantic server_proj1.c -o server_proj1.exe
gcc -W -Wall -Wextra -ansi -pedantic client_proj1.c -o client_proj1.exe

Depending on which version of gcc you have, you may receive compilation warnings of "implicit declaration of functions bzero and bcopy". 
The code should run just fine.

How To Run
==========
to run the server: ./server_proj1.exe <mode> <port> <filename> <block_size (in bytes)> <block_delay (in microseconds)> 
to run the client: ./client_proj1.exe <mode> <server> <port> <received_filename> <stats_filename> <client_buffer (in bytes)> <timeout limit (in secs only)> <timeout limit (in millisecs only)>

Notes on how to Run
=====
1) To change packet sizes in client, simply change the value of client_buffer in the command line of client
2) The total timeout is the timeout limit (in secs) + the timeout limit (in millisecs).

SPECIAL NOTES
=============
To run across two windows machines, one needs to first disable the windows firewall (include instructions). And on top of that make a new inbound and outbound rule (the type of rule should be program, and the rest just enable the option that will allow everyone in).