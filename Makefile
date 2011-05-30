# Makefile

CFLAGS=-Wall -O2 -ggdb3
targets=instructions emux51 module
objects=obj/instructions.o obj/emux51.o obj/arch.o obj/module.o

.PHONY: clean main

build: ${targets}
#	${CC}  -ldl ${objects} -o bin/emux51
	${CC} -lpthread `pkg-config --libs gtk+-2.0` -ldl ${objects} -o bin/emux51
${targets}:
	make --makefile=src/Makefile CFLAGS="${CFLAGS}" arch
	make --makefile=src/Makefile CFLAGS="${CFLAGS}" target=$@
mods:
	make --makefile=modules/Makefile

clean:
#	if [ -e log ]; then rm log; fi
	>log
	rm obj/*
	rm bin/*

