; Copyright 1993 Shaun Blackmore

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h

;        void Filter_RegisterPreFilter(char *FilterName, FilterHandler handler, int R12, int TaskHandle);

        PREAMBLE
        STARTCODE Filter_RegisterPreFilter
;
        STMFD   sp!, {lr}
        SWI     SWI_Filter_RegisterPreFilter
        LDMFD   sp!, {pc}
;
        END
