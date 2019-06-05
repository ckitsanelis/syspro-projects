






all: elegxos




elegxos: main.o hash.o graph.o stack.o

	gcc -o elegxos main.o hash.o graph.o stack.o



main.o: main.c
	
	gcc -c main.c -g



hash.o: hash.c

	gcc -c hash.c -g


graph.o: graph.c

	gcc -c graph.c -g

stack.o: stack.c

	gcc -c stack.c -g



clean:
	
	rm *o elegxos

