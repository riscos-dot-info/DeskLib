;
;       Title                  : Wimp Poll
;       System                 : Wimp Library
;       Version                : 1.10
;       Copyright              : (C) John Winters
;       Date                   : 22nd September, 1991
;       Author                 : John H. Winters
;
;       Modifications          : 1.10 Jason Williams
;                                Added support for passing r3 (non-0 pollword)
;                                (new feature of the RISC OS 3 WIMP)
;
;
;============================================================================
;
;  Include files.
;
;============================================================================
;
        GET     RegDefs.h
        GET     SwiNos.h
        GET     Macros.h
;
;============================================================================
;
;  Code.
;
;============================================================================
;
        PREAMBLE

        STARTCODE Wimp_CorruptFPStateOnPoll
;
        LDR     a1, [pc, #L00001c-.-8]
        MOV     a2, #0
        STR     a2, [a1, #0]
        MOVS    pc, lr
L00001c
        DCD     |x$dataseg|

        IMPORT  |x$stack_overflow|
        IMPORT  |_kernel_fpavailable|
        ;MPORT  fp_status_save
        IMPORT  |_printf|
;
;
        STARTCODE Wimp_Poll3
;
        MOV     ip, sp
        STMFD   sp!, {v1, v2, v3, fp, ip, lr, pc}
        SUB     fp, ip, #4
        CMPS    sp, sl
        BLLT    |x$stack_overflow|
;
;  Save the parameters.
;
        MOV     v3, a3
        MOV     v2, a1
        MOV     v1, a2
;
;  Check whether to save the FP status.
;
        LDR     a1, [pc, #L00001c-.-8]
        LDR     a1, [a1, #0]
        CMPS    a1, #0
        BLNE    |_kernel_fpavailable|
        CMPNES  a1, #0
        BLNE    fp_status_save
;
;  Save a flag indicating whether we saved or not.
;  Note that the use of lr as a temporary variable precludes this
;  code running in SVC mode.
;
        MOV     lr, a1
;
;  Do the poll.
;
        MOV     a1, v2
        ADD     a2, v1, #4
        MOV     a4, v3
        SWI     SWI_Wimp_Poll + XOS_Bit
        SUB     a2, a2, #4
        STR     a1, [a2, #0]
        MOVVC   a1, #0
        CMP     lr, #0
        BLNE    fp_status_restore
        LDMDB   fp, {v1, v2, v3, fp, sp, pc}

        STARTCODE Wimp_PollIdle3
;
        MOV     ip, sp
        STMFD   sp!, {v1 - v4, fp, ip, lr, pc}
        SUB     fp, ip, #4
        CMPS    sp, sl
        BLLT    |x$stack_overflow|
;
;  Save the parameters.
;
        MOV     v2, a1
        MOV     v1, a2
        MOV     v3, a3
        MOV     v4, a4
;
;  Check whether to save the fp status.
;
        LDR     a1, [pc, #L00001c-.-8]
        LDR     a1, [a1, #0]
        CMPS    a1, #0
        BLNE    |_kernel_fpavailable|
        CMPNES  a1, #0
        BLNE    fp_status_save
        MOV     lr, a1
        MOV     a1, v2
        ADD     a2, v1, #4
        MOV     a3, v3
        MOV     a4, v4
        SWI     SWI_Wimp_PollIdle + XOS_Bit
        CMP     lr, #0
        BLNE    fp_status_restore
        SUB     a2, a2, #4
        STR     a1, [a2, #0]
        MOVVC   a1, #0
        LDMDB   fp, {v1-v4, fp, sp, pc}


        STARTCODE Wimp_SaveFPStateOnPoll
;
        LDR     a1, [pc, #L00001c-.-8]
        MOV     a2, #1
        STR     a2, [a1, #0]
        MOVS    pc, lr

fp_status_restore
        MOV     v1, #0
        WFS    v1
        LDFE    f4, [sp, #0]
        LDFE    f5, [sp, #12]
        LDFE    f6, [sp, #24]
        LDFE    f7, [sp, #36]
        ADD     sp, sp, #48
        LDMIA   sp!, {v1}
        WFS    v1
        MOVS    pc, lr

fp_status_save
        RFS    a2
        STMDB   sp!, {a2}
        MOV     a2, #0
        WFS    a2
        SUB     sp, sp, #48
        STFE    f4, [sp, #0]
        STFE    f5, [sp, #12]
        STFE    f6, [sp, #24]
        STFE    f7, [sp, #36]
        MOVS    pc, lr

    AREA |C$$data|

|x$dataseg|

save_fp
        DCD     &00000000


    END
