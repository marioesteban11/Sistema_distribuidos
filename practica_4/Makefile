.SUFFIXES: .o .c
.c.o:
	$(CC) -c $(CFLAGS) $<


CC = gcc
CFLAGS = -g -O2 -pthread

SRC_PROXY = proxy.h proxy.c
SRC_1 = publisher.c $(SRC_PROXY)
SRC_2 = broker.c $(SRC_PROXY)
SRC_2 = subscriber.c $(SRC_PROXY)

OBJ1 = publisher.o proxy.o
OBJ2 = broker.o proxy.o
OBJ2 = subscriber.o proxy.o

#all:  $(p1) $(p2)
all: $(OBJ1) $(OBJ2)
	gcc ${CFLAGS} -o publisher $(OBJ1)
	gcc ${CFLAGS} -o broker $(OBJ2)
	gcc ${CFLAGS} -o subscriber $(OBJ2)


clean:
	$(RM) $(OBJ1) $(OBJ2) publisher broker subscriber
