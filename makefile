CC = gcc
CFLAGS = -Wall -pthread
#-Wall Todos los Warnings
#-pthread Necesario para los hiloss

#Lista los .c e indica que necesita sus .o
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

vm: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@
