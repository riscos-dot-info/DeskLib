; Author: Copyright 1993 Jason Williams

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h

; os_error *PopUp_Open(popup_block *params)

        PREAMBLE
        STARTCODE PopUp_Open
;
        STMFD   sp!, {lr}
        SWI     SWI_PopUp_Open + XOS_Bit
        LDMFD   sp!, {pc}
;
        END
