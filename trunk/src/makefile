MODE ?= normal
TARGET ?= linux

CC_palmos=m68k-palmos-gcc
AR_palmos=m68k-palmos-ar
CCFLAGS_palmos= -g -fno_builtin -I/m68k_palmos/include
ARFLAGS_palmos= rcs

CC_linux=gcc
AR_linux=ar
ARFLAGS_linux= rvs
CCFLAGS_linux=
CCFLAGS_linux_debug= -g

CC=$(CC_$(TARGET))
AR=$(AR_$(TARGET))
CPPFLAGS=$(CCFLAGS_$(TARGET)) $(CCFLAGS_$(TARGET)_$(MODE)) -I. -I$(TARGET)
ARFLAGS=$(ARFLAGS_$(TARGET)) $(ARFLAGS_$(TARGET)_$(MODE))

all: libv4p.a libgm.a

install: libv4p.a libgm.a
	-mkdir -p /usr/include/v4p/
	cp libv4p.a /usr/lib/
	cp libgm.a /usr/lib/
	cp *.h /usr/include/v4p/

clean:
	-rm *.a *.o

libv4p.a: quickheap.o sortable.o lowmath.o v4p.o v4pi.o
	$(AR) $(ARFLAGS) $@ $?

libgm.a: gamemachine.o
	$(AR) $(ARFLAGS) $@ $?

VPATH=.:$(TARGET)
