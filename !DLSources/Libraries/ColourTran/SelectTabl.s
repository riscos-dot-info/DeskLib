;-------------------------------------------------------------------------------
; ColourTrans_SelectTable(int src_mode, int *src_pal,
;                         int dst_mode, int *dst_pal,
;                         char *table);
; Copyright 1993 Shaun Blackmore


        GET     macros.h
        GET     ^.h.RegNames
        GET     ^.h.SWInames
;
        PREAMBLE
        STARTCODE ColourTrans_SelectTable
;
        MOV     ip,sp
        STMFD   sp!,{r4,lr}
        LDR     r4,[ip,#0]
        SWI     XSWI_ColourTrans_SelectTable
        MOVVC   r0, #0
        LDMFD   sp!,{r4,pc}
;
        END
