
LIB1=	libfbui.a
SRC1=	libfbui.c libfbuifont.c libfbuidialog.c
OBJ1=	libfbui.o libfbuifont.o libfbuidialog.o

LIB2=	libfbuiimage.a
SRC2=	libfbuiimage.c 
OBJ2=	libfbuiimage.o

CLOCK=	Clock/fbclock
LOAD=	LoadMonitor/fbload
CALC=	Calc/fbcalc
TERM=	Term/fbterm
DIRECT=	Directory/fbdirectory
MPEG=	MPEG/mpeg2decode
SCRIBBLE=	Scribble/fbscribble
VIEWER=	Viewer/fbview
BIFF=	MailCheck/fbcheck
TODO=	ToDo/fbtodo
WM=	WindowManager/fbwm
PM=	PanelManager/fbpm
LAUNCH=	Launcher/fblaunch
TEST1=	Test1/fbtest
TEST2=	Test2/fbtest
TEST3=	Test3/fbtest
TEST4A=	Test4a/fbtest
TEST4B=	Test4b/fbtest
TEST5=	Test5/fbtest
TEST6=	Test6/fbtest
TEST7=	Test7/fbtest
TEST8=	Test8/fbtest
BENCH=	Benchmark/fbmark

TARGETS=	${CLOCK} ${LOAD} ${CALC} ${TERM} ${MPEG} ${SCRIBBLE} ${VIEWER} ${BIFF} ${TEST1} ${TEST2} ${TEST3} ${TEST4A} ${TEST4B} ${TEST5} ${TEST6} ${TEST7} ${TEST8} ${WM} ${PM} ${TODO} ${LAUNCH} ${DIRECT} ${BENCH}

all:	${LIB1} ${LIB2} ${TARGETS}

CFLAGS= -g -I. -Wall

${LIB1}: ${OBJ1}
	ar rs ${LIB1} ${OBJ1} 

${LIB2}: ${OBJ2}
	ar rs ${LIB2} ${OBJ2} 

${CLOCK}:
	( cd Clock ; make clean ; make )

${LOAD}:
	( cd LoadMonitor ; make clean ; make )

${TERM}:
	( cd Term ; make clean ; make )

${MPEG}:
	( cd MPEG ; make clean ; make )

${SCRIBBLE}:
	( cd Scribble; make clean ; make )

${BENCH}:
	( cd Benchmark; make clean ; make )

${VIEWER}:
	( cd Viewer; make clean ; make )

${BIFF}:
	( cd MailCheck; make clean ; make )

${CALC}:
	( cd Calc; make clean ; make )

${TEST1}:
	( cd Test1; make clean ; make )

${TEST2}:
	( cd Test2; make clean ; make )

${TEST3}:
	( cd Test3; make clean ; make )

${TEST4A}:
	( cd Test4a; make clean ; make )

${TEST4B}:
	( cd Test4b; make clean ; make )

${TEST5}:
	( cd Test5; make clean ; make )

${TEST6}:
	( cd Test6; make clean ; make )

${TEST7}:
	( cd Test7; make clean ; make )

${TEST8}:
	( cd Test8; make clean ; make )

${WM}:
	( cd WindowManager; make clean ; make )

${PM}:
	( cd PanelManager; make clean ; make )

${LAUNCH}:
	( cd Launcher; make clean ; make )

${TODO}:
	( cd ToDo; make clean ; make )

${DIRECT}:
	( cd Directory; make clean ; make )

clean:
	rm -f ${OBJ1} ${LIB1} ${OBJ2} ${LIB2} 
	( cd Clock ; make clean )
	( cd LoadMonitor ; make clean )
	( cd Term ; make clean )
	( cd MPEG ; make clean )
	( cd Scribble; make clean )
	( cd Viewer; make clean )
	( cd MailCheck; make clean )
	( cd Calc; make clean )
	( cd Test1; make clean )
	( cd Test2; make clean )
	( cd Test3; make clean )
	( cd Test4a; make clean )
	( cd Test4b; make clean )
	( cd Test5; make clean )
	( cd Test6; make clean )
	( cd Test7; make clean )
	( cd Test8; make clean )
	( cd WindowManager; make clean )
	( cd PanelManager; make clean )
	( cd ToDo; make clean )
	( cd Launcher; make clean )
	( cd Directory; make clean )
	( cd Benchmark; make clean )

# finit

