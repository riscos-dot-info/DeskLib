; Author: Copyright 1993 Shaun Blackmore

        GET     ^.h.regdefs
        GET     ^.h.swinos
        GET     ^.h.macros

;os_error *Font_ReadThresholds(FontThresholds *t);


        PREAMBLE
        STARTCODE Font_ReadThresholds
;
        STMFD   sp!, {lr}
        MOV     r1,r0
        SWI     SWI_Font_ReadThresholds + XOS_Bit
        MOVVC   r0,#0
        LDMFD   sp!, {pc}
        END
