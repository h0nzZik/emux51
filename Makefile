# Makefile for linux and linux to windows cross-compiling
CFLAGS=-c -Wall -O2 -Wformat `${PKG-CONFIG} --cflags gtk+-2.0 glib-2.0`
INCLUDE=-I include

ifeq (${arch}, windows)
	PKG-CONFIG=mingw32-pkg-config
	CC=i686-pc-mingw32-gcc
	OUTDIR=out/windows


ifeq (${debug},1)
	LDFLAGS=`${PKG-CONFIG} --libs  gtk+-2.0 gthread-2.0` -lwinmm 
	OUT=${OUTDIR}/emux51-con.exe
else
	LDFLAGS=`${PKG-CONFIG} --libs  gtk+-2.0 gthread-2.0` -lwinmm -mwindows
	OUT=${OUTDIR}/emux51.exe
endif
	DEX=.dll
	HOME_VAR=USERPROFILE

else
	PKG-CONFIG=pkg-config
	arch=nixies
	LDFLAGS=`${PKG-CONFIG} --libs gtk+-2.0 gthread-2.0`
	OUT=out/nixies/emux51
	OUTDIR=out/nixies
	PIC=-fPIC
	DEX=.so
	HOME_VAR=HOME
endif

DEFINES=-DMODULE_EXTENSION=\"${DEX}\" -DHOME_VAR=\"${HOME_VAR}\"

OBJ=.obj/${arch}
LOG=out/${arch}/log.txt
archtarget=${arch}

targets=instructions emux51 module hex settings ${archtarget} gui alarm
objects=${OBJ}/instructions.o ${OBJ}/emux51.o ${OBJ}/${archtarget}.o ${OBJ}/module.o \
	${OBJ}/hex.o ${OBJ}/gui.o ${OBJ}/settings.o ${OBJ}/alarm.o

widgets=port_selector 7seg
widgeto=${OBJ}/port_selector.o ${OBJ}/7seg.o


.PHONY: clean
.PHONY: build_all
.PHONY: build

build:	${targets} gui alarm
	@ echo linking..
	@ ${CC} ${BUILD} -L${OUTDIR} -lemux_widgets ${objects} ${LDFLAGS}  -o ${OUT}	\
	  >> ${LOG}

build_all: widgets build modules

${targets}:
	@ echo ${CC} src/$@.c
	@ ${CC} ${INCLUDE} ${CFLAGS} ${DEFINES} -o ${OBJ}/$@.o src/emux51/$@.c 2>>${LOG}

mods:
	@ make --makefile=modules/Makefile ARCH=${ARCH}

widgets: ${widgets}
	@ echo linking widgets..
	@ ${CC} -shared -o ${OUTDIR}/libemux_widgets${DEX} ${widgeto} `${PKG-CONFIG} --libs gtk+-2.0`

${widgets}:
	@ echo ${CC} src/widgets/$@.c
	@${CC} ${INCLUDE} ${PIC} ${CFLAGS} -o ${OBJ}/$@.o src/widgets/$@.c


modules=3x7seg.mod 7seg.mod led.mod switch.mod 4x7seg.mod 8x7seg.mod #keyboard.mod

modules: ${modules}
${modules}:
	@ echo ${CC} src/modules/${@:.mod=.c}
	@ ${CC} -DBUILDING_MODULE ${INCLUDE} ${PIC} ${CFLAGS} -o ${OBJ}/modules/${@:.mod=.o}\
		src/modules/${@:.mod=.c}
	@ echo linking ${OBJ}/modules/${@:.mod=.o}
	@ ${CC} -shared -L${OUTDIR} -lemux_widgets -o ${OUTDIR}/modules/${@:.mod=${DEX}}\
		${OBJ}/modules/${@:.mod=.o} `${PKG-CONFIG} --libs gtk+-2.0`
log:
	@cat ${LOG}
lines:
	@ cat src/emux51/*.c src/modules/*.c src/widgets/*.c include/*.h Makefile | wc -l
clean:
	rm -f ${OBJ}/*.o
	rm -f ${OBJ}/modules/*.o
	rm -f ${LOG}
	rm -f ${OUT}
	rm -f ${OUTDIR}/libemux_widgets${DEX}
#	rm -f ${OUTDIR}/modules/*${DEX}

