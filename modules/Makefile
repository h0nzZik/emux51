#modules/Makefile

target=led switch
includes=-I include 
cflags=-c -fPIC `pkg-config --cflags gtk+-2.0`

all: ${target}

${target}:
	${CC} ${includes} ${cflags} -o modules/.obj/$@.o \
		modules/src/$@.c
	${CC} -shared -o modules/bin/$@.so modules/.obj/$@.o
	rm modules/.obj/$@.o
	
