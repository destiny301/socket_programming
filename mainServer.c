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
	struct addrinfo hint1, hint2, *serv1, *serv2, *servM, *servdb, *servcalc, *p1, *p2, *p3, *p4, *p5;
	struct sockaddr_storage their_addr;
	struct sockaddr_in my_addr;
	socklen_t addr_size;
	int status, getsock_check;
	int yes = 1;
	int socket_tcp, socket_udp, socket_monitor, socket_db, socket_calc, socket_mesg, socket_display;
	char *message;
	char *name, *name_client;

	memset(&hint1, 0, sizeof hint1);//clear hints
	hint1.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hint1.ai_socktype = SOCK_STREAM;//TCP

	memset(&hint2, 0, sizeof hint2);//clear hints
	hint2.ai_family = AF_UNSPEC;//IPv4 or TPv6
	hint2.ai_socktype = SOCK_DGRAM;//UDP

	//get sockaddr and other detailed information
	//[imitate from Beej's Guide to Network Programmin]
	if((status = getaddrinfo("127.0.0.1", "21243", &hint1, &serv1))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}

	if((status = getaddrinfo("127.0.0.1", "25243", &hint1, &servM))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}

	if((status = getaddrinfo("127.0.0.1", "22243", &hint2, &serv2))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}
	if((status = getaddrinfo("127.0.0.1", "23243", &hint2, &servdb))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}
	if((status = getaddrinfo("127.0.0.1", "24243", &hint2, &servcalc))!=0)
	{
		fprintf(stderr, "getaddrinfo%s\n", gai_strerror(status));
		exit(1);
	}


	//made a socket(IPv4 or IPv6, TCP), and bind it to the server sockaddr
	//[copy from Beej's Guide to Network Programming]
	for(p1 = serv1; p1 !=NULL; p1 = p1->ai_next)
	{
		if((socket_tcp = socket(p1->ai_family, p1->ai_socktype, p1->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_tcp, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		if(bind(socket_tcp, p1->ai_addr, p1->ai_addrlen) == -1)
		{
			close(socket_tcp);
			perror("bind");
			continue;
		}	
		break;
	}
	//p5 = socket_monitor
	for(p5 = servM; p5 !=NULL; p5 = p5->ai_next)
	{
		if((socket_monitor = socket(p5->ai_family, p5->ai_socktype, p5->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_monitor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		if(bind(socket_monitor, p5->ai_addr, p5->ai_addrlen) == -1)
		{
			close(socket_monitor);
			perror("bind");
			continue;
		}	
		break;
	}
	//udp port
	for(p2 = serv2; p2 !=NULL; p2 = p2->ai_next)
	{
		if((socket_udp = socket(p2->ai_family, p2->ai_socktype, p2->ai_protocol)) == -1)
		{
			perror("socket");
			continue;
		}
		//address the "Address already in use" problem, reuse the port
		if(setsockopt(socket_udp, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes) == -1)
		{
			perror("setsockopt");
			exit(1);
		}
		if(bind(socket_udp, p2->ai_addr, p2->ai_addrlen) == -1)
		{
			close(socket_udp);
			perror("bind");
			continue;
		}	
		break;
	}
	//database server
	for(p3 = servdb; p3 !=NULL; p3 = p3->ai_next)
	{
		if((socket_db = socket(p3->ai_family, p3->ai_socktype, p3->ai_protocol)) == -1)
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
	//calculation server
	for(p4 = servcalc; p4 !=NULL; p4 = p4->ai_next)
	{
		if((socket_calc = socket(p4->ai_family, p4->ai_socktype, p4->ai_protocol)) == -1)
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
	freeaddrinfo(serv1);
	freeaddrinfo(serv2);
	freeaddrinfo(servM);
	freeaddrinfo(servdb);
	freeaddrinfo(servcalc);


	//check if there is a avaliable socket
	if(p1 == NULL)
	{
		fprintf(stderr, "server:failed to bind\n");
		exit(1);
	}
	//listen to other clients, the max of waiting queue is 20
	if((listen(socket_tcp, 20))==-1)
	{
		perror("listen");
		exit(1);
	}

	if((listen(socket_monitor, 20))==-1)
	{
		perror("listen");
		exit(1);
	}

	printf("The Main Server is up and running\n");

	long id, size;
	int len_send, len_recv, bytes_sent, bytes_recv;
	long value[4];
	char *words[10];
	double Tt, Tp;

	addr_size = sizeof their_addr;
	socket_display = accept(socket_monitor, (struct sockaddr *)&their_addr, &addr_size);

	//loop to accept the request from clients
	while(1)
	{
		//accept a request from one client
		//[copy from Beej's Guide to Network Programming]
		addr_size = sizeof their_addr;
		socket_mesg = accept(socket_tcp, (struct sockaddr *)&their_addr, &addr_size);

		len_recv = 100;
		char buff[len_recv];

		//receive message from client
		bytes_recv = recv(socket_mesg, buff, len_recv, 0);//[copy from Beej's Guide to Network Programming]
		if(bytes_recv==-1)
		{
			perror("recv");
		}
		else if(bytes_recv>0)
		{
			int i = 0, j = 0;
			buff[bytes_recv] = '\0';

			//get id and size from received buf[]
			id = atol(strtok(buff, " "));
			size = atol(strtok(NULL, " "));

			printf("Receive Link %ld, file size %ldMB.\n", id, size);

			//send reply to monitor
			char message1[50] = "";
			sprintf(message1, "Main server receives Link %ld and file size %ldMB from client.\n", id, size);

			len_send = strlen(message1);//[copy from Beej's Guide to Network Programming]
			bytes_sent = send(socket_display, message1, len_send, 0);//[copy from Beej's Guide to Network Programming]

			if(bytes_sent == -1)
			{
				perror("send");
			}
		}


		//send id to database server
		char msg[50] = "";
		sprintf(msg, "%ld", id);
		len_send = strlen(msg);//[copy from Beej's Guide to Network Programming]
		bytes_sent = sendto(socket_db, msg, len_send, 0, p3->ai_addr, p3->ai_addrlen);//[copy from Beej's Guide to Network Programming]
		if(bytes_sent == -1)
		{
			perror("sendto");
			exit(1);
		}
		else
		{
			printf("Send Link %ld to database server.\n", id);

			//send reply to monitor
			char message2[50] = "";
			sprintf(message2, "Main server sends Link %ld to database server.\n", id);

			len_send = strlen(message2);//[copy from Beej's Guide to Network Programming]
			bytes_sent = send(socket_display, message2, len_send, 0);//[copy from Beej's Guide to Network Programming]

			if(bytes_sent == -1)
			{
				perror("send");
			}
		}

		//accept information from database server
		addr_size = sizeof their_addr;
		len_recv = 100;
		char buf[len_recv];
		bytes_recv = recvfrom(socket_udp, buf, len_recv-1, 0, (struct sockaddr *)&their_addr, &addr_size);//[copy from Beej's Guide to Network Programming]
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
			char message3[50] = "";
			if(value[0]==0)
			{
				sprintf(message3, "Main server receives information from database server: no match found.\n\n");
				printf("Receive no match found.\n\n");

			}
			else
			{
				sprintf(message3, "Main server receives information from database server: link capacity %ldMbps, link length %ldkm, and propagation velocity %ldkm/s.\n", value[1], value[2], value[3]);
				printf("Receive link capacity %ldMbps, link length %ldkm, and propagation velocity %ldkm/s.\n", value[1], value[2], value[3]);
			}
			//send reply to monitor
			len_send = strlen(message3);//[copy from Beej's Guide to Network Programming]
			bytes_sent = send(socket_display, message3, len_send, 0);//[copy from Beej's Guide to Network Programming]

			if(bytes_sent == -1)
			{
				perror("send");
			}
		}
		//send to calculate
		if(value[0]!=0)
		{
			char message[50] = "";
			sprintf(message, "%ld %ld %ld %ld", size, value[1], value[2], value[3]);
			len_send = strlen(message);//[copy from Beej's Guide to Network Programming]
			bytes_sent = sendto(socket_calc, message, len_send, 0, p4->ai_addr, p4->ai_addrlen);//[copy from Beej's Guide to Network Programming]
			if(bytes_sent == -1)
			{
				perror("sendto");
			}
			else
			{
				printf("Send information to calculation server.\n");

				//send reply to monitor
				char message4[50] = "";
				sprintf(message4, "Main Server sends information to calculation server.\n");

				len_send = strlen(message4);//[copy from Beej's Guide to Network Programming]
				bytes_sent = send(socket_display, message4, len_send, 0);//[copy from Beej's Guide to Network Programming]

				if(bytes_sent == -1)
				{
					perror("send");
				}
			}

			//receive the calculation result
			addr_size = sizeof their_addr;
			len_recv = 100;
			char result[len_recv];
			bytes_recv = recvfrom(socket_udp, result, len_recv-1, 0, (struct sockaddr *)&their_addr, &addr_size);//[copy from Beej's Guide to Network Programming]
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

				//send reply to monitor
				char message5[50] = "";
				sprintf(message5, "Main Server receives information from calculation server.\n");

				len_send = strlen(message5);//[copy from Beej's Guide to Network Programming]
				bytes_sent = send(socket_display, message5, len_send, 0);//[copy from Beej's Guide to Network Programming]

				if(bytes_sent == -1)
				{
					perror("send");
				}
			}
		}

		//send reply to client
		char msg2[50] = "";

		if(value[0]!=0)
			sprintf(msg2, "%.2f %.2f", Tt, Tp);
		else
			sprintf(msg2, "no");

		len_send = strlen(msg2);//[copy from Beej's Guide to Network Programming]
		bytes_sent = send(socket_mesg, msg2, len_send, 0);//[copy from Beej's Guide to Network Programming]

		if(bytes_sent == -1)
		{
			perror("send");
		}
		else
		{
			//send reply to monitor
			char message6[50] = "";
			if(value[0]!=0)
			{
				sprintf(message6, "Main Server sends information to client: transmission delay %.2fms, propagation delay %.2fms, total delay %.2fms.\n\n", Tt, Tp, Tt+Tp);
				len_send = strlen(message6);//[copy from Beej's Guide to Network Programming]
				bytes_sent = send(socket_display, message6, len_send, 0);//[copy from Beej's Guide to Network Programming]

				if(bytes_sent == -1)
				{
					perror("send");
				}
			}
		}

		close(socket_mesg);//[copy from Beej's Guide to Network Programming]
	}
	close(socket_display);
	close(socket_monitor);
	close(socket_tcp);//[copy from Beej's Guide to Network Programming]
	close(socket_calc);
	close(socket_db);
	close(socket_udp);
	return 0;
}