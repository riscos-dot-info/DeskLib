# Project: haddoctosh
# GNU Make makefile for haddoctosh
# Wimpslot: 5000k
# make: make

# Set the default build environment
default: riscos

# Set the default output executable
TARGET = ../../haddoctosh

# Warning options
WARNINGS = \
		-Wall -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings \
		-Wstrict-prototypes -Wnested-externs -Winline -Wno-unused \
		-W -Wcast-qual -Wshadow

# Builds
ALL_BUILDS = riscos unix

# RISC OS GCC options
riscos: CC = gcc
riscos: CFLAGS =  -DRISCOS -Ilibxml: -mpoke-function-name -mthrowback $(WARNINGS)
riscos: LIBS = libxml:o.libxml zlib:o.zlib
riscos: LINKFLAGS =

# Unix GCC options
unix: CC = gcc
unix: CFLAGS = -I/usr/include/libxml2/ -g $(WARNINGS)
unix: LIBS = -lxml2

# All builds depend on the target executable
$(ALL_BUILDS): $(TARGET)

# Nice way to get all (object/source) files
OBJS = $(patsubst %.c,%.o,$(filter %.c,$(wildcard *)))
SRCS = $(filter %.c,$(wildcard *))

# Target executable
$(TARGET): $(OBJS)
	gcc  $(LINKFLAGS) -o$(TARGET) $(OBJS) $(LIBS)

# Rules
.c.o: $(CC) $(CFLAGS) -c $< -o $@
.s.o: $(AS) $(ASFLAGS) -c $< -o $@

# Dependencies
