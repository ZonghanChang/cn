OBJS = client serverA serverB serverC serverD
CC = gcc

all: client serverA serverB serverC serverD

client:
	$(CC) -o client client.c -lsocket -lnsl -lresolv

serverA:
	$(CC) -o serverA serverA.c -lsocket -lnsl -lresolv

serverB:
	$(CC) -o serverB serverB.c -lsocket -lnsl -lresolv

serverC:
	$(CC) -o serverC serverC.c -lsocket -lnsl -lresolv

serverD:
	$(CC) -o  serverD serverD.c -lsocket -lnsl -lresolv

clean: 
	rm -rf $(OBJS)
