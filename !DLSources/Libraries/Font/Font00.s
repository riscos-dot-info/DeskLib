; Author: Copyright 1993 Shaun Blackmore

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h

; os_error *Font_CacheAddr(int *version, int *cacheused, int *cachesize);

        PREAMBLE
        STARTCODE Font_CacheAddr
;
        STMFD   sp!, {v1,v2,v3,lr}
        MOV     v1,a1
        MOV     v2,a2
        MOV     v3,a3
        SWI     SWI_Font_CacheAddr + XOS_Bit
        STR     r0,[v1]
        STR     r3,[v2]
        STR     r2,[v3]
        MOVVC   a1,#0
        LDMFD   sp!, {v1,v2,v3,pc}
;
        END
