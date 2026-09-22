// Client

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

int ReadLine(int Sock, char *line, int iMax)
{
	int i = 0;
	int n = 0;

	char ch = '\0';

	while(i < iMax - 1)
	{
		n = read(Sock, &ch, 1);

		if(n <= 0)
		{
			break;
		}

		line[i++] = ch;

		if(ch == '\n')
		{
			break;
		}
	} // end of while
	
	line[i] = '\0';

	return i;
}

////////////////////////////////////////////////////////////////////
//
//	Command line argument application
//	1st argument: IP address
//	2nd argument: Port number
//	3rd argument: Targeted file name
//	4th argument: New file name
//
//	./client 127.0.0.1 9000    Demo.txt A.txt
//
//	argv[0]  argv[1]   argv[2] argv[3]  argv[4]
//
//  argc = 5
//
////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
	int Sock = 0;
	int Port = 0;					// argv[2]
	int iRet = 0;
	int outfd = 0;
	int n = 0;
	int toRead = 0;

	struct sockaddr_in ServerAddr;

	char *ip = NULL;				// argv[1]
	char *Filename = NULL;			// argv[3]
	char *OutFilename = NULL;		// argv[4]
	char Header[64] = {'\0'};
	char Buffer[1024] = {'\0'};

	long received = 0;
	long remaining = 0;

	if(argc < 5 || argc > 5)
	{
		printf("Unable to proceed as invalid number of arguments.\n");
		printf("Please provide:\n");
		printf("1. IP address\n");
		printf("2. Port number\n");
		printf("3. Targeted file name\n");
		printf("4. New file name\n");

		return -1;
	}

	// store command line arguments into the variables
	ip = argv[1];
	Port = atoi(argv[2]);
	Filename = argv[3];
	OutFilename = argv[4];

	//////////////////////////////////////////////////////////////////
	//	step 1: create tcp socket
	//////////////////////////////////////////////////////////////////

	Sock = socket(AF_INET, SOCK_STREAM, 0);

	if(Sock < 0)
	{
		printf("Unable to create client socket.\n");

		return -1;
	}

	//////////////////////////////////////////////////////////////////
	//	step 2: connect with server
	//////////////////////////////////////////////////////////////////

	memset(&ServerAddr, 0, sizeof(ServerAddr));

	ServerAddr.sin_family = AF_INET;
	ServerAddr.sin_port = htons(Port);

	// convert the ip address into binary
	inet_pton(AF_INET, ip, &ServerAddr.sin_addr); // might fail

	iRet = connect(Sock, (struct sockaddr *)&ServerAddr, sizeof(ServerAddr));

	if(iRet == -1)
	{
		printf("Unable to connect with server.\n");
		close(Sock);

		return -1;
	}

	//////////////////////////////////////////////////////////////////
	//	step 3: send file name
	//////////////////////////////////////////////////////////////////
	write(Sock, Filename, strlen(Filename));
	write(Sock, "\n", 1);

	//////////////////////////////////////////////////////////////////
	//	step 4: read the header
	//////////////////////////////////////////////////////////////////
	iRet = ReadLine(Sock, Header, sizeof(Header));

	if(iRet <= 0)
	{
		printf("Server gets disconnected abnormally.\n");
		close(Sock);
		return -1;
	}

	long FileSize = 0;

	sscanf(Header, "OK %ld", &FileSize);
	printf("File size is %d.\n", FileSize);

	//////////////////////////////////////////////////////////////////
	//	step 5: create new file
	//////////////////////////////////////////////////////////////////

	outfd = open(OutFilename, O_CREAT | O_WRONLY | O_TRUNC, 0777);

	if(outfd < 0)
	{
		printf("Unable to create downloaded file.\n");
		return -1;
	}

	while(received < FileSize)
	{
		remaining = FileSize - received;

		if(remaining > 1024)
		{
			toRead = 1024;
		}
		else
		{
			toRead = remaining;
		}

		n = read(Sock, Buffer, toRead);

		write(outfd, Buffer, n);

		received = received + n;
	} // end of while
	
	close(outfd);
	close(Sock);

	if(received == FileSize)
	{
		printf("Download successful!\n");
		return 0;
	}
	else
	{
		printf("Download failed.\n");
		return -1;
	}

	return 0;
}// end of main