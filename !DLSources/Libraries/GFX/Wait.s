;-------------------------------------------------------------------------------
; GFX_Wait (void)
; by Jason Williams 1993
;
        GET     ^.h.Macros
        GET     ^.h.RegDefs
        GET     ^.h.SWINos
;
        PREAMBLE
        STARTCODE GFX_Wait
;
        STMFD   sp!,{lr}
        MOV     a1, #19
        SWI     XSWI_OS_Byte
        LDMFD   sp!,{pc}^
;
        END
