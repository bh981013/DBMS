all: main.o file.o db.o bpt.o buffer.o
	gcc -g -o main main.o file.o db.o bpt.o buffer.o

main.o: main.c
	gcc -g -c -o main.o main.c

file.o: file.h file.c
	gcc -g -c -o file.o file.c
db.o: db.h db.c
	gcc -g -c -o db.o db.c
bpt.o: bpt.h bpt.c
	gcc -g -c -o bpt.o bpt.c
buffer.o: buffer.h buffer.c
	gcc -g -c -o buffer.o buffer.c

clean:
	rm *.o main file bpt db buffer
