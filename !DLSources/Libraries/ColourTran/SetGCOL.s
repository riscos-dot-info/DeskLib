;-------------------------------------------------------------------------------
; ColourTrans_SetGCOL(int r0, int r1, int r2);
; Copyright 1993 Shaun Blackmore


        GET     macros.h
        GET     ^.h.RegNames
        GET     ^.h.SWInames
;
        PREAMBLE
        STARTCODE ColourTrans_SetGCOL
;
        STMFD   sp!,{r4,lr}
        MOV     r3,r1
        MOV     r4,r2
        SWI     XSWI_ColourTrans_SetGCOL
        MOVVC   r0, #0
        LDMFD   sp!,{r4,pc}
;
        END
