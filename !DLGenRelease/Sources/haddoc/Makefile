# Project: haddoc
# GNU Make makefile for haddoc
# Wimpslot: 5000k
# make: make

# Set the default build environment
default: riscos

# Set the default output executable
TARGET = ../../haddoc

# Warning options
WARNINGS = \
		-Wall -Wundef -Wpointer-arith -Wcast-align -Wwrite-strings \
		-Wstrict-prototypes -Wnested-externs -Winline -Wno-unused \
		-W -Wcast-qual -Wshadow

# Builds
ALL_BUILDS = riscos unix ro_timings

# To get timings on RISC OS 
ro_timings: CC = gcc
ro_timings: CFLAGS = -DRISCOS  -DRISCOS_TIMINGS -O2 -IPCRE: -IDesklib: -mpoke-function-name -mthrowback $(WARNINGS)
ro_timings: LIBS = PCRE:libpcre.o Desklib:o.Desklib

# RISC OS GCC options
riscos: CC = gcc
riscos: CFLAGS = -DRISCOS -O2 -IPCRE: -mpoke-function-name -mthrowback $(WARNINGS)
riscos: LIBS = PCRE:libpcre.o

# Unix GCC options
unix: CC = gcc
unix: CFLAGS = -g $(WARNINGS)
unix: LIBS = -lpcre

# All builds depend on the target executable
$(ALL_BUILDS): $(TARGET)

# Nice way to get all (object/source) files
OBJS = $(patsubst %.c,%.o,$(filter %.c,$(wildcard *)))
SRCS = $(filter %.c,$(wildcard *))

# Target executable
$(TARGET): $(OBJS)
	gcc -o $(TARGET) $(OBJS) $(LIBS)

# Rules
.c.o: $(CC) -O2 $(CFLAGS) -c $< -o $@
.s.o: $(AS) $(ASFLAGS) -c $< -o $@

# Dependencies
