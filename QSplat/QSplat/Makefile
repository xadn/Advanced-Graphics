OS=$(shell uname)

ifeq ($(OS), Darwin)
	CC = g++
	OPTS = -O3 -funroll-loops -ftree-vectorize -NDEBUG -msse3
	LIBOPT = -framework GLUT -framework OpenGL -framework Cocoa
else
	CC = g++
	LIBOPT = -lglut -lGLU -lGL -lm
endif

all : main

main: *.[ch] Makefile
	$(CC) -I. -o main *.cpp $(OPTS) $(LIBOPT)

clean:
	rm -rf main