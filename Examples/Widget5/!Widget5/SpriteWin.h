/************************************************************************
 *                                                                      *
 *        File: SpriteWin.h                                             *
 *                                                                      *
 *     Purpose: Store 256 grey level arbitrary sized images in 6 'image *
 *              banks' and display them in a window using a pixel       *
 *              colour translation table to map the grey levels to the  *
 *              screen palette if required.                             *
 *                                                                      *
 ************************************************************************/

 /*
  * number of image banks
  */
#define number_IMAGES 6

 /*
  * image bank information block
  */
typedef struct {
    window_handle window;
    int                 imageno;          /* for internal use only */
    sprite_area         spritearea;
    int                 header;           /* offset from start of spritearea to header */
    int                 image;            /* offset from start of spritearea to image */
    int                 linebytes;
    wimp_point          size;
    wimp_point          plotcoord;
    sprite_scalefactors scale;            /* used for zoom window */
    sprite_scalefactors screenscale;      /* used on screen */
    char                *pixtranstable;
    char                pixtransbuffer[256];
} spritewin_block;

 /*
  * initialisation to be done on startup
  */
extern void SpriteWin_Init(void);

 /*
  * returns TRUE if the image is free
  */
extern BOOL SpriteWin_FreeImage(int imageno);

 /*
  * returns TRUE if image is same size as 'res'
  */
extern BOOL SpriteWin_SameSize(wimp_point *res, int imageno);

 /*
  * returns TRUE if both images are the same size
  */
extern BOOL SpriteWin_Identical(int image1, int image2);

 /*
  * refreshes the window of the image bank 'imageno'
  */
extern void SpriteWin_RefreshWin(int imageno);

 /*
  * creates blank image at size 'res' in image bank 'imageno' and opens window
  * returns TRUE if image created
  */
extern BOOL SpriteWin_Create(wimp_point *res, int imageno);

 /*
  * creates just blank image in 'block' at size 'res'
  */
extern BOOL SpriteWin_CreateImage(spritewin_block *block, wimp_point *res);
    
 /*
  * creates window for image in 'block' and displays it
  */
extern void SpriteWin_CreateWindow(spritewin_block *block);

 /*
  * returns the value of the pixel at 'pos'
  */
extern int SpriteWin_GetPixel(wimp_point *pos, int imageno);

 /*
  * sets the colour of the pixel at 'pos' to 'pixel'
  */
extern void SpriteWin_PutPixel(wimp_point *pos, int pixel, int imageno);

 /*
  * returns the size of the image 'imageno'
  */
extern wimp_point *SpriteWin_GetSize(int imageno);

 /*
  * returns a pointer to the spritewin_block for image bank 'imageno'
  */
extern spritewin_block *SpriteWin_GetImageBlock(int imageno);

 /*
  * adds the image bank info block 'block' to the list of image banks
  * returns FALSE if bank in use
  */
extern BOOL SpriteWin_AddImageBlock(spritewin_block *block, int imageno);

 /*
  * removes the image bank info block 'block' from the list of image banks
  */
extern void SpriteWin_RemoveImageBlock(int imageno);

 /*
  * initialises variables in information block
  */
extern void SpriteWin_InitBlock(spritewin_block *block, int imageno);

 /*
  * closes savebox and menu for save widget 5 file
  */
extern void SpriteWin_CloseSaveWidget5(void);

 /*
  * closes savebox and menu for all other file formats
  */
extern void SpriteWin_CloseSave(void);
