/************************************************************************
 *                                                                      *
 *        File: MixWin.h                                                *
 *                                                                      *
 *     Purpose: Provides user interface to mix two images process       *
 *                                                                      *
 ************************************************************************/

 /*
  * initialisation to be done at startup
  */
void MixWin_Init(void);


 /*
  * call to open mix process window
  */
void MixWin_Open(int source1, int source2, int dest, wimp_point *openpos);

