@
@       Title                  : Wimp Add Messages.
@       System                 : Wimp Library
@       Version                : 1.0
@       Copyright              : (C) Antony Sidwell
@       Date                   : 28th December, 2002
@       Author                 : Antony Sidwell
@
@       Function               : Extends the set of user messages that the
@                                Wimp sends to the task.
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
        
        .global Wimp_AddMessages
Wimp_AddMessages:
@
        MOV     ip, lr

        SWI     SWI_Wimp_AddMessages + XOS_Bit
        MOVVC   a1, #0

        MOV     pc, ip
@
