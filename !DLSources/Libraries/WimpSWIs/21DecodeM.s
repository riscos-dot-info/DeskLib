;
;       Title                  : Wimp Decode Menu
;       System                 : Wimp Library
;       Version                : 1.0
;       Copyright              : (C) John Winters
;       Date                   : 12th January, 1990
;       Author                 : John H. Winters
;
;       Function               : Decodes a menu result.
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
        GET     regdefs.h
        GET     swinos.h
        GET     macros.h
;
;============================================================================
;
;  Code.
;
;============================================================================
;
        PREAMBLE
        STARTCODE Wimp_DecodeMenu
;
;        STMFD   sp!, {lr}
        MOV      ip, lr

        MOV     a4, a3
        MOV     a3, a2
        MOV     a2, a1
        SWI     SWI_Wimp_DecodeMenu + XOS_Bit
        MOVVC   a1, #0

        MOV     pc, ip
;        LDMFD   sp!, {pc}
;
        END
