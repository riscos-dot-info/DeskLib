; Author: Copyright 1993 Jason Williams

        GET     ^.h.regdefs
        GET     ^.h.swinos
        GET     ^.h.macros

; os_error *PopUp_Close(popup_handle)

        PREAMBLE
        STARTCODE PopUp_Close
;
        STMFD   sp!, {lr}
        SWI     SWI_PopUp_Close + XOS_Bit
        LDMFD   sp!, {pc}
;
        END
