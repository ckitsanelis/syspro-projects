



all: boardpost board


boardpost: boardpost.o list.o

	gcc -o boardpost boardpost.o list.o



board: board.o list.o

	gcc -o board board.o list.o



board.o: board.c
	
	gcc -c board.c -g



boardpost.o: boardpost.c
	
	gcc -c boardpost.c -g



list.o: list.c

	gcc -c list.c -g


clean:
	
	rm *o boardpost board
