;-------------------------------------------------------------------------------
; OS_WriteN
;
        GET     ^.h.Macros
        GET     ^.h.RegDefs
        GET     ^.h.SWINos
;
        PREAMBLE
        STARTCODE GFX_WriteN
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_WriteN
        LDMFD   sp!,{pc}^
;
        END
