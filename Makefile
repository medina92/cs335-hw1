# cs335 lab1
# to compile your project, type make and press enter
CFLAGS = -I ./include
LIB    = ./libggfonts.so

all: lab1

lab1: lab1.cpp
	g++ lab1.cpp -Wall -olab1 $(LIB) -lrt -pthread -lX11 -lGL -lGLU -lm

clean:
	rm -f lab1
	rm -f *.o

