

OS=$(shell uname)

ifeq ($(OS), Darwin)
    CC = g++
	LIBOPT = -lGLEW -framework GLUT -framework OpenGL -framework Cocoa
else
    CC = g++
	LIBOPT = -L/usr/X11R6/lib -lGLEW -lglut -lm 
endif

all : main

main: Makefile
	$(CC) -I. -ggdb -o main main.cpp $(LIBOPT)

clean:
	rm -rf main main.dSYM
