;-------------------------------------------------------------------------------
; OS_WriteC - VDU(ch)
;
        GET     ^.h.Macros
        GET     ^.h.RegDefs
        GET     ^.h.SWINos
;
        PREAMBLE
        STARTCODE GFX_VDU
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_WriteC
        LDMFD   sp!,{pc}
;
        END
