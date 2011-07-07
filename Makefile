# Makefile

CFLAGS=-Wall -O2

targets=instructions emux51 module hex
objects=.obj/instructions.o .obj/emux51.o .obj/arch.o .obj/module.o \
	.obj/hex.o .obj/gui.o

ARCH=posix

.PHONY: clean

build:	${targets} arch gui
	${CC} -lpthread `pkg-config --libs gtk+-2.0 gthread-2.0` -ldl ${objects} -o bin/emux51 >> log

${targets}:
	${CC} -I include -c ${CFLAGS} -o .obj/$@.o src/$@.c 2>>log

arch:	
	${CC} -I include -c ${CFLAGS} -o .obj/arch.o src/arch/${ARCH}.c 2>>log
	
gui:
	${CC} -I include -c ${CFLAGS} `pkg-config --cflags gtk+-2.0`\
	-o .obj/gui.o src/gui.c 2>>log

mods:
	make --makefile=modules/Makefile

clean:
	>log
	rm .obj/*
	rm bin/*

