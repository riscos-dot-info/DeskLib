;
;       Title                  : Create palette
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) Ainsley Pereira
;       Date                   : Sun 27th February 94
;       Author                 : Ainsley M. Pereira
;
;       Function               : Creates a palette for a sprite
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
        STARTCODE Sprite_CreatePalette
;
        STMFD     sp!, {a1-a2, lr}
        CMP       a3,#0
        MOVEQ     a4,#0
        MOVNE     a4,#&80000000
        LDMFD     sp!, {a2-a3}
        MOV       a1, #256
        ADD	  a1, a1, #37
        SWI       OS_SpriteOp + XOS_Bit
        MOV       a1, a4
        MOVVS     a1, #0
        LDMFD     sp!, {pc}
;
	END
