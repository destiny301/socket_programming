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
	struct addrinfo hints, *serv, *servcalc, *p, *o;
	struct sockaddr_storage their_addr;
	struct sockaddr_in my_addr;
	socklen_t addr_size;
	int status, getsock_check;
	int yes = 1;
	int socket_connect, socket_calc;

	memset(&hints, 0, sizeof hints);//clear hints
	hints.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hints.ai_socktype = SOCK_DGRAM;//UDP

	//get sockaddr and other detailed information
	//[imitate from Beej's Guide to Network Programmin]
	if((status = getaddrinfo("127.0.0.1", "24243", &hints, &servcalc))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}

	//made a socket(IPv4 or IPv6, TCP), and bind it to the server sockaddr
	//[copy from Beej's Guide to Network Programming]
	for(o = servcalc; o !=NULL; o = o->ai_next)
	{
		if((socket_calc = socket(o->ai_family, o->ai_socktype, o->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_calc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		if(bind(socket_calc, o->ai_addr, o->ai_addrlen) == -1)
		{
			close(socket_calc);
			perror("bind");
			continue;
		}	
		break;
	}
	if((status = getaddrinfo("127.0.0.1", "22243", &hints, &serv))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}
	//made a socket(IPv4 or IPv6, TCP) to communicate with mainServer
	for(p = serv; p !=NULL; p = p->ai_next)
	{
		if((socket_connect = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_connect, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		
		break;
	}
	//check port number
	 // addr_size = sizeof my_addr;
	 // getsock_check=getsockname(socket_calc,(struct sockaddr*)&my_addr, (socklen_t *)&addr_size);
	 // if (getsock_check== -1) { perror("getsockname"); exit(1);}
	 // printf("port:%d\n", ntohs(my_addr.sin_port));

	//check if there is a avaliable socket
	if(o == NULL)
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}
	freeaddrinfo(serv);
	freeaddrinfo(servcalc);
	printf("The Calculation Server is up and running.\n");

	//loop to accept the request from mainServer
	int len_send, len_recv, bytes_sent, bytes_recv;
	long size, C, L, Vp;
	char *words[10];
	double td, pd;
	while(1)
	{
		size = 0; C = 0; L=0; Vp=0;
		addr_size = sizeof their_addr;
		len_recv = 100;
		char buf[len_recv];
		//receive message and split it
		bytes_recv = recvfrom(socket_calc, buf, len_recv-1, 0, (struct sockaddr *)&their_addr, &addr_size);//[copy from Beej's Guide to Network Programming]
		if(bytes_recv==-1)
		{
			perror("recv");
		}
		else if(bytes_recv>0)
		{
			int i = 0;
			buf[bytes_recv] = '\0';

			char *num;
			num = strtok(buf, " ");
			while(num != NULL)
			{
				words[i] = num;	
				i++;			
				num = strtok(NULL, " ");
			}
			size = atol(words[0]);
			C = atol(words[1]);
			L = atol(words[2]);
			Vp = atol(words[3]);
			printf("Receive request from Main Server.\n");
		}
		//calculate the delay
		td = ((double)(size*8)/C)*1000;
		pd = ((double)L/Vp)*1000;
		char msg[50] = "";
		sprintf(msg, "%.4f %.4f", td, pd);

		//send reply
		len_send = strlen(msg);//[copy from Beej's Guide to Network Programming]
		bytes_sent = sendto(socket_connect, msg, len_send, 0, p->ai_addr, p->ai_addrlen);//[copy from Beej's Guide to Network Programming]

		if(bytes_sent == -1)
		{
			perror("send");
		}
		else
		{
			printf("Send transmission delay %.2fms, propagation delay %.2fms, total delay %.2fms.\n\n", td, pd, td+pd);
		}
		
	}
	close(socket_connect);//[copy from Beej's Guide to Network Programming]
	close(socket_calc);
	return 0;
}