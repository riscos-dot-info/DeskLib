/*
 *  ATrans.c Andrew's Transfer library
 *
 *  Provides very simple routines for loading and saving in the desktop
 */

#include "DeskLib:Event.h"
#include "DeskLib:EventMsg.h"
#include "DeskLib:Msgs.h"
#include "DeskLib:WimpSWIs.h"
#include "DeskLib:Error.h"
#include "DeskLib:StringCR.h"
#include "DeskLib:KernelSWIs.h"
#include "DeskLib:DragASpr.h"
#include "DeskLib:SWI.h"
#include "ATrans.h"
/* #include "MyHeader.h" */
#include "MySwis.h"
#include "kernel.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static ATrans_defblock defblock;
static BOOL initialised = FALSE;

/* responds to messages */
BOOL ATrans_message(event_pollblock *event, void *reference)
{
  int *buffer;
  int size;
  event_type mess;
  BOOL truefalse;
                                                 
  switch(event->data.message.header.action)
  {
     case message_DATASAVE:
     if((event->data.message.header.sender != event_taskhandle) &&
     ((defblock.checkload)(event->data.message.data.datasave.filetype,FALSE))){
        strcpy(event->data.message.data.datasave.leafname, "<Wimp$Scrap>");
        event->data.message.data.datasave.estsize = -1;
        event->data.message.header.yourref = event->data.message.header.myref;
        event->data.message.header.action = message_DATASAVEACK;
        event->data.message.header.size = 60;
        Wimp_SendMessage(event_USERMESSAGE,
                         &event->data.message,
                         event->data.message.header.sender,
                         0);
     }
     return(TRUE);
     break;

     case message_DATASAVEACK:
     if( event->data.message.header.yourref == defblock.ref ){
        event->data.message.data.datasaveack.estsize = defblock.size;
        event->data.message.header.yourref = event->data.message.header.myref;
        event->data.message.header.action = message_DATALOAD;
        Wimp_SendMessage(event_USERMESSAGERECORDED,
                         &event->data.message,
                         event->data.message.data.datasaveack.window,
                         event->data.message.data.datasaveack.icon);
        (defblock.savefile)(event->data.message.data.datasaveack.filename,
                            defblock.type,
                            (event->data.message.data.datasaveack.estsize!=-1));
        defblock.ref = event->data.message.header.myref;
        Wimp_CreateMenu((menu_ptr) -1, 0, 0); /* remove any menu from screen */
      }
      return(TRUE);
      break;

      case message_DATALOAD:
      if( (event->data.message.header.sender != event_taskhandle) &&
      ((defblock.checkload)(event->data.message.data.dataload.filetype,FALSE))){
         event->data.message.header.yourref = event->data.message.header.myref;
         event->data.message.header.action = message_DATALOADACK;
         Wimp_SendMessage(event_USERMESSAGE,
                          &event->data.message,
                          event->data.message.header.sender,
                          0);
      if( strcmp(event->data.message.data.dataload.filename,"<Wimp$Scrap>") != 0)
          truefalse = TRUE;
      else
          truefalse = FALSE;
         (defblock.loadfile)(event->data.message.data.dataload.filename,
                             event->data.message.data.dataload.filetype,
                             truefalse);
      if( truefalse == FALSE )
         OSCLI("Remove <Wimp$Scrap>");                       
      }
      return(TRUE);
      break;

      case message_DATAOPEN:
      if((defblock.checkload)(event->data.message.data.datasave.filetype,TRUE)){
         event->data.message.header.yourref = event->data.message.header.myref;
         event->data.message.header.action = message_DATALOADACK;
         Wimp_SendMessage(event_USERMESSAGE,
                          &event->data.message,
                          event->data.message.header.sender,
                          0);
         (defblock.loadfile)(event->data.message.data.dataload.filename,
                             event->data.message.data.dataload.filetype,
                             TRUE);
      }
      return(TRUE);
      break;

      case message_RAMFETCH:
      buffer = malloc(event->data.message.data.ramfetch.buffsize);
      size = (defblock.sendblock)(buffer,
                                  event->data.message.data.ramfetch.buffsize,
                                  defblock.type);
      Wimp_TransferBlock(event_taskhandle,
                         buffer,
                         event->data.message.header.sender,
                         event->data.message.data.ramfetch.buffer,
                         size);
      free(buffer);
      if(size < event->data.message.data.ramfetch.buffsize)
         mess = event_USERMESSAGE;
      else
         mess = event_USERMESSAGERECORDED;
      event->data.message.header.yourref = event->data.message.header.myref;
      event->data.message.header.action = message_RAMTRANSMIT;
      event->data.message.data.ramtransmit.byteswritten = size;
      Wimp_SendMessage(mess,
                       &event->data.message,
                       event->data.message.header.sender,
                       0);
      return(TRUE);
      break;
  }

  return(FALSE);
}

/* acknowledges messages */
BOOL ATrans_acknowledge(event_pollblock *event, void *reference)
{
  char err_msg[61];
  
  switch(event->data.message.header.action)
  {
     case message_DATALOAD:
     if(defblock.savefile == NULL)
        return(TRUE);
     if( event->data.message.header.yourref == defblock.ref){
        if(strcmp(event->data.message.data.dataload.filename,"<Wimp$Scrap>")==0)
           OSCLI("Remove <Wimp$Scrap>");
        Msgs_Lookup("msg.rdied", err_msg, 60);
        Error_Report(1, err_msg);
     }
     return(TRUE);
     break;

     case message_RAMTRANSMIT:
     if(defblock.sendblock == NULL)
        return(TRUE);
     Msgs_Lookup("msg.rdied", err_msg, 60);
     Error_Report(1, err_msg);
     return(TRUE);
     break;
   }

   return(FALSE);
}

/* called when drag is terminated to save file */
BOOL ATrans_savedrag(event_pollblock *event, void *reference)
{
  int solid;
  mouse_block    ptr;
  /* icon_block     iconblock; */

  if(defblock.dragging == FALSE)
     return(FALSE);

  OS_Byte(osbyte_READCMOSRAM, 28, 0, NULL, &solid);

  if((solid & 2) == 2)
     DragASprite_Stop();
  else
     Wimp_DragBox((drag_block *) -1);

  defblock.dragging = FALSE;

  Wimp_GetPointerInfo( &ptr);

  event->data.message.header.size            = 80;
  event->data.message.header.yourref         = 0;
  event->data.message.header.action          = message_DATASAVE;
  event->data.message.data.datasave.window   = ptr.window;
  event->data.message.data.datasave.icon     = ptr.icon;
  event->data.message.data.datasave.pos      = ptr.pos;
  event->data.message.data.datasave.estsize  = defblock.size;
  event->data.message.data.datasave.filetype = defblock.type;

  strcpy(event->data.message.data.datasave.leafname, LeafName(defblock.file));

  Wimp_SendMessage(event_SENDWANTACK,
                   &event->data.message,
                   (message_destinee) ptr.window,
                   ptr.icon);

  defblock.ref = event->data.message.header.myref;

  return(TRUE);
}

/* call this when the file icon is dragged */
void ATrans_startdrag(char *filename, int filetype, int filesize,
                      window_handle window, icon_handle icon, sprite_area area)
{
  char spritename[15];
  char number[5];
  drag_block     draginfo;
  int solid;
  wimp_rect bound;
  int block[20];
  int winminx, winminy, winmaxx, winmaxy;
  _kernel_swi_regs r;

  OS_Byte(osbyte_READCMOSRAM, 28, 0, NULL, &solid);

  block[0] = window;
  r.r[1] = (int) block; 
  _kernel_swi( SWI_XOS_Bit | SWI_Wimp_GetWindowState, &r, &r );
  winminx = block[1];
  winminy = block[2];
  winmaxx = block[3];
  winmaxy = block[4];

  block[0] = window;
  block[1] = icon;
  r.r[1] = (int) block;
  _kernel_swi( SWI_XOS_Bit | SWI_Wimp_GetIconState, &r, &r );

  bound.min.x = winminx + block[2];
  bound.max.x = winminx + block[4];
  bound.min.y = winminy + block[3] + (winmaxy - winminy);
  bound.max.y = winminy + block[5] + (winmaxy - winminy);

  if((solid & 2) == 2){
     strcpy(spritename, "file_");
     if(filetype < 0x100)
        strcat(spritename, "0");
     if(filetype < 0x10)
        strcat(spritename, "0");
     sprintf(number,"%x", filetype);
     strcat(spritename, number);

     if(area == (sprite_area) 1)
        DragASprite_Start(0xC5, (void *) 1, spritename, &bound, (wimp_rect *) 0);
     if(area == (sprite_area) 0)
        DragASprite_Start(0xC5, (void *) 0, spritename, &bound, (wimp_rect *) 0);
     if((area != (sprite_area) 1) && (area != (sprite_area) 0))
        DragASprite_Start(0xC5, area, spritename, &bound, (wimp_rect *) 0);
  }
  else{

  draginfo.window            = 0;
  draginfo.type              = drag_FIXEDBOX;
  draginfo.screenrect        = bound;
  draginfo.parent.min.x      = 0;
  draginfo.parent.min.y      = 0;
  draginfo.parent.max.x      = 10000;
  draginfo.parent.max.y      = 10000;
  Wimp_DragBox( &draginfo);
  }

  defblock.type = filetype;
  strcpy(defblock.file, filename);
  defblock.size = filesize;
  defblock.dragging = TRUE;
}

/* initialised event handlers and stores addresses of callback routines */
void ATrans_Init(ATrans_checkload checkload,
                 ATrans_loadfile loadfile,
                 ATrans_savefile savefile,
                 ATrans_sendblock sendblock)
{

  if(initialised == TRUE){
     Error_ReportInternal(0,"ATrans can only be initialised once at present");
     return;
  }

  defblock.checkload = checkload;
  defblock.loadfile = loadfile;
  defblock.savefile = savefile;
  defblock.sendblock = sendblock;

  Event_Claim(event_USERDRAG, event_ANY, event_ANY, ATrans_savedrag, NULL);

  if(defblock.checkload != NULL)
     EventMsg_Claim(message_DATASAVE, event_ANY, ATrans_message, NULL);
  if(defblock.savefile != NULL)
     EventMsg_Claim(message_DATASAVEACK, event_ANY, ATrans_message, NULL);
  if((defblock.loadfile != NULL) && (defblock.checkload != NULL))
     EventMsg_Claim(message_DATALOAD, event_ANY, ATrans_message, NULL);
  if((defblock.loadfile != NULL) && (defblock.checkload != NULL))
     EventMsg_Claim(message_DATAOPEN, event_ANY, ATrans_message, NULL);
  if(defblock.sendblock != NULL)
     EventMsg_Claim(message_RAMFETCH, event_ANY, ATrans_message, NULL);

  if((defblock.savefile != NULL) || (defblock.sendblock != NULL))
     Event_Claim(event_USERMESSAGEACK, event_ANY, event_ANY, ATrans_acknowledge, NULL);
  
}


        


