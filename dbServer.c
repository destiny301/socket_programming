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

//a struct for some data of the link
struct link
{
	long id;
	long C;
	long L;
	long Vp;
};

int main(int argc, char *argv[])
{
	struct addrinfo hints, *serv, *servdb, *p, *o;
	struct sockaddr_storage their_addr;
	struct sockaddr_in my_addr;
	socklen_t addr_size;
	int status, getsock_check;
	int yes = 1;
	int socket_connect, socket_db;

	memset(&hints, 0, sizeof hints);//clear hints
	hints.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hints.ai_socktype = SOCK_DGRAM;//UDP

	//get sockaddr and other detailed information
	//[imitate from Beej's Guide to Network Programmin]
	if((status = getaddrinfo("127.0.0.1", "23243", &hints, &servdb))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}

	//made a socket(IPv4 or IPv6, TCP), and bind it to the server sockaddr
	//[copy from Beej's Guide to Network Programming]
	for(o = servdb; o !=NULL; o = o->ai_next)
	{
		if((socket_db = socket(o->ai_family, o->ai_socktype, o->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_db, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		if(bind(socket_db, o->ai_addr, o->ai_addrlen) == -1)
		{
			close(socket_db);
			perror("bind");
			continue;
		}	
		break;
	}
	//made a socket(IPv4 or IPv6, TCP) to communicate with mainserver
	if((status = getaddrinfo("127.0.0.1", "22243", &hints, &serv))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}
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
	// getsock_check=getsockname(socket_db,(struct sockaddr*)&my_addr, (socklen_t *)&addr_size);
	// if (getsock_check== -1) { perror("getsockname"); exit(1);}
	// printf("port:%d\n", ntohs(my_addr.sin_port));

	//check if there is a avaliable socket
	if(o == NULL)
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}		
	freeaddrinfo(serv);
	freeaddrinfo(servdb);
	printf("The Database Server is up and running.\n");

	//read data of the link from the data.txt
	FILE *fp = NULL;
	fp = fopen("database_final.txt", "r");
	struct link ln[10]={0};
	int i = 0;
	while(!feof(fp))
	{
		fscanf(fp, "%ld", &ln[i].id);
		fscanf(fp, "%ld", &ln[i].C);
		fscanf(fp, "%ld", &ln[i].L);
		fscanf(fp, "%ld\n", &ln[i].Vp);
		i++;
	}
	fclose(fp);

	long C, L, Vp;
	int len_send, len_recv, bytes_sent, bytes_recv;

	while(1)
	{
		C = 0; L=0; Vp=0;
		addr_size = sizeof their_addr;
		len_recv = 100;
		char buf[len_recv];
		long id;
		//receive request for the data
		bytes_recv = recvfrom(socket_db, buf, len_recv-1, 0, (struct sockaddr *)&their_addr, &addr_size);//[copy from Beej's Guide to Network Programming]
		if(bytes_recv==-1)
		{
			perror("recv");
		}
		else if(bytes_recv>0)
		{
			buf[bytes_recv] = '\0';
			id = atol(buf);
			printf("Receive request from Main Server.\n");
		}

		char msg[50] = "";
		for(int j = 0; j<i; j++)
		{
			if(id == ln[j].id)
			{
				C = ln[j].C;
				L = ln[j].L;
				Vp = ln[j].Vp;
				sprintf(msg, "%ld %ld %ld %ld", id, C, L, Vp);
			}
		}
		if(C == 0)
			{
				id = 0;
				sprintf(msg, "%ld %ld %ld %ld", id, C, L, Vp);
			}
		//send reply
		len_send = strlen(msg);//[copy from Beej's Guide to Network Programming]
		bytes_sent = sendto(socket_connect, msg, len_send, 0, p->ai_addr, p->ai_addrlen);//[copy from Beej's Guide to Network Programming]

		if(bytes_sent == -1)
		{
			perror("send");
		}
		else
		{
			if(C!=0)
				printf("Send link %ld, capacity %ldMbps, link length %ldkm, propagation velocity %ldkm/s.\n\n", id, C, L, Vp);
			else 
				printf("No match found.\n\n");
		}
		
	}
	close(socket_connect);//[copy from Beej's Guide to Network Programming]
	close(socket_db);
	return 0;
}