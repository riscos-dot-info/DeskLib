;-------------------------------------------------------------------------------
; GFX_Plot (int plotcode, int x, int y)
; by Jason Williams 1993
;
        GET     ^.h.Macros
        GET     ^.h.RegDefs
        GET     ^.h.SWINos
;
        PREAMBLE
        STARTCODE GFX_Plot
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_Plot
        LDMFD   sp!,{pc}
;
        END
