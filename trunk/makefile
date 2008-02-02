all: libv4p.a

clean:
	-rm *.a *.o

libv4p.a: lowmath.o v4p.o v4pi.o
	m68k-palmos-ar rcs libv4p.a lowmath.o v4p.o v4pi.o

v4p.o: v4p.c v4p.h
	m68k-palmos-gcc -fno-builtin -o v4p.o -I/m68k-palmos/include -I. -c v4p.c

v4pi.o: v4pi.c v4pi.h
	m68k-palmos-gcc -fno-builtin -o v4pi.o -I/m68k-palmos/include -I. -c v4pi.c

lowmath.o: lowmath.c lowmath.h
	m68k-palmos-gcc -fno-builtin -o lowmath.o -I/m68k-palmos/include -I. -c lowmath.c
