@-------------------------------------------------------------------------------
@ OS_WriteN
@
        .include     "Macros.h"
        .include     "RegDefs.h"
        .include     "SwiNos.h"
@
        
        .globl GFX_WriteN
@
        SWI     XSWI_OS_WriteN
        MOV     pc, lr
@
