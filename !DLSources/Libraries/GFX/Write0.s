;-------------------------------------------------------------------------------
; OS_Write0
;
        GET     ^.h.Macros
        GET     ^.h.RegDefs
        GET     ^.h.SWINos
;
        PREAMBLE
        STARTCODE GFX_Write0
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_Write0
        LDMFD   sp!,{pc}^
;
        END
