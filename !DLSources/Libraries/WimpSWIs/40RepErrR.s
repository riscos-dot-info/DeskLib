; Wimp_ReportErrorR
; - a version of Wimp_ReportError that actually returns the value (which
;   button was clicked) that the SWI call returns.
;   Jason Williams 1993

        GET     regdefs.h
        GET     swinos.h
        GET     macros.h
;
        PREAMBLE
        STARTCODE Wimp_ReportErrorR
;
;        STMFD   sp!, {lr}
        MOV     ip, lr

        SWI     SWI_Wimp_ReportError + XOS_Bit
        MOV     a1, a2

        MOV     pc, ip
;        LDMFD   sp!, {pc}
;
        END
