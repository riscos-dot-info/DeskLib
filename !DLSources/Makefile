#
# GNU make makefile for Desklib.  It simply finds all the .c and .s files in
# the various Libraries directories and calls the standard rules to compile
# them, and then whacks the whole lot together with libfile.
#
# Headers for IMTIA
# -----------------
# Project:   DeskLib
# Make:		 make
# Wimpslot:  15000k

# Choice of compilers
CC = cc -c -ffahi -throwback -IC:
# CC = gcc -c -mthrowback -mlibscl -IC:

AS = objasm

LIB = libfile -c -l $@

# Get all .c and .s files from the directories in "Libraries", and
# convert the endings to .o instead
all_objects = $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard Libraries/*/*))))

#
# For RISC OS use
#
# Gets all object directories in Libraries
library_dirs = $(subst /,.,$(filter-out Libraries/CVS, $(wildcard Libraries/*)))
# (adds .o.* to each directory for libfile to use)
libfile_objects = $(addsuffix .o.*, $(library_dirs))
# (creates o directories) 
discard = $(foreach dir, $(library_dirs), $(shell cdir $(dir).o))

#
# For GCC-SDK use (make filenames like filename.o) (?)
#                        
# libfile_objects = $(addsuffix *.o, $(filter-out Libraries/CVS, $(wildcard Libraries/*)))

DeskLib: $(all_objects)
	$(LIB) $(libfile_objects)

makedirs: $(discard)

# Static dependencies:
.SUFFIXES: .s .c .o

.c.o:
        $(CC) -o $@ $<
.s.o:
		$(AS) -from $< -to $@

