;-------------------------------------------------------------------------------
; GFX_Wait (void)
; by Jason Williams 1993
;
        GET     Macros.h
        GET     RegDefs.h
        GET     swinos.h
;
        PREAMBLE
        STARTCODE GFX_Wait
;
        STMFD   sp!,{lr}
        MOV     a1, #19
        SWI     XSWI_OS_Byte
        LDMFD   sp!,{pc}
;
        END
