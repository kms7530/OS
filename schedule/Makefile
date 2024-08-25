sched: main.o
	gcc -o sched main.o -lrt

main.o: main.c
	gcc -c -o main.o main.c -lrt

clean:
	rm -rf main.o sched