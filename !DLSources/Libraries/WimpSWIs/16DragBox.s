;
;       Title                  : Wimp Drag Box.
;       System                 : Wimp Library
;       Version                : 1.0
;       Copyright              : (C) John Winters
;       Date                   : 12th January, 1990
;       Author                 : John H. Winters
;
;       Function               : Initiates a dragging operation.
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
        STARTCODE Wimp_DragBox
;
;        STMFD   sp!, {lr}
        MOV     ip, lr

        MOV     a2, a1
        SWI     SWI_Wimp_DragBox + XOS_Bit
        MOVVC   a1, #0

        MOVS    pc, ip
;        LDMFD   sp!, {pc}^
;
        END
