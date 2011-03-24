
CC = g++

LIBOPT = -L/usr/X11R6/lib -lGLEW -lglut -lm 

all : main

main: *.[ch] Makefile
	$(CC) -I. -ggdb -o main main.c $(LIBOPT)

clean:
	rm main
