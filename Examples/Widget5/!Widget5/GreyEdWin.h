/************************************************************************
 *                                                                      *
 *        File: GreyEdWin.h                                             *
 *                                                                      *
 *     Purpose: Provides user interface to edit greyelevel process      *
 *                                                                      *
 ************************************************************************/

 /*
  * initialisation to be done at startup
  */
void GreyEdWin_Init(void);


 /*
  * call to open greylevel edit process window
  */
void GreyEdWin_Open(int source, int dest, wimp_point *openpos);

