/************************************************************************
 *                                                                      *
 *        File: ProcessWin.h                                            *
 *                                                                      *
 *     Purpose: Provides user interface to image processing functions   *
 *                                                                      *
 ************************************************************************/

 /*
  * initialisation to be done at startup
  */
void ProcessWin_Init(void);


 /*
  * called with the process number (menuselection on process menu)
  * and source image number to open the correct process window
  */
void ProcessWin_Start(int process, int sourceimage);

