;
;       Title                  : Sprite 16U
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) John Winters
;       Date                   : 9th February, 1992
;       Author                 : John H. Winters
;
;       Function               : Issues an OS_SpriteOp 16.
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
        STARTCODE Sprite_Get
;
        MOV     ip, sp
        STMFD   sp!, {a1, a2, a3, a4, v1, v2, v3, v4, v5, lr}
        LDMFD   sp!, {a2, a3, a4, v1}
        LDMFD   ip!, {v3, v4, v5}
        MOV     a1, #16
        ADD     a1, a1, #256
        SWI     OS_SpriteOp + XOS_Bit
        LDRVC   ip, [ip, #0]
        STRVC   a3, [ip, #0]
        MOVVC   a1, #0
        LDMFD   sp!, {v1, v2, v3, v4, v5, pc}
;
        END
