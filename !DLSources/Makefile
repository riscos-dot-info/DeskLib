# GNU Make makefile for DeskLib

AR = libfile

CC = cc
CFLAGS = -ffahi -throwback -IDeskLib:
AS = objasm
ASFLAGS = -I$(@D)/s

WARNINGS = -ansi -pedantic -Wall -Wundef -Wpointer-arith \
           -Wcast-align -Wwrite-strings -Wstrict-prototypes \
           -Wmissing-prototypes -Wmissing-declarations -Wnested-externs \
           -Winline -Wno-unused -Wno-long-long -W -Wcast-qual -Wshadow

#CC = gcc -mlibscl
#CFLAGS = -I. -IDeskLib: -mthrowback $(WARNINGS) -O2
#AS = gcc

DESKLIB = <DeskLib$$Dir>.o.DeskLib
DESKLIBDIR = <DeskLib$$Dir>.o
DEBUGS = <DeskLib$$Dir>.o.Debug

libraries := $(notdir $(filter-out Libraries/CVS, $(wildcard Libraries/*)))

lib_objects = $(addprefix Libraries.,$(addsuffix .o.*, $(libraries)))

all_objects = $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard Libraries/*/*))))

# Make the library
all: $(DESKLIB) debugs smerror

# Link the objects together to make a new library
$(DESKLIB): $(all_objects)
	@mkdir $(subst .,/,$@)/..
	$(AR) -c $@ $(AFLAFS) $(lib_objects)

# Insert all the Makefiles in the Otherlibs directory as well.
# This provides the targets debugs and smerror                  
include OtherLibs/*/Makefile

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
	$(AS) $(ASFLAGS) -c $< -o $@

