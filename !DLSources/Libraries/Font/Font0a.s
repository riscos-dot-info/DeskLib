; Author: Copyright 1993 Shaun Blackmore

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h

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
