; Author: Copyright 1993 Shaun Blackmore

        GET     ^.h.regdefs
        GET     ^.h.swinos
        GET     ^.h.macros

;os_error *Font_SetFont(FontHandle font);



        PREAMBLE
        STARTCODE Font_SetFont
;
        STMFD   sp!, {lr}
        SWI     SWI_Font_SetFont + XOS_Bit
        MOVVC   r0,#0
        LDMFD   sp!, {pc}
;
        END
