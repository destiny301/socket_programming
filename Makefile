all: 
	gcc -o "Main Server" mainServer.c
	gcc -o "Database Server" dbServer.c
	gcc -o "Calculation Server" calcServer.c

mainServer:
	./"Main Server"

dbServer:
	./"Database Server"

calcServer:
	./"Calculation Server"
