;
;       Title                  : Read Control Block
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) Ainsley Pereira
;       Date                   : Sun 27th February 94
;       Author                 : Ainsley M. Pereira
;
;       Function               : Reads info about user sprite area
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
        STARTCODE Sprite_ReadControl
;
        STMFD     sp!, {a2-a4, v1-v6, lr}
        MOV       a2, a1
        MOV       a1, #256 + 8
        SWI       OS_SpriteOp + XOS_Bit
        MOV       a1, a4
        LDR       ip, [sp], #4
        CMP       ip, #0
        STRNE     a3, [ip]
        LDR       ip, [sp], #4
        CMP       ip, #0
        STRNE     v1, [ip]
        LDR       ip, [sp], #4
        CMP       ip, #0
        STRNE     v2, [ip]
        LDMFD     sp!, {v1-v6, pc}
;
	END
