; Copyright 1993 Shaun Blackmore

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h

;        void Filter_DeRegisterPreFilter(char *FilterName, FilterHandler handler, int R12, int TaskHandle);

        PREAMBLE
        STARTCODE Filter_DeRegisterPreFilter
;
        STMFD   sp!, {lr}
        SWI     SWI_Filter_DeRegisterPreFilter
        LDMFD   sp!, {pc}
;
        END
