#
# This makefile template was created by Jerzy Witkowski
#
# 2004-01-21
#

all:	Totoe.prc

Totoe.ro:	Totoe.rcp
	rm -f Totoe.ro
	pilrc -ro Totoe.rcp

Totoe.o:	Totoe.c Totoe.h
	rm -f *.grc
	m68k-palmos-gcc -fno-builtin  -I/usr/src/v4p  -o Totoe.o -I. -I/m68k-palmos/include -c Totoe.c

Totoe.prc:	Totoe Totoe.ro
	build-prc -n 'Totoe' -c 'TOTE' -o Totoe.prc Totoe.ro Totoe

Totoe:	Totoe.o  /usr/src/v4p/v4p.o /usr/src/v4p/v4pi.o /usr/src/v4p/lowmath.o
	m68k-palmos-gcc -o Totoe Totoe.o  /usr/src/v4p/v4p.o /usr/src/v4p/v4pi.o /usr/src/v4p/lowmath.o  -L/m68k-palmos/lib

clean:
	rm -f Totoe.ro Totoe Totoe.o

custom1:
	make -C /usr/src/v4p

custom2:
	echo Add your own commands by editing the file makefile.ppc

custom3:
	echo Add your own commands by editing the file makefile.ppc
