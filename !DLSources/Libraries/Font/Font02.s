; Author: Copyright 1993 Shaun Blackmore

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h

; os_error *Font_LoseFont(FontHandle font);

;       r0=font;


        PREAMBLE
        STARTCODE Font_LoseFont
;
        STMFD   sp!, {lr}
        SWI     SWI_Font_LoseFont + XOS_Bit
        MOVVC   r0,#0
        LDMFD   sp!, {pc}
;
        END
