#
# This makefile template was created by Jerzy Witkowski
#
# 2004-01-21
#

all:	Toto.prc

Toto.ro:	Toto.rcp
	rm -f Toto.ro
	pilrc -ro Toto.rcp

Toto.o:	Toto.c Toto.h
	rm -f *.grc
	m68k-palmos-gcc -fno-builtin  -I/usr/src/v4p  -o Toto.o -I. -I/m68k-palmos/include -c Toto.c

Toto.prc:	Toto Toto.ro
	build-prc -n 'Toto' -c 'TOTO' -o Toto.prc Toto.ro Toto

Toto:	Toto.o  /usr/src/v4p/v4p.o /usr/src/v4p/v4pi.o /usr/src/v4p/lowmath.o
	m68k-palmos-gcc -o Toto Toto.o  /usr/src/v4p/v4p.o /usr/src/v4p/v4pi.o /usr/src/v4p/lowmath.o  -L/m68k-palmos/lib

clean:
	rm -f Toto.ro Toto Toto.o

custom1:
	make -C /usr/src/v4p

custom2:
	echo Add your own commands by editing the file makefile.ppc

custom3:
	echo Add your own commands by editing the file makefile.ppc
