# Project: DeskLib
# Make: make
# Wimpslot: 5000k
#
# GNU Make makefile for DeskLib.  Defaults to using gcc as the compiler.
#
# Various things passed on the command line can alter this makefile's behaviour;
# these are:
#
#   COMPILER - either "gcc" or "norcroft", depends on what you want to use
#   PREFIX   - this is only used when COMPILER is gcc; it gives a prefix for all
#              gcc commands executed.  Default is "gccbin:".
#

# Defaults
COMPILER = gcc
PREFIX = gccbin:

DESKLIB = <DeskLib$$Dir>.o.DeskLib
DESKLIBDIR = <DeskLib$$Dir>.o

libraries := $(notdir $(filter-out Libraries/.svn Libraries/h \
               Libraries/Macros.h Libraries/RegDefs.h, $(wildcard Libraries/*)))

all_objects := $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard Libraries/*/*))))

lib_objects := $(addprefix Libraries.,$(addsuffix .o.*, $(libraries)))

# Defaults for norcy
ifeq (norcroft, $(COMPILER))
  CC = cc
  CFLAGS = -ffahi -throwback -IC:

  AS = objasm
  ASFLAGS = -ILibraries -I$(@D) -throwback

  AR = libfile
  ARFLAGS =
endif

# Defaults for gcc
ifeq (gcc, $(COMPILER))
  WARNINGS = -std=c99 -pedantic -Wall -Wundef -Wpointer-arith -Wuninitialized\
             -Wcast-align -Wwrite-strings -Wstrict-prototypes -Wunused\
             -Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
             -Winline -Wno-unused -Wno-long-long -W -Wcast-qual -Wshadow

  CC = $(PREFIX)gcc
  CFLAGS = -mthrowback -O3 $(WARNINGS) -mpoke-function-name -mlibscl

  AS = $(PREFIX)gcc
  ASFLAGS = -Wa,-ILibraries,-I$(@D) -mthrowback -mlibscl -c

  AR = $(PREFIX)libfile
  ARFLAGS =
endif

# Make the library
all: $(DESKLIB)

# Link the objects together to make a new library
$(DESKLIB): $(all_objects)
	@mkdir -p $(basename $@)
	$(AR) $(ARFLAGS) -c $@ $(lib_objects)

# Clean the whole library out in one go
clean:
	-@$(cleanlibs)
	Remove $(DESKLIB) 

# To empty the 'o' directories for each library
# This a hack, we'd be much better off with a "rm" command that could take
# a list of things to delete.  

define cleanlib                     
$(shell Echo Wipe Libraries.$(library).o.* ~C~VF~R)
$(shell X Wipe Libraries.$(library).o.* ~C~VF~R)
endef

cleanlibs = $(foreach library, $(libraries), $(cleanlib))


zip:
	make -C ../.. -f DeskLib/!DLSources/Makefile ziptargets

ziptargets:
	zip -9r RAM::RamDisc0.$$.sources/zip DeskLib.!DLSources -x *./svn.* -x *.o.*
	zip -9r RAM::RamDisc0.$$.core/zip DeskLib.!DeskLib DeskLib.!DLUser DeskLib.!Help DeskLib.Docs -x *./svn.*
	zip -9r RAM::RamDisc0.$$.examples/zip DeskLib.Examples -x *./svn.* 

.c.o:
	@mkdir -p $(@D)/o
	$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	@mkdir -p $(@D)/o
	$(AS) $(ASFLAGS) $< -o $@

