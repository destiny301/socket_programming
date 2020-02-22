all:
	gcc -o "Main Server" mainServer.c
	gcc -o Monitor monitor.c
	gcc -o "Database Server" dbServer.c
	gcc -o "Calculation Server" calcServer.c
	gcc -o client client.c

mainServer:
	./"Main Server"

dbServer:
	./"Database Server"

calcServer:
	./"Calculation Server"

monitor:
	./Monitor
