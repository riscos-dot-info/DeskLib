#include "DeskLib:Sprite.h"

/*
 *  ATrans.h Header for Andrew's Transfer library
 *
 *  Provides simple routines for loading and saving in the desktop
 */

typedef BOOL (*ATrans_checkload)(int filetype, BOOL data_open);
/*
 * This is called when file is double clicked (data_open = TRUE)
 * or dragged onto the program (data_open = FALSE).
 * The function should return TRUE if the program is prepared
 * to load that particular file.
 */

typedef void (*ATrans_loadfile)(char *filename, int filetype, BOOL safe);
/*
 * This function is called to load a particular file.
 * safe = TRUE indicates that the file has come from a "safe" source.
 */

typedef void (*ATrans_savefile)(char *filename, int filetype, BOOL safe);
/*
 * This function is called to do the saveing of files.
 * safe = TRUE indicates that the file is "safe".
 */

typedef int (*ATrans_sendblock)(int *pointer, int size, int filetype);
/*
 * This function is called when a Ram Transfer is performed.
 * If this routine is not defined then ATrans_savefile will always be
 * used.
 * The routine should fill the buffer at pointer to a maximum of
 * size, and is called repeatedly until a non-full buffer is returned.
 * It should return the ammount of data placed in the buffer.
 */

typedef struct
{
  int type;
  int size;
  int ref;
  char file[256];
  BOOL dragging;
  ATrans_checkload checkload;
  ATrans_loadfile loadfile;
  ATrans_savefile savefile;
  ATrans_sendblock sendblock;
} ATrans_defblock;

/*
 * call this when a drag of the file icon in your savebox is detected.
 * area sets the spritearea that is used to obtain the icon sprite for
 * solid drags (if selected in CMOS RAM).
 * filesize is the expected size of the file for passing in the messages.
 * window and icon should describe the icon that is being dragged.
 * filename is the proposed filename (not necessarily a leafname).
 * area can be either a user sprite area, or 1 for the wimp sprite area,
 * or 0 for the system sprite area.
 */
void ATrans_startdrag(char *filename, int filetype, int filesize,
                      window_handle window, icon_handle icon, sprite_area area);


/*
 * call this to initialise the library, do it only once
 * if any of the routines are not present then set the argument to NULL
 * note: for saving, at least ATrans_savefile must be defined.
 *       for loading, both ATrans_loadfile and ATrans_checkload must be defined.
 */
void ATrans_Init(ATrans_checkload checkload,
                 ATrans_loadfile loadfile,
                 ATrans_savefile savefile,
                 ATrans_sendblock sendblock);

