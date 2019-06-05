


all: bankserver bankclient



bankserver: bankserver.o hash.o func.o queue.o

	gcc -o bankserver bankserver.o hash.o func.o queue.o -pthread



bankclient: bankclient.o func.o

	gcc -o bankclient bankclient.o func.o



bankserver.o: bankserver.c
	
	gcc -c bankserver.c -g -pthread



bankclient.o: bankclient.c
	
	gcc -c bankclient.c -g



hash.o: hash.c

	gcc -c hash.c -g


queue.o: queue.c

	gcc -c queue.c -g



func.o: func.c

	gcc -c func.c -g




clean:
	
	rm *o bankserver bankclient
