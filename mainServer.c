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

int main()
{
	struct addrinfo hints, *serv, *servdb, *servcalc, *o, *p, *q;
	struct sockaddr_storage their_addr;
	struct sockaddr_in my_addr;
	socklen_t addr_size;
	int status, getsock_check;
	int yes = 1;
	int socket_connect, socket_db, socket_calc;

	memset(&hints, 0, sizeof hints);//clear hints
	hints.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hints.ai_socktype = SOCK_DGRAM;//UDP

	//get sockaddr and other detailed information
	//[imitate from Beej's Guide to Network Programmin]
	if((status = getaddrinfo("127.0.0.1", "22243", &hints, &serv))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}
	if((status = getaddrinfo("127.0.0.1", "23243", &hints, &servdb))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}
	if((status = getaddrinfo("127.0.0.1", "24243", &hints, &servcalc))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}

	//made a socket(IPv4 or IPv6, TCP), and bind it to the server sockaddr
	//[copy from Beej's Guide to Network Programming]
	for(o = serv; o !=NULL; o = o->ai_next)
	{
		if((socket_connect = socket(o->ai_family, o->ai_socktype, o->ai_protocol)) == -1)
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
		if(bind(socket_connect, o->ai_addr, o->ai_addrlen) == -1)
		{
			close(socket_connect);
			perror("bind");
			continue;
		}	
		break;
	}

	for(p = servdb; p !=NULL; p = p->ai_next)
	{
		if((socket_db = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
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
		break;
	}

	for(q = servcalc; q !=NULL; q = q->ai_next)
	{
		if((socket_calc = socket(q->ai_family, q->ai_socktype, q->ai_protocol)) == -1)
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
		
		break;
	}
	//check port number
	// addr_size = sizeof my_addr;
	// getsock_check=getsockname(socket_connect,(struct sockaddr*)&my_addr, (socklen_t *)&addr_size);
	// if (getsock_check== -1) { perror("getsockname"); exit(1);}
	// printf("port:%d\n", ntohs(my_addr.sin_port));
	
	//free the linked list
	freeaddrinfo(serv);
	freeaddrinfo(servdb);
	freeaddrinfo(servcalc);

	//check if there is a avaliable socket
	if(o == NULL)
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}
	
	printf("The Main Server is up and listening.\n");

	//loop to communicate with the database server and the calculation server
	long id, size;
	int len_send, len_recv, bytes_sent, bytes_recv;
	long value[4];
	char *words[10];
	double Tt, Tp;
	while(1)
	{
		printf("Please input link ID and file size(<ID> <size>):\n");
		scanf("%ld %ld", &id, &size);
		printf("Link %ld, file size %ldMB.\n", id, size);

		//send id
		char msg[50] = "";
		sprintf(msg, "%ld", id);
		len_send = strlen(msg);//[copy from Beej's Guide to Network Programming]
		bytes_sent = sendto(socket_db, msg, len_send, 0, p->ai_addr, p->ai_addrlen);//[copy from Beej's Guide to Network Programming]
		if(bytes_sent == -1)
		{
			perror("sendto");
			exit(1);
		}
		else
		{
			printf("Send Link %ld to database server.\n", id);
		}

		//accept information from database server
		addr_size = sizeof their_addr;
		len_recv = 100;
		char buf[len_recv];
		bytes_recv = recvfrom(socket_connect, buf, len_recv-1, 0, (struct sockaddr *)&their_addr, &addr_size);//[copy from Beej's Guide to Network Programming]
		if(bytes_recv==-1)
		{
			perror("recvfrom");
			exit(1);

		}
		else if(bytes_recv>0)
		{
			int i = 0;
			buf[bytes_recv] = '\0';
			//split the information received from the database
			char *num;
			num = strtok(buf, " ");
			while(num != NULL)
			{
				words[i] = num;	
				i++;			
				num = strtok(NULL, " ");
			}
			//divert string into long int number
			value[0] = atol(words[0]);
			value[1] = atol(words[1]);
			value[2] = atol(words[2]);
			value[3] = atol(words[3]);
			if(value[0]==0)
			{
				printf("Receive no match found.\n\n");

			}
			else
				printf("Receive link capacity %ldMbps, link length %ldkm, and propagation velocity %ldkm/s.\n", value[1], value[2], value[3]);
		}
		//send to calculate
		if(value[0]==0)
			continue;
		else
		{
		char message[50] = "";
		sprintf(message, "%ld %ld %ld %ld", size, value[1], value[2], value[3]);
		len_send = strlen(message);//[copy from Beej's Guide to Network Programming]
		bytes_sent = sendto(socket_calc, message, len_send, 0, q->ai_addr, q->ai_addrlen);//[copy from Beej's Guide to Network Programming]
		if(bytes_sent == -1)
		{
			perror("sendto");
		}
		else
		{
			printf("Send information to calculation server.\n");
		}

		//receive the calculation result
		addr_size = sizeof their_addr;
		len_recv = 100;
		char result[len_recv];
		bytes_recv = recvfrom(socket_connect, result, len_recv-1, 0, (struct sockaddr *)&their_addr, &addr_size);//[copy from Beej's Guide to Network Programming]
		if(bytes_recv==-1)
		{
			perror("recv");
		}
		else if(bytes_recv>0)
		{
			int j = 0;
			result[bytes_recv] = '\0';
			//split transformation delay and propagation delay
			char *number;
			number = strtok(result, " ");
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
		
	}
	close(socket_connect);//[copy from Beej's Guide to Network Programming]
	close(socket_calc);
	close(socket_db);
	return 0;
}