;
;       Title                  : Plot grey
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) Ainsley Pereira
;       Date                   : Sun 27th February 94
;       Author                 : Ainsley M. Pereira
;
;       Function               : Plots sprite greyscaled/antialiased
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
        STARTCODE Sprite_PlotGrey
;
        MOV       ip, sp
        STMFD     sp!, {v1-v6, lr}
        LDR       v4, [ip]
        MOV       v3, a4
        MOV       v2, #0
        LDR       v1, [a3, #4]
        LDR       a4, [a3, #0]
        MOV       a3, a2
        MOV       a2, a1
        MOV       a1, #256
        ADD       a1, a1, #53
        SWI       OS_SpriteOp + XOS_Bit
        MOVVC     a1, #0
        LDMFD     sp!, {v1-v6, pc}^
;
	END
