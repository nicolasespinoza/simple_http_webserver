CC = gcc
CFLAGS = -Wall -pedantic -g -std=c99
MAIN = server
OBJS = server.o toolkit.o array_list.o net.o

all : $(MAIN)

$(MAIN) : $(OBJS) httpd/util/toolkit.h httpd/util/array_list.h httpd/net.h
	$(CC) $(CFLAGS) -o $(MAIN) $(OBJS)

server.o : httpd/server.c httpd/util/toolkit.h httpd/util/array_list.h httpd/net.h
	$(CC) $(CFLAGS) -c httpd/server.c

toolkit.o : httpd/util/toolkit.c
	$(CC) $(CFLAGS) -c httpd/util/toolkit.c

array_list.o : httpd/util/array_list.c
	$(CC) $(CFLAGS) -c httpd/util/array_list.c

net.o : httpd/net.c
	$(CC) $(CFLAGS) -c httpd/net.c

clean :
	rm *.o $(MAIN) core*