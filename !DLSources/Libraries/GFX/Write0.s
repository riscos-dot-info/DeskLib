@-------------------------------------------------------------------------------
@ OS_Write0
@
        .include     "Macros.h"
        .include     "RegDefs.h"
        .include     "SwiNos.h"
@
        
        .globl GFX_Write0
@
        SWI     XSWI_OS_Write0
        MOV     pc, lr
@
