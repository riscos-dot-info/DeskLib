/************************************************************************
 *                                                                      *
 *        File: Misc.c                                                  *
 *                                                                      *
 *     Purpose: Provides miscellanous routines used by more than one    *
 *              file in the program                                     *
 *                                                                      *
 ************************************************************************/

extern void inc_icon(window_handle window, icon_handle icon, int upperlimit);
extern void dec_icon(window_handle window, icon_handle icon, int lowerlimit);

 /*
  * set filetype of icon
  */
extern void Icon_SetFileType(window_handle window, icon_handle icon, int filetype);

 /*
  * returns filetype or -1 if object not found
  */
extern void Popup_menu(window_handle window, icon_handle icon, menu_ptr menu);

 /*
  * makes the entry in the menu send a menuwarn message or not
  * as yesno
  */
extern void Menu_MakeWarn(menu_ptr menu, int entry, BOOL yesno);

 /*
  * gets the current position of the topleft corner of the window
  */
extern void Window_GetPos(window_handle window, wimp_point *pos);

 /*
  * Shows the window in the position openpos
  */
extern void Window_ShowPos(window_handle window, wimp_point *openpos);


