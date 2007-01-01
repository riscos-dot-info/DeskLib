@ Author Copyright 1993 Shaun Blackmore

        .include     "RegDefs.h"
        .include     "SwiNos.h"
        .include     "Macros.h"

@ os_error *Font_LoseFont(FontHandle font);

@       r0=font;


        
        .globl Font_LoseFont
@
        STMFD   sp!, {lr}
        SWI     SWI_Font_LoseFont + XOS_Bit
        MOVVC   r0,#0
        LDMFD   sp!, {pc}
@
