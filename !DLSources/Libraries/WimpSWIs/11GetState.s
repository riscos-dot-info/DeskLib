;
;       Title                  : Wimp Get Window State
;       System                 : Wimp Library
;       Version                : 1.0
;       Copyright              : (C) John Winters
;       Date                   : 12th January, 1990
;       Author                 : John H. Winters
;
;       Function               : Gets the state of a window.
;
;
;       Modification history.
;
;       Version                : (Reflect in header IDENT)
;       Date                   :
;       Author                 :
;       Changes                :
;
;
;============================================================================
;
;  Include files.
;
;============================================================================
;
        GET     RegDefs.h
        GET     SwiNos.h
        GET     Macros.h
;
;============================================================================
;
;  Code.
;
;============================================================================
;
        PREAMBLE
        STARTCODE Wimp_GetWindowState
;
;        STMFD   sp!, {lr}
        MOV     ip, lr

        STR     a1, [a2, #0]
        SWI     SWI_Wimp_GetWindowState + XOS_Bit
        MOVVC   a1, #0

        MOVS    pc, ip
;        LDMFD   sp!, {pc}^
;
        END
