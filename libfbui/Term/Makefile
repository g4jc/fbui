
SRC=ansi.c fbterm.c main.c forkpty.c iswprint.c 
EXE=fbterm


fbterm:	${SRC}
	gcc -DHAVE_FORKPTY -I.. -g ${SRC} ../libfbui.a ../libfbuiimage.a -o ${EXE} -lutil -ljpeg -ltiff
	strip fbterm

install: ${EXE}
	cp ${EXE} /usr/bin
	chmod 755 /usr/bin/${EXE}

clean:
	rm -f ${EXE}
