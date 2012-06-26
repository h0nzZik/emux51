# Makefile for linux and linux to windows cross-compiling

PROGRAM=emux51
VERSION=0.3.1

ifndef PREFIX
	PREFIX=/usr
endif

ifeq (${arch}, windows)
	BPREFIX		= i686-pc-mingw32-
	OUTDIR		= out/windows
	SHARED_EXT	= .dll
	EXECUTABLE_EXT	= .exe
	ARCH_LDFLAGS	= -lwinmm -mwindows
	PIC		=
	HOME_VAR	= USERPROFILE
#	ARCH_DEFINES	= -D PORTABLE

	BIN_PATH	= .
	LIB_PATH	= .
	DATA_PATH	= .
	DEF_GLADE_FILE	= ${DATA_PATH}/emux51.glade

else
	arch		= nixies
	BPREFIX		=
	OUTDIR		= out/nixies
	SHARED_EXT	= .so
	EXECUTABLE_EXT	=
	ARCH_LDFLAGS	= -export-dynamic
	PIC		= -fPIC
	HOME_VAR	= HOME
	
	BIN_PATH	= ${PREFIX}/bin
	LIB_PATH	= ${PREFIX}/lib
	DATA_PATH	= ${PREFIX}/share
	DEF_GLADE_FILE	= ${DATA_PATH}/emux51/emux51.glade

endif
#
#ifndef (${GLADE_FILE})
#	GLADE_FILE=emux51.glade
#endif
#
	
DATA_FILES	= emux51.glade emux51.png


MODULE_PATH	= ${LIB_PATH}/emux51-modules
MODULE_REGEX	= "^[a-zA-Z0-9].*"${SHARED_EXT}"$$"

PKG_CONFIG	= ${BPREFIX}pkg-config
CC		= ${BPREFIX}gcc

OUT		= ${OUTDIR}/${PROGRAM}${EXECUTABLE_EXT}
MODULE_OUT	= ${OUTDIR}/emux51-modules
GENERAL_CFLAGS	= -c -Wall -O2 -Wformat



GTK_NEW_DEFINES	= -D GTK_DISABLE_SINGLE_INCLUDES	\
		  -D GDK_DISABLE_DEPRECATED		\
		  -D GTK_DISABLE_DEPRECATED

DEFINES		= -D MODULE_EXTENSION=\"${SHARED_EXT}\"	\
		  -D HOME_VAR=\"${HOME_VAR}\"		\
		  -D MODULE_REGEX=\"${MODULE_REGEX}\"	\
		  -D UI_FILE=\"${DEF_GLADE_FILE}\"	\
		  -D MODULE_DIR=\"${MODULE_PATH}\"	\
		  ${GTK_NEW_DEFINES}			\
		  ${DEBUG_DEFINES}

INCLUDE		= -I include

GTK_CFLAGS	= `${PKG_CONFIG} --cflags gtk+-2.0`
GTK_LDFLAGS	= `${PKG_CONFIG} --libs   gtk+-2.0`

LDFLAGS		= ${GTK_LDFLAGS}		\
		  ${ARCH_LDFLAGS}

CFLAGS		= ${GENERAL_CFLAGS}		\
		  ${GTK_CFLAGS}			\
		  ${DEFINES}			\
		  ${ARCH_DEFINES}


MODULES_CFLAGS	= -D BUILDING_MODULE		\
		  ${INCLUDE}			\
		  ${PIC}			\
		  ${CFLAGS}


OBJ		= .obj/${arch}



#	Files to compile

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

modules		= 7seg.mod			\
		  led.mod			\
		  switch.mod			\
		  8x7seg.mod			\
		  keyboard.mod			\
		  5x7matrix.mod			\
		  5x7matrix-degraded.mod	\
		  dynamic_display.mod

#	Rules

build_all: directory widgets build modules

directory:
	mkdir -p ${OBJ}/modules
	mkdir -p ${MODULE_OUT}

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
	rm -f ${OUTDIR}/emux51-modules/*${SHARED_EXT}

install:
	mkdir -p ${BIN_PATH}
	mkdir -p ${LIB_PATH}/emux51-modules
	mkdir -p ${DATA_PATH}/emux51
	mkdir -p ${DATA_PATH}/applications
	cp ${DATA_FILES} ${DATA_PATH}/emux51
#	cp emux51.glade ${DATA_PATH}
#	cp emux51.png ${DATA_PATH}
	cp ${OUT} ${BIN_PATH}
	cp ${OUTDIR}/libemux_widgets${SHARED_EXT} ${LIB_PATH}
	cp -R ${OUTDIR}/emux51-modules/ ${LIB_PATH}
	cp emux51.desktop ${DATA_PATH}/applications

uninstall:
	rm -r ${LIB_PATH}/emux51-modules
	rm -r ${DATA_PATH}/emux51
	rm ${LIB_PATH}/libemux_widgets${SHARED_EXT}
	rm ${BIN_PATH}/emux51
	rm ${DATA_PATH}/applications/emux51.desktop

PKGDIR=emux51-${VERSION}
package:
	rm -rf ${PKGDIR}
	mkdir -p ${PKGDIR}
	cp -R src/ ${PKGDIR}
	cp -R include/ ${PKGDIR}
	cp emux51.png emux51.glade emux51.desktop ${PKGDIR}
	cp Makefile README gpl-3.0.txt ${PKGDIR}
	cp run runwin ${PKGDIR}
	tar -czf ${PKGDIR}.tar.gz ${PKGDIR}
	rm -rf ${PKGDIR}

