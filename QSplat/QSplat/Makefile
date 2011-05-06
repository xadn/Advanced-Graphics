OS=$(shell uname)

ifeq ($(OS), Darwin)
	CC = g++
	LIBOPT = -framework GLUT -framework OpenGL -framework Cocoa
else
	CC = g++
	LIBOPT = -lglut -lGLU -lGL -lm
endif

all : main

main: *.[ch] Makefile
	$(CC) -I. -o main main.cpp $(LIBOPT)

clean:
	rm -rf main