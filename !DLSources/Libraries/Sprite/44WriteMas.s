;
;       Title                  : Write mask pixel
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) Ainsley Pereira
;       Date                   : Sun 27th February 94
;       Author                 : Ainsley M. Pereira
;
;       Function               : Write pixel to mask
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
        GET     swinos.h
        GET     Macros.h
;
;============================================================================
;
;  Code.
;
;============================================================================
;
        PREAMBLE
        STARTCODE Sprite_WriteMaskPixel
;
        STMFD     sp!, {v1-v6, lr}
        CMP       a4, #0
        MOVEQ     v2, #0
        MOVNE     v2, #1
        LDR       v1, [a3, #4]
        LDR       a4, [a3, #0]
        MOV       a3, a2
        MOV       a2, a1
        MOV       a1, #256 + 44
        SWI       OS_SpriteOp + XOS_Bit
        MOVVC     a1, #0
        LDMFD     sp!, {v1-v6, pc}
;
        END
