CC = g++
LIBOPT = -lglut -lGLU -lGL -lm

all : main

main: *.[ch] Makefile
	$(CC) -I. -o main main.cpp $(LIBOPT)

clean:
	rm main