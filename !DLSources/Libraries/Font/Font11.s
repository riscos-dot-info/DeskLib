@ Author Copyright 1993 Shaun Blackmore

        .include     "RegDefs.h"
        .include     "SwiNos.h"
        .include     "Macros.h"

@os_error *Font_ListFonts(char *name, int *count);

        
        .global Font_ListFonts
Font_ListFonts:
@
        STMFD   sp!, {r4,lr}
        MOV     r4,r1
        MOV     r3,#-1
        LDR     r2,[r4,#0]
        MOV     r1,r0
        SWI     SWI_Font_ListFonts + XOS_Bit
        STR     r2,[r4,#0]
        MOVVC   r0,#0
        LDMFD   sp!, {r4,pc}
@
