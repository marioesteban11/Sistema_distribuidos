
.SUFFIXES: .o .c
.c.o:
	$(CC) -c $(CFLAGS) $<


CC = gcc
CFLAGS = -g -O2 -pthread

SRC_PROXY = proxy.h proxy.c
SRC_1 = p1.c $(SRC_PROXY)
SRC_2 = p2.c $(SRC_PROXY)
SRC_3 = p3.c $(SRC_PROXY)

OBJ1 = p1.o
OBJ2 = p2.o
OBJ3 = p3.o

#all:  $(p1) $(p2)
all: $(OBJ1) $(OBJ2) $(OBJ3)
	gcc ${CFLAGS} -o p1 $(OBJ1)
	gcc ${CFLAGS} -o p2 $(OBJ2)
	gcc ${CFLAGS} -o p3 $(OBJ3)

clean:
	$(RM) $(OBJ1) $(OBJ2) $(OBJ3) client server
