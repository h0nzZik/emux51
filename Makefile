# Makefile

CFLAGS=-Wall -pedantic -O2 -ggdb3
targets=instructions emux51
objects=obj/instructions.o obj/emux51.o obj/arch.o

.PHONY: clean main

build: ${targets}
	${CC} ${objects} -o bin/emux51
${targets}:
	make --makefile=src/Makefile CFLAGS="${CFLAGS}" arch
	make --makefile=src/Makefile CFLAGS="${CFLAGS}" target=$@

clean:
	rm obj/*
	rm bin/*
	rm log

