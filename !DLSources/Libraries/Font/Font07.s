; Author: Copyright 1993 Shaun Blackmore

        GET     ^.h.regdefs
        GET     ^.h.swinos
        GET     ^.h.macros

;os_error *Font_Caret(int colour, int height, int flags, int x, int y)



        PREAMBLE
        STARTCODE Font_Caret
;
        MOV     ip,sp
        STMFD   sp!, {r4,lr}
        LDR     r4,[ip,#0]
        SWI     SWI_Font_Caret + XOS_Bit
        MOVVC   r0,#0
        LDMFD   sp!, {r4,pc}
;
        END
