# Project: DeskLib (DLUser)
# Make: make
# Wimpslot: 5000k
#
# GNU Make makefile for DLUser.  Defaults to using Norcroft as the compiler.
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

# Defaults for norcy
ifeq (norcroft, $(COMPILER))

  CC = cc
  CFLAGS = -ffahi -throwback

  AS = objasm
  ASFLAGS = -ILibraries -I$(@D) -throwback

  AR = libfile    
  ARFLAGS = -o
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
endif

DLUSER = <DeskLib_User$$Dir>.o.DLUser
DLUSERDIR = <DeskLib_User$$Dir>.o

libraries := $(notdir $(filter-out Libraries/CVS, $(wildcard Libraries/*)))

lib_objects = $(addprefix Libraries.,$(addsuffix .o.*, $(libraries)))

all_objects = $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard Libraries/*/*))))

# Make the library
all: $(DLUSER)

# Link the objects together to make a new library
$(DLUSER): $(all_objects)
	@mkdir -p $(basename $@)
	$(AR) $(ARFLAGS) -c $@ $(lib_objects)


# Clean the whole library out in one go
clean:
	-@$(cleanlibs)
	Remove $(DLUSER)

# To empty the 'o' directories for each library
# This a hack, we'd be much better off with a "rm" command that could take
# a list of things to delete.  

define cleanlib                     
$(shell Echo Wipe Libraries.$(library).o.* ~C~VF~R)
$(shell X Wipe Libraries.$(library).o.* ~C~VF~R)
endef

cleanlibs = $(foreach library, $(libraries), $(cleanlib))

.c.o:
	@mkdir -p $(@D)/o
	$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	@mkdir -p $(@D)/o
	$(AS) $(ASFLAGS) $< -o $@

