all: HW2

HW2: hw2.c
	gcc -o HW2 hw2.c

clean:
	rm -rf HW2
