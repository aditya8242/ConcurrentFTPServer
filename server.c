// Server

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/stat.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<stdbool.h>	// might not use it

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//    Function Name:    SendFileToClient
//    Description:      Used to send file to client
//    Input:            fd, filename  
//
/////////////////////////////////////////////////////////////////////////////////////////////////

void SendFileToClient(int ClientSocket, char *Filename)
{
	int fd = 0;
	int BytesRead = 0;

	struct stat sobj;

	char Buffer[1024];
	char Header[64] = {'\0'};

	fd = open(Filename, O_RDONLY);

	// unable to open file
	if(fd < 0)
	{
		// send error message to client
		write(ClientSocket, "ERR\n", 4);

		return;
	}

	// might fail, write if
	if(stat(Filename, &sobj) != 0)
	{
		return;
	}

	// example header
	// Header: OK 1700

	snprintf(Header, sizeof(Header), "OK %ld\n", (long)sobj.st_size);

	// write header to client
	write(ClientSocket, Header, strlen(Header));

	// Send actual file contents
	while((BytesRead = read(fd, Buffer, sizeof(Buffer))) != 0)
	{
		// send the data to client
		write(ClientSocket, Buffer, BytesRead);
	}

	close(fd);
}

////////////////////////////////////////////////////////////////////
//
//	Command line argument application
//	1st argument: Port number
//	./server 9000
//	argv[0]  argv[1]
//
////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	int ServerSocket = 0;
	int ClientSocket = 0;
	int Port = 0;
	int iRet = 0;

	char FileName[50] = {'\0'};

	struct sockaddr_in ServerAddr;
	struct sockaddr_in ClientAddr;

	socklen_t AddrLen = sizeof(ClientAddr);

	pid_t pid = 0;

	if(argc < 2 || argc > 2)
	{
		printf("Unable to proceed as invalid number of arguments.\n");
		printf("Please provide the port number\n");
		return -1;
	}

	// port number of server
	Port = atoi(argv[1]);

	//////////////////////////////////////////////////////////////////
	//	step 1: create tcp socket
	//////////////////////////////////////////////////////////////////

	ServerSocket = socket(AF_INET, SOCK_STREAM, 0);

	if(ServerSocket < 0)
	{
		printf("Unable to create server socket.\n");

		return -1;
	}

	//////////////////////////////////////////////////////////////////
	//	step 2: bind socket to ip and port
	//////////////////////////////////////////////////////////////////

	memset(&ServerAddr, 0, sizeof(ServerAddr));

	// initialize the structure
	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);
	ServerAddr.sin_addr.s_addr = INADDR_ANY;

	iRet = bind(ServerSocket, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));

	if(iRet == -1)
	{
		printf("Unable to bind.\n");
		close(ServerSocket);

		return -1;
	}

	//////////////////////////////////////////////////////////////////
	//	step 3: listen for client connections
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	step 3: listen for client connections
	//////////////////////////////////////////////////////////////////

	iRet = listen(ServerSocket, 11);

	if(iRet == -1)
	{
		printf("Server unable to listen the request.\n");

		close(ServerSocket);

		return -1;
	}

	printf("Server is running on port: %d\n", Port);

	//////////////////////////////////////////////////////////////////
	//	Loop which accepts client requests continuously
	//////////////////////////////////////////////////////////////////

	// loop to accept multiple client request
	while(1)
	{
		//////////////////////////////////////////////////////////////////
		//	step 4: accept the client request
		//////////////////////////////////////////////////////////////////

		memset(&ClientAddr, 0, sizeof(ClientAddr));

		printf("Server is waiting for client request\n");
		ClientSocket = accept(ServerSocket, (struct sockaddr *)&ClientAddr, &AddrLen);

		if(ClientSocket == -1)
		{
			printf("Unable to accept client request.\n");

			continue;	// for while, bad joke
		}

		printf("Client gets connected: %s.\n", inet_ntoa(ClientAddr.sin_addr));

		//////////////////////////////////////////////////////////////////
		//	step 5: create new process to handle client request
		//////////////////////////////////////////////////////////////////

		pid = fork();

		if(pid < 0)
		{
			printf("Unable to create a new process for request.\n");

			close(ClientSocket);

			continue;
		}

		// new process gets created for client
		if(pid == 0)
		{
			printf("New process is created for client request.\n");

			close(ServerSocket);

			memset(FileName, 0, sizeof(FileName));

			iRet = read(ClientSocket, FileName, sizeof(FileName) - 1);
			FileName[iRet] = '\0';

			FileName[strcspn(FileName, "\n")] = '\0';
			
			printf("Client requested file name: %s\n", FileName);

			SendFileToClient(ClientSocket, FileName);

			close(ClientSocket);

			printf("File transfer done.\nClient disconnected.\n");

			exit(0);	// kill the child process

		}// end of if (fork)
		else	// parent process (server)
		{
			close(ClientSocket);
		}// end of else

	}// end of while
	
	close(ServerSocket);

	return 0;
}// end of main