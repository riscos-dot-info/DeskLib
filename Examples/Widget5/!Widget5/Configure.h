/************************************************************************
 *                                                                      *
 *        File: Configure.h                                             *
 *                                                                      *
 *     Purpose: Configuration options for program.                      *
 *              Routines to load and save options and display window to *
 *              change options.                                         *
 *                                                                      *
 ************************************************************************/

 /*
  * size of widget 4 image for the loading functions
  */
extern wimp_point widget4size;

 /*
  * whether the destination radios should show free banks or freebanks and banks of
  * the same size in the process windows
  */
#define use_FREE       0
#define use_FREEORSAME 1
extern int destinationbanktype;

 /*
  * size of graph for histograms
  */
#define graphtype_LARGE 0
#define graphtype_SMALL 1
extern int graphtype;

 /*
  * TRUE when the full version is enabled.
  * ie. support for Widget4 and Widget5 file formats
  */
extern BOOL fullversion;

 /*
  * smoothed drawing setting for drawing on the greymap
  */
extern int drawtype;
#define drawtype_NORMAL;
#define drawtype_SMOOTH;
#define drawtype_VSMOOTH;

 /*
  * real-time updates
  */
extern BOOL realtime;

 /*
  * open configure window on screen
  */
extern void Configure_Open(void);


 /*
  * initialisation to be done on startup (loads options file aswell)
  */
extern void Configure_Init(void);


