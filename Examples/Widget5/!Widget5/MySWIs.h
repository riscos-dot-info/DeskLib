/*
 * SWI definitions for ArcComm
 */

#define OS_CLI                          0x000005

/* #define SWI_OS_ReadSysInfo              0x000058 */
/* #define SWI_OS_Module                   0x00001e */

#ifndef SWI_OS_GBPB
#define SWI_OS_GBPB                     0x00000c
#endif

#define SWI_Wimp_ReadPalette            0x0400e5
#define SWI_Wimp_SetPalette             0x0400e4
#define SWI_Wimp_SetColourMapping       0x0400f8
#define SWI_Wimp_CreateIcon             0x0400c2
#define SWI_Wimp_StartTask              0x0400de
#define SWI_Wimp_SetMode                0x0400e3
#define SWI_Wimp_GetWindowState         0x0400cb
#define SWI_Wimp_GetIconState           0x0400ce
#define SWI_Wimp_ForceRedraw            0x0400d1
#define SWI_Wimp_BlockCopy              0x0400eb
#define SWI_ColourTrans_InvalidateCache 0x040750
#define SWI_Font_ListFonts              0x040091
#define SWI_Font_DecodeMenu             0x0400a0

#define SWI_OutlineWimp_Fonthandle      0x0496C0
#define SWI_OutlineWimp_KeyShortCut     0x0496C1
#define SWI_OutlineWimp_Configuration   0x0496C2

#define SWI_ZapRedraw_RedrawArea        0x048480 + 0x00
#define SWI_ZapRedraw_ReadVduVars       0x048480 + 0x0e
#define SWI_ZapRedraw_CreatePalette     0x048480 + 0x0a
#define SWI_ZapRedraw_GetRectangle      0x048480 + 0x0f
#define SWI_ZapRedraw_RedrawWindow      0x048480 + 0x13
#define SWI_ZapRedraw_ConvertBitmap     0x048480 + 0x03
#define SWI_ZapRedraw_CachedCharSize    0x048480 + 0x08

#define SWI_ColourTrans_ReturnColourNumber 0x040744
#define SWI_ColourTrans_SelectTable     0x040740

#ifndef SWI_OS_File
#define SWI_OS_File                     0x000008
#endif

#ifndef SWI_OS_ReadVarVal
#define SWI_OS_ReadVarVal               0x000023
#endif


