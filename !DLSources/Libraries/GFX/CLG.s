;-------------------------------------------------------------------------------
; GFX_CLG(void)
; by Jason Williams 1993
;
        GET     ^.h.Macros
        GET     ^.h.RegDefs
        GET     ^.h.SWINos
;
        PREAMBLE
        STARTCODE GFX_CLG
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_WriteI + 16
        LDMFD   sp!,{pc}^
;
        END
