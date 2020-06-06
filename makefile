all: cmatrix-bitmap

cmatrix-bitmap: cmatrix-bitmap.c
	gcc -g -o cmatrix-bitmap cmatrix-bitmap.c


clean:
	rm -f cmatrix-bitmap.o cmatrix-bitmap