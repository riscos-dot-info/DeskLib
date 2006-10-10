/************************************************************************
 *                                                                      *
 *        File: CheckBanks.h                                            *
 *                                                                      *
 *     Purpose: Provides routines to keep the state of the image bank   *
 *              icons updated.                                          *
 *                                                                      *
 ************************************************************************/


 /*
  * definition of the closewindow routine that get called when the
  * window is / has to be closed
  */
typedef void (*closewindow_routine)(BOOL ok, BOOL keepopen, void *range); 

 /*
  * information block passed to all the procedures
  */
typedef struct {
   window_handle window;
   int min;
   int max;
   int sourceimage; /* only used for scan destination and scan source */
   closewindow_routine close;   /* only used for scan destination */
   wimp_point size;             /* only used for scan destination and scan source */
} iconrange_block;

 /*
  * initialisation to be done on startup
  */
extern void CheckBanks_Init(void);

 /*
  * install routines to update image bank radios to make used banks greyed out
  */
extern void CheckBanks_InstallScanFree(iconrange_block *range);

 /*
  * remove routines which update image bank radios to make used banks greyed out
  */
extern void CheckBanks_RemoveScanFree(iconrange_block *range);


 /*
  * return the number of the image bank selected
  */
extern int image_getselect(iconrange_block *range);


 /*
  * install routines to update image bank radios to make unusable banks greyed out
  */
extern void CheckBanks_InstallScanDestination(iconrange_block *range);

 /*
  * remove routines that update image bank radios to make unusable banks greyed out
  */
extern void CheckBanks_RemoveScanDestination(iconrange_block *range);


 /*
  * install routines to update image bank radios to greyed out unusable input
  * images
  */
extern void CheckBanks_InstallScanSource(iconrange_block *range);

 /*
  * remove routines to update image bank radios to greyed out unusable input
  * images
  */
extern void CheckBanks_RemoveScanSource(iconrange_block *range);





