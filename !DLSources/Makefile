# Project: DeskLib
# Make: make
# Wimpslot: 5000k
#
# GNU Make makefile for DeskLib.  Defaults to using Norcroft as the compiler.
#
# Various things passed on the command line can alter this makefile's behaviour;
# these are:
#
#   COMPILER - either "gcc" or "norcroft", depends on what you want to use
#   PREFIX   - this is only used when COMPILER is gcc; it gives a prefix for all
#              gcc commands executed.  Default is "gccbin:".
#

# Defaults
COMPILER = norcroft
PREFIX = gccbin:

DESKLIB = <DeskLib$$Dir>.o.DeskLib
DESKLIBDIR = <DeskLib$$Dir>.o
DEBUGS = <DeskLib$$Dir>.o.Debug

libraries := $(notdir $(filter-out Libraries/CVS Libraries/h \
               Libraries/Macros.h Libraries/RegDefs.h, $(wildcard Libraries/*)))

all_objects := $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard Libraries/*/*))))


# Defaults for norcy
ifeq (norcroft, $(COMPILER))

  CC = cc
  CFLAGS = -ffahi -throwback

  AS = objasm
  ASFLAGS = -ILibraries -I$(@D) -throwback

  AR = libfile
  ARFLAGS =

  lib_objects := $(addprefix Libraries.,$(addsuffix .o.*, $(libraries)))
endif

# Defaults for gcc
ifeq (gcc, $(COMPILER))

  WARNINGS = -ansi -pedantic -Wall -Wundef -Wpointer-arith \
             -Wcast-align -Wwrite-strings -Wstrict-prototypes \
             -Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
             -Winline -Wno-unused -Wno-long-long -W -Wcast-qual -Wshadow

  CC = $(PREFIX)gcc
  CFLAGS = -mlibscl -mthrowback -O2 $(WARNINGS)

  AS = $(PREFIX)gcc
  ASFLAGS = -Wa,-ILibraries,-I$(@D) -mthrowback -c

  AR = $(PREFIX)libfile
  ARFLAGS =

  lib_objects := $(addprefix Libraries/,$(addsuffix /*.o, $(libraries)))
endif

# Make the library
all: $(DESKLIB) debugs smerror

# Link the objects together to make a new library
$(DESKLIB): $(all_objects)
	@mkdir -p $(basename $@)
	$(AR) $(ARFLAGS) -c $@ $(lib_objects)

# Insert all the Makefiles in the Otherlibs directory as well.
# This provides the targets debugs and smerror                  

include OtherLibs/*/Makefile*

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
	dir ^.^
	zip -9r RAM::RamDisc0.$$.sources/zip DeskLib.!DLSources -x *.CVS.* -x *.o.*
	zip -9r RAM::RamDisc0.$$.core/zip DeskLib.!DeskLib DeskLib.!DLUser DeskLib.!Help DeskLib.Docs  -x *.CVS.* -x *cvsignore
	zip -9r RAM::RamDisc0.$$.examples/zip DeskLib.Examples -x *.CVS.* 

.c.o:
	@mkdir -p $(@D)/o
	$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	@mkdir -p $(@D)/o
	$(AS) $(ASFLAGS) $< -o $@

