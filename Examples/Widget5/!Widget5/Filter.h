/************************************************************************
 *                                                                      *
 *        File: Filter.h                                                *
 *                                                                      *
 *     Purpose: Builds filter menu for convolution window from Messages *
 *              file and fills matrix on window with numbers on menu    *
 *              selections.                                             *
 *                                                                      *
 ************************************************************************/
                         
 /*
  * initialisation to be done on startup
  */
extern void Filter_Init(void);

 /*
  * installs pop-up menu on convolution window 'window'
  */
extern void Filter_InstallMenu(window_handle window);


 /*
  * removes pop-up menu from convolution window 'window'
  */
extern void Filter_RemoveMenu(window_handle window);


 /*
  * this fills 'anchor' and 'matrix' with the values from 'window'
  */
extern void Filter_GetMatrix(window_handle window,
                             wimp_point *anchor, int (*matrix)[3][3]);







