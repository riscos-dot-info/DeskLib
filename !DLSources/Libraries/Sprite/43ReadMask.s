;
;       Title                  : Read mask pixel
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) Ainsley Pereira
;       Date                   : Sun 27th February 94
;       Author                 : Ainsley M. Pereira
;
;       Function               : Read pixel from a mask
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
        STARTCODE Sprite_ReadMaskPixel
;
        STMFD     sp!, {v1-v6, lr}
        LDR       v1, [a3, #4]
        LDR       a4, [a3, #0]
        MOV       a3, a2
        MOV       a2, a1
        MOV       a1, #256
        ADD       a1, a1, #43
        SWI       OS_SpriteOp + XOS_Bit
        MOV       a1, v2
        LDMFD     sp!, {v1-v6, pc}
;
        END
