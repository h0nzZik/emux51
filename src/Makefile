#	src/Makefile

.PHONY:	all
ARCH=posix

all:${target}

${target}:include/${target}.h
	echo $@
	${CC} -I include -c ${CFLAGS} -o obj/${target}.o src/${target}.c 2>>log
arch:
	${CC} -I include -c ${CFLAGS} -o obj/arch.o src/arch/${ARCH}.c 2>>log
