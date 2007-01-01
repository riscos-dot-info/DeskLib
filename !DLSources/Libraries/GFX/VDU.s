@-------------------------------------------------------------------------------
@ OS_WriteC - VDU(ch)
@
        .include     "Macros.h"
        .include     "RegDefs.h"
        .include     "SwiNos.h"
@
        
        .globl GFX_VDU
@
        SWI     XSWI_OS_WriteC
        MOV     pc, lr
@
