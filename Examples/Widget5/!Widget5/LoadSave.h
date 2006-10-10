/************************************************************************
 *                                                                      *
 *        File: LoadSave.h                                              *
 *                                                                      *
 *     Purpose: Provides routines to load and save images as Widget5    *
 *              (text), Widget4 (simple byte per pixel) and Sprite      *
 *              format.                                                 *
 *                                                                      *
 ************************************************************************/

 /*
  * filetype number for files that belong to program
  */
#define filetype_WIDGET5 0x042
#define filetype_WIDGET4 0x041

 /*
  * initialisation to be done on startup
  */
extern void LoadSave_Init(void);

 /*
  * checks to see if the prog was started up by doubleclicking on a file and
  * so loads this file
  */
extern void LoadSave_CheckStartupLoad(int argc, char *argv[]);

 /*
  * sets the number of the image bank being saved, and the number of greys for the image
  */
extern void LoadSave_SetSaveBank(int imageno, int numbergreys);

 /*
  * call this to save the file after setting up the image bank
  */
extern void LoadSave_savefile(char *filename, int filetype, BOOL safe);

