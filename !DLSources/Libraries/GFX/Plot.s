;-------------------------------------------------------------------------------
; GFX_Plot (int plotcode, int x, int y)
; by Jason Williams 1993
;
        GET     Macros.h
        GET     RegDefs.h
        GET     SwiNos.h
;
        PREAMBLE
        STARTCODE GFX_Plot
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_Plot
        LDMFD   sp!,{pc}
;
        END
