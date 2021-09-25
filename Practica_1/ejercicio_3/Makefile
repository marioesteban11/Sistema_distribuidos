
.SUFFIXES: .o .c
.c.o:
	$(CC) -c $(CFLAGS) $<


CC = gcc
CFLAGS = -g -O2 -pthread

SRC_PROXY = proxy.h proxy.c
SRC_1 = client.c $(SRC_PROXY)
SRC_2 = server.c $(SRC_PROXY)

OBJ1 = client.o 
OBJ2 = server.o 

#all:  $(p1) $(p2)
all: $(OBJ1) $(OBJ2)
	gcc ${CFLAGS} -o client $(OBJ1)
	gcc ${CFLAGS} -o server $(OBJ2)


clean:
	$(RM) $(OBJ1) $(OBJ2) client server
