;-------------------------------------------------------------------------------
; OS_WriteC - VDU(ch)
;
        GET     Macros.h
        GET     RegDefs.h
        GET     SwiNos.h
;
        PREAMBLE
        STARTCODE GFX_VDU
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_WriteC
        LDMFD   sp!,{pc}
;
        END
