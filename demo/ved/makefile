# Project big lines
RESULT = ved
C_SOURCES = editor.c
STATIC_LIBS = v4p gm
EXTERNAL_LIBS = SDL
TOP=../..


VPATH=$(TOP):.

# Default compile mode
TARGET ?= linux
MODE ?= normal

# Settings dispatching
CC_palmos=m68k-palmos-gcc
AR_palmos=m68k-palmos-ar
CCFLAGS_palmos= -g -fno_builtin -I/m68k_palmos/include
ARFLAGS_palmos= rcs

CC_linux=gcc
AR_linux=ar
ARFLAGS_linux= rsv
CCFLAGS_linux=
CCFLAGS_linux_debug= -g

CC=$(CC_$(TARGET))
AR=$(AR_$(TARGET))
CPPFLAGS=$(CCFLAGS_$(TARGET)) $(CCFLAGS_$(TARGET)_$(MODE)) -I. -I../../$(TARGET) -I../..
ARFLAGS=$(ARFLAGS_$(TARGET)) $(ARFLAGS_$(TARGET)_$(MODE))

OBJECTS = $(C_SOURCES:%.c=%.o) $(CPP_SOURCES:%.cpp=%.o)

all: $(RESULT)

clean:
	-rm *.a *.o $(RESULT)

$(RESULT): $(OBJECTS) $(STATIC_LIBS:%=lib%.a)
	gcc $(CPPFLAGS) -o $@ $(OBJECTS) $(EXTERNAL_LIBS:%=-l%) -L$(TOP) -L . $(STATIC_LIBS:%=-l%)

$(STATIC_LIBS:%=lib%.a): FORCE
	$(MAKE) -C $(TOP) $@

FORCE:

