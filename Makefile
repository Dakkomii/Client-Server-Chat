#makefile for project 2



CFLAGS = -Wall -g
CC     = gcc $(CFLAGS)


PROGRAMS = bl_client	bl_server	simpio_demo

all : $(PROGRAMS)

bl_server : bl_server.o server_funcs.o util.o blather.h
	$(CC) -o bl_server bl_server.o server_funcs.o util.o -lpthread

bl_client : bl_client.o simpio.o util.o blather.h
	$(CC) -o bl_client bl_client.o simpio.o util.o -lpthread

simpio_demo : simpio_demo.o simpio.o util.o blather.h
	$(CC) -o simpio_demo simpio_demo.o simpio.o util.o -lpthread

bl_client.o : bl_client.c blather.h
	$(CC) -c bl_client.c

simpio.o : simpio.c blather.h
	$(CC) -c simpio.c

bl_server.o : bl_server.c blather.h
	$(CC) -c bl_server.c

server_funcs.o : server_funcs.c blather.h
	$(CC) -c server_funcs.c

util.o : util.c blather.h
	$(CC) -c util.c

simpio_demo.o : simpio_demo.c blather.h
	$(CC) -c simpio_demo.c

clean :
	rm -f *.o *.fifo $(PROGRAMS)

include test_Makefile