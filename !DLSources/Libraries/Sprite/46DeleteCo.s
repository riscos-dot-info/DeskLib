;
;       Title                  : Delete Column
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) Ainsley Pereira
;       Date                   : Sun 27th February 94
;       Author                 : Ainsley M. Pereira
;
;       Function               : Delete column of pixels
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
        STARTCODE Sprite_DeleteColumn
;
        STMFD     sp!, {a1-a3, v1-v6, lr}
        LDMFD     sp!, {a2-a4}
        MOV       a1, #256
        ADD	  a1, a1, #46
        SWI       OS_SpriteOp + XOS_Bit
        MOVVC     a1,#0
        LDMFD     sp!, {v1-v6, pc}^
;
        END
