/************************************************************************
 *                                                                      *
 *        File: LoadSave.c                                              *
 *                                                                      *
 *     Purpose: Provides routines to load and save images as Widget5    *
 *              (text), Widget4 (simple byte per pixel) and Sprite      *
 *              format.                                                 *
 *                                                                      *
 ************************************************************************/

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Core.h"              /* usefull core functions          */
#include "DeskLib:Error.h"             /* Error despatcher                */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:File.h"              /* Low level file handling         */
#include "DeskLib:Msgs.h"              /* Message translation code        */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Sound.h"             /* Beep!                           */
#include "DeskLib:Template.h"          /* Template loading and caching    */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Screen.h"            /* Caching screen info             */
#include "DeskLib:Sprite.h"            /* Sprite handling routines        */
#include "DeskLib:File.h"              /* OS file IO                      */
#include "DeskLib:KeyCodes.h"          /* Codes for wimp returned keys    */
#include "DeskLib:Hourglass.h"         /* control hourglass               */

#include "HackLib:flex.h"              /* Flexable malloc                 */

#include "ATrans.h"                    /* My load/save library            */

#include "kernel.h"                    /* clib kernel functions           */
#include "MySWIs.h"                    /* SWI no defs                     */
#include "LoadSave.h"
#include "SpriteWin.h"
#include "Misc.h"
#include "CheckBanks.h"
#include "Configure.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern window_handle db;

/* icon numbers for image bank radios */
#define image_MIN 11
#define image_MAX 16

/* image bank number that is being saved */
static int saveimagebank = -1;
/* number of greys in saved image */
static int savenumbergreys = 0;

typedef struct {
   iconrange_block range;
   spritewin_block *image;
} tempimage_block;

void load_widget5(char *filename);
void load_widget4(char *filename);
void load_sprite(char *filename);
void load_abort1(file_handle channel, tempimage_block *block);
void load_abort2(file_handle channel, tempimage_block *block);
void save_widget5(char *filename, int imageno, int numbergreys);
void save_widget4(char *filename, int imageno);
void save_sprite(char *filename, int imageno);
tempimage_block *open_load(char *filename, int filetype);
void close_load(tempimage_block *block, BOOL ok);
static BOOL Close_load(event_pollblock *event, void *reference);
static BOOL Click_load(event_pollblock *event, void *reference);
BOOL write_newline(file_handle channel);
BOOL write_space(file_handle channel);
BOOL write_text(file_handle channel, char *text, int padto);
BOOL read_charint(file_handle channel, int *integer);
void NoLoad(char *err);
void NoMemLoad(void);
void NoSave(char *err);
void InvalidFile(void);
void TooShort(void);

/******************************************************************************
 *                            ATrans routines                                 *
 ******************************************************************************/

static BOOL checkload(int filetype, BOOL data_open)
{
 /*
  * called when a file is dragged to the app (data_open = FALSE)
  * or double clicked (data_open = TRUE)
  * returns TRUE if the app wants to load that file
  */

  switch(filetype)
  {
    case filetype_WIDGET5: /* load widget5 always when enabled */
      return(fullversion);
      break;

    case filetype_WIDGET4: /* load widget4 always when enabled */
      return(fullversion);
      break;

    case filetype_SPRITE: /* load sprite when dragged only */
      return(!data_open);
      break;
  }

  return(FALSE); /* don't load anything else */
}

/******************************************************************************/

static void loadfile(char *filename, int filetype, BOOL safe)
{

  Hourglass_On();

  switch(filetype)
  {
    case filetype_WIDGET5:
      load_widget5(filename);
      break;

    case filetype_WIDGET4:
      load_widget4(filename);
      break;

    case filetype_SPRITE:
      load_sprite(filename);
      break;

  }

  Hourglass_Off();

}

/******************************************************************************/

extern void LoadSave_savefile(char *filename, int filetype, BOOL safe)
{

  if(saveimagebank != -1){

     Hourglass_On();

     switch(filetype)
     {
       case filetype_WIDGET5:
         SpriteWin_CloseSaveWidget5();
         save_widget5(filename, saveimagebank, savenumbergreys);
         break;

       case filetype_WIDGET4:
         SpriteWin_CloseSave();
         save_widget4(filename, saveimagebank);
         break;

       case filetype_SPRITE:
         SpriteWin_CloseSave();
         save_sprite(filename, saveimagebank);
         break;

     }

     Hourglass_Off();
  }

  saveimagebank = -1;

}

/******************************************************************************
 *                            extern routines                                 *
 ******************************************************************************/

extern void LoadSave_Init(void)
{
 /*
  * initialisation to be done on startup
  */

  ATrans_Init(checkload, loadfile, LoadSave_savefile, NULL);

}

/******************************************************************************/

extern void LoadSave_CheckStartupLoad(int argc, char *argv[])
{
 /*
  * checks to see if the prog was started up by doubleclicking on a file and
  * so loads this file
  */
  int filetype;

 /*
  * return if no file to load
  */
  if(argc == 1)
     return;

 /*
  * call loadfile with the filename and filetype of the input file
  */
  filetype = File_GetType(argv[1]);

  loadfile(argv[1], filetype, TRUE);

}

/******************************************************************************/

extern void LoadSave_SetSaveBank(int imageno, int numbergreys)
{
 /*
  * sets the number of the image bank being saved
  */
  
  saveimagebank = imageno;
  savenumbergreys = numbergreys;

}

/******************************************************************************
 *                             load routines                                  *
 ******************************************************************************/

void load_widget5(char *filename)
{
  tempimage_block *block;
  file_handle channel;
  int value;
  wimp_point res;
  wimp_point pos;
  int numbergreys;
  char *image;
  int linebytes;
  char greys[15];

 /*
  * open load file window
  */
  if( (block = open_load(filename, filetype_WIDGET5)) == NULL )
     return;

/* from now on block contains 2 malloced areas and the scan routine is on */

 /*
  * open file and return if fails
  */
  channel = File_Open(filename, file_READ);
  if(channel == NULL){
     if(file_lasterror != NULL){
        NoLoad(file_lasterror->errmess);
        load_abort1(channel, block);
        return;
     }
  }

 /*
  * read in size of image and number of greys
  */
  if( !read_charint(channel, &res.x) ) {
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

  if( !read_charint(channel, &res.y) ) {
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

  if( !read_charint(channel, &numbergreys) ) {
     InvalidFile();
     load_abort1(channel, block);
     return;
  }


 /*
  * set size and number of greys in window
  */
  Icon_SetInteger(block->range.window, 4, res.x);
  Icon_SetInteger(block->range.window, 6, res.y);
  Msgs_Lookup("txt.greys", greys, 14);
  Icon_printf(block->range.window, 8, "%i %s", numbergreys, greys);

 /*
  * create blank image of correct size
  */
  SpriteWin_InitBlock(block->image, 0);
  if( !SpriteWin_CreateImage(block->image, &res) ) {
     NoMemLoad();
     load_abort1(channel, block);
     return;
  }
  
/* from now on block contains 2 malloc areas, the scan routine is on, and sprite area */

  image = (char *)(block->image)->spritearea + (block->image)->image;
  linebytes = (block->image)->linebytes;

 /*
  * read in image data, scaled to 256 grey levels
  */
  for(pos.y = 0; pos.y < res.y; pos.y++){

     Hourglass_Percentage( (pos.y * 100) / res.y);
  
     for(pos.x = 0; pos.x < res.x; pos.x++){

        if( !read_charint(channel, &value) ){
           TooShort();
           File_Close(channel);
           return;
        }
        image[pos.x + (pos.y * linebytes)] = (value * 256) / numbergreys;
     }
  }

  File_Close(channel);
 
}

/******************************************************************************/

void load_widget4(char *filename)
{
  tempimage_block *block;
  file_handle channel;
  wimp_point res;
  wimp_point pos;
  char *image;
  int linebytes;
  char greys[15];

 /*
  * open load file window
  */
  if( (block = open_load(filename, filetype_WIDGET4)) == NULL )
     return;

/* from now on block contains 2 malloced areas and the scan routine is on */

 /*
  * open file and return if fails
  */
  channel = File_Open(filename, file_READ);
  if(channel == NULL){
     if(file_lasterror != NULL){
        NoLoad(file_lasterror->errmess);
        load_abort1(channel, block);
        return;
     }
  }

 /*
  * set size and number of greys in window
  */
  res = widget4size;
  
  Icon_SetInteger(block->range.window, 4, res.x);
  Icon_SetInteger(block->range.window, 6, res.y);
  Msgs_Lookup("txt.greys", greys, 14);
  Icon_printf(block->range.window, 8, "256 %s", greys);

 /*
  * create blank image of correct size
  */
  SpriteWin_InitBlock(block->image, 0);
  if( !SpriteWin_CreateImage(block->image, &res) ) {
     NoMemLoad();
     load_abort1(channel, block);
     return;
  }
  
/* from now on block contains 2 malloc areas, the scan routine is on, and sprite area */

  image = (char *)(block->image)->spritearea + (block->image)->image;
  linebytes = (block->image)->linebytes;

 /*
  * read in image data
  */
  for(pos.y = 0; pos.y < res.y; pos.y++){

     Hourglass_Percentage( (pos.y * 100) / res.y);
  
     if( File_ReadBytes(channel, &image[pos.y * linebytes], res.x) != 0 ){
        TooShort();
        File_Close(channel);
        return;
     }
     
  }

  File_Close(channel);
 
}

/******************************************************************************/

void load_sprite(char *filename)
{
  tempimage_block *block;
  file_handle channel;
  wimp_point res;
  sprite_header *header;

  int spriteoffset;
  int nextoffset;
  int imageoffset;
  int mode;
  int width;
  int height;
  int firstused;
  int lastused;

  int fileimagesize;
  int memoryimagesize;

  char greys[15];

 /*
  * open load file window
  */
  if( (block = open_load(filename, filetype_SPRITE)) == NULL)
     return;

/* from now on block contains 2 malloced areas and the scan routine is on */

 /*
  * open file and return if fails
  */
  channel = File_Open(filename, file_READ);
  if(channel == NULL){
     if(file_lasterror != NULL){
        NoLoad(file_lasterror->errmess);
        load_abort1(channel, block);
        return;
     }
  }

 /*
  * read offset to start of sprite area in file
  */
  File_Seek(channel, 4); /* offset to first sprite */
  spriteoffset = File_ReadInt(channel) - 4;
  if(spriteoffset == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read offset to next sprite in file (end of first sprite)
  */
  File_Seek(channel, spriteoffset);
  nextoffset = File_ReadInt(channel);
  if(nextoffset == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read width of image in words (32 bits) - 1
  */
  File_Seek(channel, spriteoffset + 16);
  width = File_ReadInt(channel);
  if(width == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read height of image in scan lines - 1
  */
  height = File_ReadInt(channel);
  if(height == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read first used bit in line of data value
  */
  firstused = File_ReadInt(channel);
  if(firstused == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read last used bit (in last word) in line of data value
  */
  lastused = File_ReadInt(channel);
  if(lastused == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read offset to start of image data from start of sprite
  */
  imageoffset = File_ReadInt(channel);
  if(imageoffset == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * read mode sprite is defined in
  */
  File_Seek(channel, spriteoffset + 40); /* mode */
  mode = File_ReadInt(channel);
  if(mode == file_READERROR){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

/* should check mode for 8bit, maybe later */

 /*
  * abort if data is not aligned to left of first word
  */
  if(firstused != 0){
     InvalidFile();
     load_abort1(channel, block);
     return;
  }

 /*
  * calculate resolution of image
  */
  res.x = (width * 4) - ((firstused + 1) / 8) + ((lastused + 1) / 8);
  res.y = height + 1;

 /*
  * set size and number of greys in window
  */
  Icon_SetInteger(block->range.window, 4, res.x);
  Icon_SetInteger(block->range.window, 6, res.y);
  Msgs_Lookup("txt.greys", greys, 14);
  Icon_printf(block->range.window, 8, "256 %s", greys);

 /*
  * create blank image of correct size
  */
  SpriteWin_InitBlock(block->image, 0);
  if( !SpriteWin_CreateImage(block->image, &res) ) {
     NoMemLoad();
     load_abort1(channel, block);
     return;
  }
  
/* from now on block contains 2 malloc areas, the scan routine is on, and sprite area */

 /*
  * calculate size of image data in file
  */
  fileimagesize = nextoffset - imageoffset;

 /*
  * calculate size of image data that can be held in memory
  */
  header = (void *) ((char *)(block->image)->spritearea +
                             ((block->image)->spritearea)->firstoffset);
  memoryimagesize = header->offset_next - header->imageoffset;


 /*
  * check if there is enough room in sprite area in memory and abort if not
  */
  if(memoryimagesize < fileimagesize){
     InvalidFile();
     load_abort2(channel, block);
     return;
  }

 /*
  * read in image data
  */
  File_Seek(channel, spriteoffset + imageoffset);
  if(File_ReadBytes(channel,
                 (char *)(block->image)->spritearea + (block->image)->image,
                 fileimagesize) != 0){
     TooShort();
  }

  File_Close(channel);

}

/******************************************************************************/

void load_abort1(file_handle channel, tempimage_block *block)
{
 /*
  * closes file if open, deallocates both blocks and turns off scan routines
  */
  
  if(channel != NULL)
     File_Close(channel);

  CheckBanks_RemoveScanFree(&block->range);

  Window_Delete(block->range.window);

  free(block->image);

  free(block);

}

/******************************************************************************/

void load_abort2(file_handle channel, tempimage_block *block)
{
 /*
  * closes file if open, deallocates both blocks, turns off scan routines
  * and frees the flex block spritearea
  */
  
  if(channel != NULL)
     File_Close(channel);

  CheckBanks_RemoveScanFree(&block->range);

  Window_Delete(block->range.window);

  flex_free((flex_ptr) &(block->image)->spritearea);

  free(block->image);

  free(block);

}

/******************************************************************************
 *                             save routines                                  *
 ******************************************************************************/

void save_widget5(char *filename, int imageno, int numbergreys)
{
  spritewin_block *block;
  file_handle channel;
  char buffer[15];
  wimp_point pos;
  int value;
  char *image;
  int linebytes;
  

  block = SpriteWin_GetImageBlock(imageno);

 /*
  * don't save if there isn't really an image
  */
  if(block->spritearea == NULL)
     return;

 /*
  * open file and return if fails
  */
  channel = File_Open(filename, file_WRITE);
  if(channel == NULL){
     if(file_lasterror != NULL){
        NoSave(file_lasterror->errmess);
        return;
     }
  }

 /*
  * set filetype
  */
  File_SetType(filename, filetype_WIDGET5);

 /*
  * write image size
  */
  sprintf(buffer, "%i", block->size.x);
  if(!write_text(channel, buffer, 0)) {
     NoSave(file_lasterror->errmess);
     File_Close(channel);
     return;
  }
  if(!write_newline(channel)) {
     NoSave(file_lasterror->errmess);
     File_Close(channel);
     return;
  }

  sprintf(buffer, "%i", block->size.y);
  if(!write_text(channel, buffer, 0)) {
     NoSave(file_lasterror->errmess);
     File_Close(channel);
     return;
  }
  if(!write_newline(channel)) {
     NoSave(file_lasterror->errmess);
     File_Close(channel);
     return;
  }

 /*
  * write number of greys
  */
  sprintf(buffer, "%i", numbergreys);
  if(!write_text(channel, buffer, 0)) {
     NoSave(file_lasterror->errmess);
     File_Close(channel);
     return;
  }
  if(!write_newline(channel)) {
     NoSave(file_lasterror->errmess);
     File_Close(channel);
     return;
  }

 /*
  * write image data
  */
  image = (char *)block->spritearea + block->image;
  linebytes = block->linebytes;

  for(pos.y = 0; pos.y < block->size.y; pos.y++){

     Hourglass_Percentage( (pos.y * 100) / block->size.y);

     for(pos.x = 0; pos.x < block->size.x; pos.x++){

        value = (image[pos.x + (pos.y * linebytes)] * numbergreys) / 256;
        sprintf(buffer, "%i", value);
        if(!write_text(channel, buffer, 4)) {
           NoSave(file_lasterror->errmess);
           File_Close(channel);
           return;
        }
     }

     if(!write_newline(channel)) {
        NoSave(file_lasterror->errmess);
        File_Close(channel);
        return;
     }

  }
  
  File_Close(channel);
  
}

/******************************************************************************/

void save_widget4(char *filename, int imageno)
{
  spritewin_block *block;
  file_handle channel;
  wimp_point pos;
  char *image;
  int linebytes;
  

  block = SpriteWin_GetImageBlock(imageno);

 /*
  * don't save if there isn't really an image
  */
  if(block->spritearea == NULL)
     return;

 /*
  * open file and return if fails
  */
  channel = File_Open(filename, file_WRITE);
  if(channel == NULL){
     if(file_lasterror != NULL){
        NoSave(file_lasterror->errmess);
        return;
     }
  }

 /*
  * set filetype
  */
  File_SetType(filename, filetype_WIDGET4);

 /*
  * write image data
  */
  image = (char *)block->spritearea + block->image;
  linebytes = block->linebytes;

  for(pos.y = 0; pos.y < block->size.y; pos.y++){

     Hourglass_Percentage( (pos.y * 100) / block->size.y);

     if( File_WriteBytes(channel, &image[pos.y * linebytes], block->size.x) != 0 ){
        NoSave(file_lasterror->errmess);
        File_Close(channel);
        return;
     }

  }
  
  File_Close(channel);
  
}

/******************************************************************************/

void save_sprite(char *filename, int imageno)
{
  spritewin_block *block;

  block = SpriteWin_GetImageBlock(imageno);

 /*
  * don't save if there isn't really an image
  */
  if(block->spritearea == NULL)
     return;

  Error_Check(Sprite_Save(block->spritearea, filename));

}

/******************************************************************************
 *                             open routines                                  *
 ******************************************************************************/

tempimage_block *open_load(char *filename, int filetype)
{
 /*
  * open place loaded image window on screen
  */
  tempimage_block *block;

 /*
  * allocate memory for range info block and return NULL if error if fails
  */
  block = malloc(sizeof(tempimage_block));
  if(block == NULL){
     NoMemLoad();
     return(block);
  }

 /*
  * allocate memory for image bank info block, deallocate block and return NULL if fails
  */
  block->image = malloc(sizeof(spritewin_block));
  if(block->image == NULL){
     NoMemLoad();
     free(block);
     block = NULL;
     return(block);
  }

 /*
  * create window and fill range info block
  */
  block->range.window = Window_Create("Load", template_TITLEMIN);
  block->range.min = image_MIN;
  block->range.max = image_MAX;

 /*
  * filename
  */
  Icon_SetTextRJ(block->range.window, 3, filename);

 /*
  * set filetype of icon
  */
  Icon_SetFileType(block->range.window, 2, filetype);

  CheckBanks_InstallScanFree(&block->range);

 /*
  * install event handlers
  */
  Event_Claim(event_CLICK, block->range.window, event_ANY, Click_load, block);

  Event_Claim(event_CLOSE, block->range.window, event_ANY, Close_load, block);

  Window_Show(block->range.window, open_WHEREVER);

  return(block);
}

/******************************************************************************
 *                            close routines                                  *
 ******************************************************************************/

void close_load(tempimage_block *block, BOOL ok)
{
  int imageno;

  if(ok == TRUE){

     imageno = image_getselect(&block->range);

    /*
     * don't do anything if no bank is selected
     */
     if(imageno == -1)
        return;

    /*
     * add image bank info block to list but do nothing if bank used
     */
     if( !SpriteWin_AddImageBlock(block->image, imageno) )
        return;

    /*
     * create window
     */
     SpriteWin_CreateWindow(block->image);


  }
  else{
     flex_free((flex_ptr) &(block->image)->spritearea);
     free(block->image);
  }

  CheckBanks_RemoveScanFree(&block->range);

  Window_Delete(block->range.window);

  free(block);

}

/******************************************************************************/

static BOOL Close_load(event_pollblock *event, void *reference)
{
  close_load((tempimage_block *)reference, FALSE);

  return(FALSE);
}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_load(event_pollblock *event, void *reference)
{
  tempimage_block *block = reference;

  switch(event->data.mouse.icon)
  {
    
    case 17: /* Cancel */
       close_load(block, FALSE);
       break;

    case 18: /* Place Loaded Image */
       close_load(block, TRUE);
       break;
      
  }
  return(TRUE);
}

/******************************************************************************
 *                               misc routines                                *
 ******************************************************************************/

BOOL write_newline(file_handle channel)
{
  if(File_WriteChar(channel, 10) != NULL)
     return(FALSE);

  return(TRUE);
}

/******************************************************************************/

BOOL write_space(file_handle channel)
{
  if(File_WriteChar(channel, ' ') != NULL)
     return(FALSE);

  return(TRUE);
}

/******************************************************************************/

BOOL write_text(file_handle channel, char *text, int padto)
{
  int length;
  int loop;

 /*
  * pad out text so that it is RJ with field size 'padto'
  */
  if( (length = strlen(text)) < padto){

     for(loop = 0; loop < (padto - length); loop++){

         if(!write_space(channel))
            return(FALSE);
     }
  }

 /*
  * write text
  */
  if( File_WriteBytes(channel, text, length) != 0)
     return(FALSE);

  return(TRUE);
}

/******************************************************************************/

BOOL read_charint(file_handle channel, int *integer)
{
 /*
  * reads in a text number and returns it in 'interger'
  * FALSE is returned if the read fails on the first try (EOF)
  */
  #define sizeof_BUFFER 15
  char buffer[15];
  int index;
  int character;

 /*
  * read in text up to divider character or EOF
  */
  index = 0;

 /*
  * skip any chars before number
  */
  while( ((character = File_ReadChar(channel)) != file_READERROR) &&
         (character <= ' ') );

 /*
  * 'character' is now either the first number character or file_READERROR
  */
  if(character != file_READERROR){

     buffer[index++] = (char) character;

     while( ((character = File_ReadChar(channel)) != file_READERROR) &&
            (character > ' ') && (index < (sizeof_BUFFER - 1)) ){

        buffer[index++] = (char) character;
     }
  }
  buffer[index] = 0; /* terminate string */

 /*
  * convert text number to integer
  */
  *integer = atoi(buffer);

 /*
  * return TRUE if not at end of file
  */
  return( !((index == 0) && (character == file_READERROR)) );
}

/******************************************************************************/

void NoLoad(char *err)
{
  char err_msg[128];

  Msgs_Lookup("err.noload", err_msg, 127);
  Error_Report(1,err_msg, err);

}

/******************************************************************************/

void NoMemLoad(void)
{
  char err_msg[128];

  Msgs_Lookup("err.nomemload", err_msg, 127);
  Error_Report(1,err_msg);

}

/******************************************************************************/

void NoSave(char *err)
{
  char err_msg[128];

  Msgs_Lookup("err.nosave", err_msg, 127);
  Error_Report(1,err_msg, err);

}

/******************************************************************************/

void InvalidFile(void)
{
  char err_msg[128];

  Msgs_Lookup("err.invfile", err_msg, 127);
  Error_Report(1,err_msg);

}

/******************************************************************************/

void TooShort(void)
{
  char err_msg[128];

  Msgs_Lookup("err.short", err_msg, 127);
  Error_Report(1,err_msg);

}

