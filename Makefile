output: input.txt main.c
	gcc -o output main.c
clean:
	rm log.txt output
