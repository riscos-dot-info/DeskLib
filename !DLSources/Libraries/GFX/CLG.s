;-------------------------------------------------------------------------------
; GFX_CLG(void)
; by Jason Williams 1993
;
        GET     macros.h
        GET     RegNames.h
        GET     swinos.h
;
        PREAMBLE
        STARTCODE GFX_CLG
;
        STMFD   sp!,{lr}
        SWI     XSWI_OS_WriteI + 16
        LDMFD   sp!,{pc}
;
        END
