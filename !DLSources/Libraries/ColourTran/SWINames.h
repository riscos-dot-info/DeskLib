; h.swinames
;
; This header file for GETting from assembler source defines OS SWI names
;
;**************************************************************************
;
; SWI names are exported in two forms :
; 1)  with OS_  'OS_DoThingToOtherThing'  as N
; 2)  with XOS_ 'XOS_DoThingToOtherThing' as N + Auto_Error_SWI_bit
;

Auto_Error_SWI_bit_number * 17
Auto_Error_SWI_bit * 1 :SHL: Auto_Error_SWI_bit_number

;
; The following macro defines the SWI names using the '*' directive and the
; '#' directive to increment the SWI number
;

        GBLS    SWIClass
        MACRO
        AddSWI  $SWIName,$value
  [     "$value" = ""
$SWIClass._$SWIName # 1
  |
$SWIClass._$SWIName * $value
  ]
X$SWIClass._$SWIName * $SWIClass._$SWIName + Auto_Error_SWI_bit
        MEND

;
; Now for the SWI name table - using the '^' directive to build a table
; of SWI numbers
;

SWIClass SETS   "SWI_ColourTrans"

        ^       &40740                       ; Base for ColourTrans SWIs
        AddSWI  SelectTable                  ; &00
        AddSWI  SelectGCOLTable              ; &01
        AddSWI  ReturnGCOL                   ; &02
        AddSWI  SetGCOL                      ; &03
        AddSWI  ReturnColourNumber           ; &04
        AddSWI  ReturnGCOLForMode            ; &05
        AddSWI  ReturnColourNumberForMode    ; &06
        AddSWI  ReturnOppGCOL                ; &07
        AddSWI  SetOppGCOL                   ; &08
        AddSWI  ReturnOppColourNumber        ; &09
        AddSWI  ReturnOppGCOLForMode         ; &0A
        AddSWI  ReturnOppColourNumberForMode ; &0B
        AddSWI  GCOLToColourNumber           ; &0C
        AddSWI  ColourNumberToGCOL           ; &0D
        AddSWI  ReturnFontColours            ; &0E
        AddSWI  SetFontColours               ; &0F
        AddSWI  InvalidateCache              ; &10 

        END
