;-------------------------------------------------------------------------------
; GFX_Plot (int plotcode, int x, int y)
; by Jason Williams 1993
;
        GET     macros.h
        GET     RegNames.h
        GET     swinos.h
;
        PREAMBLE
        STARTCODE GFX_Plot
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_Plot
        LDMFD   sp!,{pc}
;
        END
