# Makefile for linux and linux to windows cross-compiling
CFLAGS=-Wall -O2
INCLUDE=-I include

ifeq (${ARCH}, WIN32)
	PKG-CONFIG=mingw32-pkg-config
	archsrc=win32.c
	CC=i686-pc-mingw32-gcc
	LDFLAGS=`${PKG-CONFIG} --libs  gtk+-2.0 gthread-2.0` -lwinmm -mwindows
	OBJ=.wobj
	OUT=bin/emux51.exe
	LOG=log/log-win.txt
else
	PKG-CONFIG=pkg-config
	archsrc=posix.c
	LDFLAGS=`${PKG-CONFIG} --libs gtk+-2.0 gthread-2.0`
	OBJ=.obj
	OUT=bin/emux51
	LOG=log/log
endif

targets=instructions emux51 module hex settings
objects=${OBJ}/instructions.o ${OBJ}/emux51.o ${OBJ}/arch.o ${OBJ}/module.o \
	${OBJ}/hex.o ${OBJ}/gui.o ${OBJ}/settings.o


.PHONY: clean

build:	${targets} arch gui
	${CC} ${objects} ${LDFLAGS} -o ${OUT} >> ${LOG}

${targets}:
	${CC} ${INCLUDE} -c ${CFLAGS} -o ${OBJ}/$@.o src/$@.c 2>>${LOG}

arch:	
	${CC} ${INCLUDE} -c ${CFLAGS} -o ${OBJ}/arch.o src/arch/${archsrc} 2>>${LOG}
	
gui:
	${CC} ${INCLUDE} -c ${CFLAGS} `${PKG-CONFIG} --cflags gtk+-2.0`\
	-o ${OBJ}/gui.o src/gui.c 2>>${LOG} 
mods:
	make --makefile=modules/Makefile

clean:
	rm -f ${LOG}
	rm -f ${OBJ}/*
	rm -f ${OUT}
cleanall:
	rm -f log/*
	rm -f .obj/*
	rm -f .wobj/*
	rm -f bin/*
	
