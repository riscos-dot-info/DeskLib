@
@       Title                  : Wimp Read Pix Trans
@       System                 : Wimp Library
@       Version                : 1.0
@       Copyright              : (C) John Winters
@       Date                   : 12th January, 1990
@       Author                 : John H. Winters
@
@       Function               : Reads a pixel translation ?
@
@
@       Modification history.
@
@       Version                : (Reflect in header IDENT)
@       Date                   :
@       Author                 :
@       Changes                :
@
@
@============================================================================
@
@  Include files.
@
@============================================================================
@
        .include     "RegDefs.h"
        .include     "SwiNos.h"
        .include     "Macros.h"
@
@============================================================================
@
@  Code.
@
@============================================================================
@
        
        .global Wimp_ReadPixTrans
Wimp_ReadPixTrans:
@
        MOV     ip, sp
        STMFD   sp!, {v1-v4, lr}
        LDR     v4, [ip, #0]
        MOV     v3, a4
        SWI     SWI_Wimp_ReadPixTrans + XOS_Bit
        MOVVC   a1, #0
        LDMFD   sp!, {v1-v4, pc}
@
