;
;       Title                  : Wimp Get Window Info (RISC OS 3).
;       System                 : Wimp Library
;       Version                : 1.0
;       Copyright              : (C) Sergio Monesi
;       Date                   : 26 Jan 1995
;       Author                 : Sergio Monesi
;
;       Function               : Get information about a window.
;                                Doesn't return icons (RISC OS 3 only)
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
        GET     ^.h.regdefs
        GET     ^.h.swinos
        GET     ^.h.macros
;
;============================================================================
;
;  Code.
;
;============================================================================
;
        PREAMBLE
        STARTCODE Wimp_GetWindowInfoNoIcons
;
        MOV     ip, lr
        ORR     a2, a1, #1        ; set bit 0 of R1
        SWI     SWI_Wimp_GetWindowInfo + XOS_Bit
        MOVVC   a1, #0
        MOVS    pc, ip
;
        END
