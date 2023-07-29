Project Description: Concurrent UDP-Based Server Daemon

This project involves the creation of a concurrent UDP-based server daemon, implemented using the UDP protocol. The server is capable of serving up to 4 clients simultaneously and can handle three API commands with specific functionalities:

* Creating a New Connection: The server allows clients to establish a new connection with it.

* Executing Linux Commands: Clients can send Linux commands to the server, which executes the commands using fork() and execvp() to process them on the server-side. The server then sends the output of the executed command back to the client using sendto().

* Disconnecting from the Server: Clients can request to disconnect from the server.

The server uses UDP for communication, and upon successful connection with a client, it spawns a new pthread to handle that client's requests concurrently.

- Client Interaction:
Clients can interact with the server using a shell prompt, using the format [<ip address>] as a [Hostname]. The following commands are implemented on the client-side:
* connect: This command is used to connect to the remote server by providing its IP address.
* disconnect: Clients can use this command to disconnect from the remote server.
* quit: This command allows clients to exit from the client shell.

- Compiling Instructions:
To compile any C file, the following commands are used:
$ gcc -o <output_filename> <Filename>.c
$ ./output_filename

Sample Test Run

The steps to run the project are as follows:
* Start the server on the spirit machine using the command: ./server 8707.
* Connect to the server from different workstations using SSH commands (e.g., ssh).
* Compile the client code on each workstation using: gcc -o client client.c.
* Run the client using ./client and type the command connect <ip_address> to connect to the server.
* After a successful connection, type the Linux command (e.g., ls) on the client side to execute it on the server.
* The client will receive the output of the executed command from the server.
Existing Bugs
There are a couple of known bugs in the project:
* Output Retrieval Issue: After entering the first command, clients need to retype the command to get the result from the server on the client terminal.
* Maximum Client Limit: The project is currently limited to accepting connections from up to 4 clients simultaneously. If a fifth client tries to connect, an error message "Error: Maximum number of clients reached" is displayed to the client.
 





