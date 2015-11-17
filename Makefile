OBJS = client serverA serverB serverC serverD
CC = gcc

all: client serverA serverB serverC serverD

client:
	$(CC) -o client client.c 

serverA:
	$(CC) -o serverA serverA.c 

serverB:
	$(CC) -o serverB serverB.c 

serverC:
	$(CC) -o serverC serverC.c 

serverD:
	$(CC) -o  serverD serverD.c 

clean: 
	rm -rf $(OBJS)
