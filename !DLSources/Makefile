# GNU Make makefile for DeskLib

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

libraries := $(notdir $(filter-out Libraries/CVS, $(wildcard Libraries/*)))

lib_objects = $(addprefix Libraries.,$(addsuffix .o.*, $(libraries)))

all_objects = $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard Libraries/*/*))))

# Make the library
all: $(DESKLIB)

# Link the objects together to make a new library
$(DESKLIB): $(all_objects)
	$(AR) -c $@ $(AFLAFS) $(lib_objects)

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
              
.c.o:
	@mkdir -p $(@D)/o
	$(CC) $(CFLAGS) -c $< -o $@

.s.o:
	@mkdir -p $(@D)/o
	$(AS) $(ASFLAGS) -c $< -o $@

