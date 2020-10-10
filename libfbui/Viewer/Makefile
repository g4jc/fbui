
EXE=fbview
SRC=	main.c 

${EXE}:	${SRC}
	gcc -Wall -lm -I.. -g ${SRC} ../libfbui.a ../libfbuiimage.a -o ${EXE} -ljpeg -ltiff
	# strip ${EXE}

clean:
	rm -f ${EXE}

install: ${EXE}
	cp ${EXE} /usr/bin
	chmod 755 /usr/bin/${EXE}

