# Makefile for linux and linux to windows cross-compiling
#TODO: -D GTK_DISABLE_DEPRECATED
GTK_NEW_FLAGS=-D GTK_DISABLE_SINGLE_INCLUDES -DGDK_DISABLE_DEPRECATED 
CFLAGS=-c -Wall -O2 -Wformat `${PKG-CONFIG} --cflags gtk+-2.0 glib-2.0` ${GTK_NEW_FLAGS}
INCLUDE=-I include
GTK_LDFLAGS=`${PKG-CONFIG} --libs  gtk+-2.0`

ifeq (${arch}, windows)
	PKG-CONFIG=mingw32-pkg-config
	CC=i686-pc-mingw32-gcc
ifndef (${OUTDIR})
	OUTDIR=out/windows
endif

ifeq (${debug},1)
	LDFLAGS=${GTK_LDFLAGS} -lwinmm 
	OUT=${OUTDIR}/emux51-con.exe
else
	LDFLAGS=${GTK_LDFLAGS} -lwinmm -mwindows
	OUT=${OUTDIR}/emux51.exe
endif
	DEX=.dll
	HOME_VAR=USERPROFILE

else
	PKG-CONFIG=pkg-config
	arch=nixies
	LDFLAGS=${GTK_LDFLAGS}
ifndef (${OUTDIR})
	OUTDIR=out/nixies
endif
	OUT=${OUTDIR}/emux51
	PIC=-fPIC
	DEX=.so
	HOME_VAR=HOME
endif

DEFINES=-DMODULE_EXTENSION=\"${DEX}\" -DHOME_VAR=\"${HOME_VAR}\"

ifndef (${OBJ})
	OBJ=.obj/${arch}
endif


archtarget=${arch}

targets=instructions emux51 module hex settings ${archtarget} gui alarm
objects=${OBJ}/instructions.o ${OBJ}/emux51.o ${OBJ}/${archtarget}.o ${OBJ}/module.o \
	${OBJ}/hex.o ${OBJ}/gui.o ${OBJ}/settings.o ${OBJ}/alarm.o

widgets=port_selector 7seg led
widgeto=${OBJ}/port_selector.o ${OBJ}/7seg.o ${OBJ}/led.o


.PHONY: clean
.PHONY: build_all
.PHONY: build


build_all: directory widgets build modules

directory:
	mkdir -p ${OBJ}/modules


build:	${targets} gui alarm
	@ echo 'linking..'
	@ ${CC} ${BUILD} -L${OUTDIR} ${objects} ${LDFLAGS}  -o ${OUT}

${targets}:
	@ echo '${CC} src/$@.c'
	@ ${CC} ${INCLUDE} ${CFLAGS} ${DEFINES} -o ${OBJ}/$@.o src/emux51/$@.c

mods:
	@ make --makefile=modules/Makefile ARCH=${ARCH}

widgets: ${widgets}
	@ echo 'linking widgets..'
	@ ${CC} -shared -o ${OUTDIR}/libemux_widgets${DEX} ${widgeto} `${PKG-CONFIG} --libs gtk+-2.0`

${widgets}:
	@ echo ${CC} src/widgets/$@.c
	@${CC} ${INCLUDE} ${PIC} ${CFLAGS} -o ${OBJ}/$@.o src/widgets/$@.c


modules=3x7seg.mod 7seg.mod led.mod switch.mod 4x7seg.mod 8x7seg.mod keyboard.mod

modules: ${modules}
${modules}:
	@ echo '${CC} src/modules/${@:.mod=.c}'
	@ ${CC} -DBUILDING_MODULE ${INCLUDE} ${PIC} ${CFLAGS} -o ${OBJ}/modules/${@:.mod=.o}\
		src/modules/${@:.mod=.c}
	@ echo 'linking ${OBJ}/modules/${@:.mod=.o}'
	@ ${CC} -shared -L${OUTDIR} -lemux_widgets -o ${OUTDIR}/modules/${@:.mod=${DEX}}\
		${OBJ}/modules/${@:.mod=.o} `${PKG-CONFIG} --libs gtk+-2.0`
lines:
	@ cat src/emux51/*.c src/modules/*.c src/widgets/*.c include/*.h Makefile | wc -l
clean:
	rm -f ${OBJ}/*.o
	rm -f ${OBJ}/modules/*.o
	rm -f ${OUT}
	rm -f ${OUTDIR}/libemux_widgets${DEX}
	rm -f ${OUTDIR}/modules/*${DEX}

