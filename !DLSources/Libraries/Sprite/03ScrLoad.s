;
;       Title                  : Sprite 03
;       System                 : Sprite Library
;       Version                : 1.0
;       Copyright              : (C) John Winters
;       Date                   : 26th January, 1992
;       Author                 : John H. Winters
;
;       Function               : Issues an OS_SpriteOp 3.
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
        STARTCODE Sprite_ScreenLoad
;
        STMFD   sp!, {lr}
        MOV     a3, a1
        MOV     a1, #3
        SWI     OS_SpriteOp + XOS_Bit
        MOVVC   a1, #0
        LDMFD   sp!, {pc}^
;
        END
