# Makefile for linux and linux to windows cross-compiling

PROGRAM=emux51

ifeq (${arch}, windows)
	PREFIX		= i686-pc-mingw32-
	OUTDIR		= out/windows
	SHARED_EXT	= .dll
	EXECUTABLE_EXT	= .exe
	ARCH_LDFLAGS	= -lwinmm -mwindows
	PIC		=
	HOME_VAR	= USERPROFILE
	USER_DEFINES	= -D PORTABLE
else
	arch		= nixies
	PREFIX		=
	OUTDIR		= out/nixies
	SHARED_EXT	= .so
	EXECUTABLE_EXT	=
	ARCH_LDFLAGS	= -export-dynamic
	PIC		= -fPIC
	HOME_VAR	= HOME
endif

ifndef (${GLADE_FILE})
	GLADE_FILE=emux.glade
endif

PKG_CONFIG	= ${PREFIX}pkg-config
CC		= ${PREFIX}gcc
OUT		= ${OUTDIR}/${PROGRAM}${EXECUTABLE_EXT}
MODULE_OUT	= ${OUTDIR}/modules
GENERAL_CFLAGS	= -c -Wall -O2 -Wformat


GTK_NEW_DEFINIES= -D GTK_DISABLE_SINGLE_INCLUDES	\
		  -D GDK_DISABLE_DEPRECATED		\
		  -D GTK_DISABLE_DEPRECATED

DEFINES		= -D MODULE_EXTENSION=\"${SHARED_EXT}\"	\
		  -D HOME_VAR=\"${HOME_VAR}\"		\
		  -D EMUX51_GLADE_FILE=\"${GLADE_FILE}\"\
		  ${GTK_NEW_DEFINES}

INCLUDE		= -I include

GTK_CFLAGS	= `${PKG_CONFIG} --cflags gtk+-2.0`
GTK_LDFLAGS	= `${PKG_CONFIG} --libs   gtk+-2.0`

LDFLAGS		= ${GTK_LDFLAGS}		\
		  ${ARCH_LDFLAGS}

CFLAGS		= ${GENERAL_CFLAGS}		\
		  ${GTK_CFLAGS}			\
		  ${DEFINES}			\
		  ${USER_DEFINES}


MODULES_CFLAGS	= -D BUILDING_MODULE		\
		  ${INCLUDE}			\
		  ${PIC}			\
		  ${CFLAGS}


OBJ		= .obj/${arch}



#	FILES

archtarget	= ${arch}

targets		= instructions			\
		  emux51			\
		  module			\
		  hex				\
		  settings			\
		  ${archtarget}			\
		  gui				\
		  alarm				\
		  lists

objects		= ${OBJ}/instructions.o		\
		  ${OBJ}/emux51.o		\
		  ${OBJ}/${archtarget}.o	\
		  ${OBJ}/module.o		\
		  ${OBJ}/hex.o			\
		  ${OBJ}/gui.o			\
		  ${OBJ}/settings.o		\
		  ${OBJ}/alarm.o		\
		  ${OBJ}/lists.o

widgets		= port_selector			\
		  7seg				\
		  led

widgeto		= ${OBJ}/port_selector.o	\
		  ${OBJ}/7seg.o			\
		  ${OBJ}/led.o

modules		= 3x7seg.mod			\
		  7seg.mod			\
		  led.mod			\
		  switch.mod			\
		  4x7seg.mod			\
		  8x7seg.mod			\
		  keyboard.mod			\
		  hello.mod			\
		  5x7matrix.mod			\
		  5x7matrix-degraded.mod

#.PHONY: clean
#.PHONY: build_all
#.PHONY: build

build_all: directory widgets build modules


directory:
	mkdir -p ${OBJ}/modules
	mkdir -p ${OUTDIR}/modules


build:	${targets} gui alarm
	@ echo 'linking..'
	@ ${CC} ${BUILD} -L${OUTDIR} ${objects} ${LDFLAGS}  -o ${OUT}

${targets}:
	@ echo '${CC} src/$@.c'
	@ ${CC} ${INCLUDE} ${CFLAGS} ${DEFINES} -o ${OBJ}/$@.o src/emux51/$@.c

widgets: ${widgets}
	@ echo 'linking widgets..'
	@ ${CC} -shared -o ${OUTDIR}/libemux_widgets${SHARED_EXT} ${widgeto} ${GTK_LDFLAGS}

${widgets}:
	@ echo ${CC} src/widgets/$@.c
	@ ${CC} ${INCLUDE} ${PIC} ${CFLAGS} -o ${OBJ}/$@.o src/widgets/$@.c



modules: ${modules}
${modules}:
	@ echo '${CC} src/modules/${@:.mod=.c}'

	@ ${CC} ${MODULES_CFLAGS} -o ${OBJ}/modules/${@:.mod=.o} src/modules/${@:.mod=.c}

	@ echo 'linking ${OBJ}/modules/${@:.mod=.o}'
	@ ${CC} -shared -L${OUTDIR} -lemux_widgets \
	  -o ${MODULE_OUT}/${@:.mod=${SHARED_EXT}}\
	  ${OBJ}/modules/${@:.mod=.o} ${GTK_LDFLAGS}
lines:
	@ cat src/emux51/*.c src/modules/*.c src/widgets/*.c include/*.h Makefile | wc -l
clean:
	rm -f ${OBJ}/*.o
	rm -f ${OBJ}/modules/*.o
	rm -f ${OUT}
	rm -f ${OUTDIR}/libemux_widgets${SHARED_EXT}
	rm -f ${OUTDIR}/modules/*${SHARED_EXT}

install:
	mkdir -p ${INSTALL_PREFIX}/bin
	mkdir -p ${INSTALL_PREFIX}/lib/emux51-modules
	mkdir -p ${INSTALL_PREFIX}/share/emux51
	cp emux.glade ${INSTALL_PREFIX}/share/emux51/emux.glade
	cp ${OUT} ${INSTALL_PREFIX}/bin
	cp ${OUTDIR}/libemux_widgets${SHARED_EXT} ${INSTALL_PREFIX}/lib
	cp -R ${OUTDIR}/modules/ ${INSTALL_PREFIX}/lib/emux51-modules
