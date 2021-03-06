.PHONY:clean
CC=gcc
CFLAGS=-Wall -g 
BIN=miniftpd
OBJS=main.o sysutil.o session.o privparent.o ftpproto.o str.o parseconf.o tunable.o
LIBS=-lcrypt
$(BIN):$(OBJS)
	$(CC) $(CFLAGS) $^ -o $@ $(LIBS)
%.o:%.c
	$(CC) $(CFLAGS) -c $< -o $@
clean:
	rm -rf *.o $(BIN)
