;
;       Title                  : Wimp Open Window
;       System                 : Wimp Library
;       Version                : 1.0
;       Copyright              : (C) Peter Naulls
;       Date                   : 20th November, 2002
;       Author                 : John H. Winters
;
;       Function               : Opens a window in a parent
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
        STARTCODE Wimp_OpenWindowNest
;
        MOV     ip, lr

        MOV     a2, a1
        MOV     v1, a3
        LDR     a3, |task|
        SWI     SWI_Wimp_OpenWindow + XOS_Bit
        MOVVC   a1, #0

        MOV     a2, #0  ; Prevent R2 retaining value
        MOV     pc, ip

|task|
        DCB    "TASK"
;
        END
