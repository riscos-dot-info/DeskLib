# Project:   Empty_Make_File

#
#  To use this makefile, simply fill in the 2 parts marked 'XXXX'.
#  'Files' lists the object files you wish to compile.
#  'XXXXLib' is the filename of your sublibrary output file.
#

.SUFFIXES:   .c .s .o
CC           = cc
CCflags      = -c -IDeskLib:,C: $(x_cflags)
ObjAsmflags  = -Stamp -NoCache -CloseExec -Quit $(x_aflags)

Files        = o.XXXX o.XXXX


XXXXLib:     $(Files)
             $(do)

.c.o:;       $(CC) $(CCflags) $< -o $@
.s.o:;       objasm $(ObjAsmflags) -from $< -to $@

