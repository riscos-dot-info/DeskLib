# GNU Make makefile for DeskLib
#
# Where appropriate, this calls make again to enter the correct subdirectory
# for the part of the library you are making, so that it can work with
# gcc which otherwise gets confused by the #include "DeskLib:xxx.h" lines.
#
# - To make the whole library again from scratch
#   make clean all
# - To make one part of the library: (eg. Url)
#   make library="Url" one
# - To make one part of the library from scratch:
#   make library="Url" cleanone one
# - To make a new library from the current object files:
#   make link

CC = cc -c -ffahi -throwback -IDeskLib: -IC: -o $@ $<
AS = objasm -from $< -to $@

# CC = gcc -c -ansi -IDeskLib: -mthrowback -o $@ $<
# CC = gcc -apcs32 -c -ansi -IDeskLib: -mthrowback -o $@ $<
# AS = gcc -Wa,-objasm,-throwback -c -o $@ $<

LIB = libfile -c -l DeskLib
MKDIR = CDir

# The location of this makefile
makefile = <DeskLib_Sources$$Dir>.Makefile

libraries := $(notdir $(filter-out Libraries/CVS, $(wildcard Libraries/*)))

lib_objects = $(addprefix Libraries.,$(addsuffix .o.*, $(libraries)))

# Make the library
all: makelibs link

# Set the

# Link the objects together to make a new library
link:
	$(LIB) $(lib_objects)
	-@Echo Made DeskLib

# Clean the whole library out in one go
clean:
	-@$(cleanlibs)
	-@X Wipe DeskLib ~CF~R~V
	-@Echo Wipe DeskLib ~CF~R~V

# Make a single sublibrary
one: $(addsuffix .library, $(library))

# Clean a single sublibrary
cleanone:; -X Wipe Libraries.$(library).o.* ~C~VF~R

# To empty the 'o' directories for each library
# This a hack, we'd be much better off with a "rm" command that could take
# a list of things to delete.  
define cleanlib                     
$(shell Echo Wipe Libraries.$(library).o.* ~C~VF~R)
$(shell X Wipe Libraries.$(library).o.* ~C~VF~R)
endef
cleanlibs = $(foreach library, $(libraries), $(cleanlib))
              
# Make all the object files for the current sublibrary
makelib:  $(patsubst %.s,%.o,$(patsubst %.c,%.o,$(filter %.c %.s,$(wildcard *))))

# We make makelibs depend on phony .library files to invoke make recursively for
# each sublibrary
makelibs: $(addsuffix .library, $(libraries))

%.library:
	$(MKDIR) Libraries.$*.o
	@$(MAKE) sublib=$* --directory Libraries.$* -f $(makefile) makelib

%.o: %.c ;$(CC)
	
%.o: %.s; $(AS)

