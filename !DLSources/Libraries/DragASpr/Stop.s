; Copyright 1993 Shaun Blackmore

        GET     ^.h.regdefs
        GET     ^.h.swinos
        GET     ^.h.macros

; void DragASprite_Stop(void);

        PREAMBLE
        STARTCODE DragASprite_Stop
;
        STMFD   sp!, {lr}
        SWI     SWI_DragASprite_Stop + XOS_Bit
        MOVVC   r0, #0
        LDMFD   sp!, {pc}
;
        END
