#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	struct addrinfo hints, *serv, *p;
	struct sockaddr_in my_addr;
	socklen_t addr_size;
	int status, getsock_check;
	int socket_connect;
	int len_send, len_recv, bytes_sent, bytes_recv;
	char *name, *name_server;

	memset(&hints, 0, sizeof hints);//clear hints
	hints.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hints.ai_socktype = SOCK_STREAM;//TCP

	//get sockaddr of server and other detailed information
	//[imitate from Beej's Guide to Network Programmin]
	if((status = getaddrinfo("127.0.0.1", "21243", &hints, &serv))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		return 0;
	}

	//made a socket(IPv4 or IPv6, TCP), and connect it with the server socket
	//[copy from Beej's Guide to Network Programming]
	for(p = serv; p !=NULL; p = p->ai_next)
	{
		if((socket_connect = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("client:socket");
			continue;
		}
		if(connect(socket_connect, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(socket_connect);
			perror("client:connect");
			continue;
		}
		break;
	}
	//check port number
	// addr_size = sizeof my_addr;
	// getsock_check=getsockname(socket_connect,(struct sockaddr*)&my_addr, (socklen_t *)&addr_size);
	// if (getsock_check== -1) { perror("getsockname"); exit(1);}
	// printf("port:%d\n", ntohs(my_addr.sin_port));

	//check if there is a avaliable socket
	if(p == NULL)
	{
		fprintf(stderr, "client:failed to connect\n");
		exit(1);
	}
	//free the linked list
	freeaddrinfo(serv);//[copy from Beej's Guide to Network Programming]
	printf("The client is up and running\n");

	//send message to main Server[id and size]
	char msg[50] = "";
	sprintf(msg, "%s %s",argv[1], argv[2]);
	
	len_send = strlen(msg);//[copy from Beej's Guide to Network Programming]

	bytes_sent = send(socket_connect, msg, len_send, 0);//[copy from Beej's Guide to Network Programming]

	if(bytes_sent == -1)
	{
		perror("send");
	}
	else
	{
		printf("Send Link %s and file size %sMB to main server\n", argv[1], argv[2]);
	}

	//receive reply from the main Server
	char *words[10];
	double Tt = 0, Tp = 0;
	len_recv = 100;
	char buf[len_recv];
	bytes_recv = recv(socket_connect, buf, len_recv, 0);//[copy from Beej's Guide to Network Programming]
	if(bytes_recv==-1)
	{
		perror("recv");
	}
	else if(bytes_recv>0)
	{
		buf[bytes_recv] = '\0';
		
		//if not found the link id
		if(strcmp(buf, "no")==0)
			printf("No match found\n\n");
		else//if found
		{
			int j = 0;
			char *number;
			number = strtok(buf, " ");
			while(number != NULL)
			{
				words[j] = number;	
				j++;			
				number = strtok(NULL, " ");
			}
			Tt = atof(words[0]);
			Tp = atof(words[1]);
			
			printf("Receive transmission delay %.2fms, propagation delay %.2fms, and total delay %.2fms.\n\n", Tt, Tp, Tt+Tp);
		
		}
		
	}
	close(socket_connect);//[copy from Beej's Guide to Network Programming]

	return 0;
}