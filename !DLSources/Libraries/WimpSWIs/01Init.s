@
@       Title                  : Wimp Task Init.
@       System                 : Wimp Library
@       Version                : 1.1
@       Copyright              : (C) John Winters
@       Date                   : 12th January, 1990
@       Author                 : John H. Winters
@
@       Function               : Initialises the Wimp to multi-task.
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
        
        .global Wimp_Initialise
Wimp_Initialise:
@
        STMFD   sp!, {v1, v2, lr}

        MOV     v1, a3                          @ R4 is ptr to taskhandle int
        MOV     a3, a2                          @ R2 is ptr to taskname
        MOV     v2, a1                          @ R5 is ptr to version int
        LDR     a1, [v2, #0]                    @ R0 is version int
        LDR     a2, DATA
        @ Added by PColmer
        @ optional parameter for RISC OS 3, R3 is ptr to list of msgs (reg a4)
        SWI     SWI_Wimp_Initialise + XOS_Bit
        STRVC   a1, [v2, #0]                    @ return found version
        STRVC   a2, [v1, #0]                    @ return task handle
        MOVVC   a1, #0
        LDMFD   sp!, {v1, v2, pc}
DATA:
        .ascii     "TASK"
@
