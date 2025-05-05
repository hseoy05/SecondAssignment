program.out : mainstreem.o directory_struct.o
	gcc -o program.out mainstreem.o directory_struct.o

mainstreem.o : mainstreem.c
	gcc -c -o mainstreem.o mainstreem.c

directory_struct.o : directory_struct.c
	gcc -c -o directory_struct.o directory_struct.c

clean:
	rm *.o program.out