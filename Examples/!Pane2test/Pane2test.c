/************************************************************************
 *                                                                      *
 *        File: Pane2test.c                                             *
 *                                                                      *
 *     Purpose: Core of program.                                        *
 *              Initialises and loads resources.                        *
 *              Handles IconBar icon and menu.                          *
 *                                                                      *
 ************************************************************************/

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Core.h"              /* usefull core functions          */
#include "DeskLib:Dialog.h"            /* High-level dialogue windows     */
#include "DeskLib:Error.h"             /* Error despatcher                */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:File.h"              /* Low level file handling         */
#include "DeskLib:GFX.h"               /* Graphics routines (GFX_Wait)    */
#include "DeskLib:Handler.h"           /* Default/example event handlers  */
#include "DeskLib:Hourglass.h"         /* Hourglass module interfaces     */
#include "DeskLib:Icon.h"              /* Icon handling automation        */
#include "DeskLib:Menu.h"              /* Menu create & show support      */
#include "DeskLib:Msgs.h"              /* Message translation code        */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Screen.h"            /* Getting screen size info, etc   */
#include "DeskLib:Sound.h"             /* Sound System control            */
#include "DeskLib:Template.h"          /* Template loading and caching    */
#include "DeskLib:Slider.h"            /* "Easy" slider handling          */
#include "DeskLib:Kbd.h"               /* Read hotkeys                    */
#include "DeskLib:Time.h"              /* "TIME"                          */
#include "DeskLib:KernelSWIs.h"        /* OS_Byte                         */
#include "DeskLib:Dialog2.h"           /* windows on menus made easy      */

#include "PaneTests.h"

#include "kernel.h"
#include "MySwis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static BOOL quit = FALSE;              /* Set to TRUE to quit the app.    */

extern sprite_area usericonsprites = NULL;

static menu_ptr mainmenu = NULL;

/* Defines for the positions at which items appear on the menus. */
#define mainmenu_INFO           0
#define mainmenu_QUIT           5

#define idle_TIME               40

/* Wimp_PollIdle delay time */ 
extern unsigned int idletime = idle_TIME;

/* startup command for the Desktop Boot file */
static char startup_command[256];

#define program_NAME "Pane2test"

void setup_menus(void);
void setup_templates(void);
void build_startupcommand(void);
static BOOL save_desktop(event_pollblock *event, void *reference);

/******************************************************************************/

static BOOL CloseWindow(event_pollblock *event, void *reference)
/*  Handler attached to all closewindow events.
 * fall through causes BEEP when window is closed which indicates that I have
 * not installed a close window hander for that window
 */
{
  Sound_SysBeep();

  if(event->type == event_CLOSE){

     Sound_SysBeep();
     Window_Delete(event->data.words[0]);
     
  } 
  return(TRUE);
}

/******************************************************************************/

static BOOL Menuwarn_proginfo(event_pollblock *event, void *reference)
{
 /*
  * called to display "about this program" window
  */

  if((menu_currentopen != mainmenu) ||
     (event->data.message.data.menuwarn.selection[0] != mainmenu_INFO))
     return(FALSE);

  Dialog2_OpenDialogMenuLeaf( event, (dialog2_block *)reference);

  return(TRUE);
}

/******************************************************************************/

static void Popup_proginfo( dialog2_block *dialog2)
{
 /*
  * called when "about this program" window is displayed to set the correct
  * version text
  */
  char version[50];

  Msgs_Lookup("info.version", version, 50);
  Icon_SetText(dialog2->window, 4, version);
}

/******************************************************************************
 *                              Click handlers                                *
 ******************************************************************************/

static BOOL Click_baricon(event_pollblock *event, void *reference)
{
 /*
  * called when mouse click on iconbar icon is made.
  * opens "Create new image" with the select button.
  * opens iconbar menu with the menu button.
  * opens debug window with the adjust button
  */

  if(event->data.mouse.button.data.menu)
     Menu_Show(mainmenu, event->data.mouse.pos.x,-1);

  return(TRUE);
}

/******************************************************************************
 *                           menu choice handlers                             *
 ******************************************************************************/

static BOOL MenuChoice_mainmenu(event_pollblock *event, void *reference)
{
 /*
  * called when menu selection of the iconbar menu is made
  */
  mouse_block ptr;

  if (menu_currentopen != mainmenu)        /* Only respond to the mainmenu  */
    return(FALSE);

  switch(event->data.selection[0])
  {

    case mainmenu_QUIT:
      quit = TRUE;
      break;

    default:
      if( (event->data.selection[0] > mainmenu_INFO) &&
          (event->data.selection[0] < mainmenu_QUIT) )
          PaneTests_MenuSelection(event->data.selection[0]);
      break;

  }

  Wimp_GetPointerInfo(&ptr);               /* If ADJUST was used to select, */
  if (ptr.button.data.adjust)              /* then we keep the menu open.   */
    Menu_ShowLast();

  return(TRUE);
}

/******************************************************************************
 *                                    main()                                  *
 ******************************************************************************/

int main(int argc, char *argv[])
{
  char appname[64];

  Resource_Initialise(program_NAME);   /* resources in <program_NAME$Dir> */

 /*
  * create startup command for Desktop Boot file
  */
  build_startupcommand();

 /*
  * load messages file and look up application name
  */
  Msgs_LoadFile("messages");
  Msgs_Lookup("app.name", appname, 64);

 /*
  * initialise event and messages systems
  */
  Event_Initialise(appname);
  EventMsg_Initialise();

 /*
  * initialise shifting heap memory manager
  */
/* don't need it yet .....
  flex_init();
  _kernel_register_slotextend(flex_budge);
*/

 /*
  * store current screen description and install event hander to keep it updated
  */
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);

 /*
  * install default event handlers
  */
  Event_Claim(event_REDRAW, event_ANY, event_ANY, Handler_NullRedraw, NULL);
  Event_Claim(event_OPEN, event_ANY, event_ANY, Handler_OpenWindow, NULL);

 /*
  * install savedesktop file message handler to insert startup command into
  * Desktop Boot file
  */
  EventMsg_Claim(message_SAVEDESK, event_ANY, save_desktop, NULL);

 /*
  * load window definitions
  */
  setup_templates();

 /*
  * event handler to close windows and beep if I have messed the code up
  */
  Event_Claim(event_CLOSE, event_ANY, event_ANY, CloseWindow, NULL); 

 /*
  * build menus
  */
  setup_menus();

 /*
  * initialise the other files
  */
  PaneTests_Init();

  /*  Main event handling loop.
   */
  while (!quit){

    Wimp_Poll(event_mask, &event_lastevent);
    /* Wimp_PollIdle(event_mask, &event_lastevent, (Time_Monotonic() + idletime)); */

    Event_Process(&event_lastevent);
  }

  Template_ClearAll();

  return(0);
}

/******************************************************************************
 *                              setup routines                                *
 ******************************************************************************/

void setup_menus(void)
{
 /*
  * build iconbar menu
  */
  char menudescription[260];
  char menutitle[15];
  dialog2_block *proginfo;

  Msgs_Lookup("title.main", menutitle, 15);
  Msgs_Lookup("menu.main", menudescription, 260);
  mainmenu  = Menu_New(menutitle, menudescription);
  proginfo = Dialog2_CreateDialogBlock( "ProgInfo", 42, 42, Popup_proginfo, NULL, NULL);
  Menu_Warn(mainmenu, mainmenu_INFO, TRUE, Menuwarn_proginfo, proginfo);
  
  Event_Claim(event_MENU,  event_ANY, event_ANY, MenuChoice_mainmenu, NULL);

}

/******************************************************************************/

void setup_templates(void)
{
 /*
  * load window and icon definition files
  * install icon on icon bar
  */
  int threeD;
  char spritefile[50];
  icon_handle baricon;

  Template_Initialise();
  /* Template_UseOutlineFonts(); */

  OS_Byte(osbyte_READCMOSRAM, 140, 0, NULL, &threeD);

  if((threeD & 1) == 1)
     Template_LoadFile("Template3D");
  else
     Template_LoadFile("Templates");

  strncpy(spritefile, resource_pathname, 39);
  if(screen_eig.y == screen_eig.x)
     strcat(spritefile, "Sprites22");
  else
     strcat(spritefile, "Sprites");
  usericonsprites = Sprite_LoadFile(spritefile);
  
  baricon = Icon_BarIconUser(program_NAME,iconbar_RIGHT,(unsigned int *) usericonsprites);

  Template_UseSpriteArea(usericonsprites);

  Event_Claim(event_CLICK, -2, baricon, Click_baricon, NULL);
}

/******************************************************************************
 *                              misc routines                                 *
 ******************************************************************************/

/******************************************************************************/

void build_startupcommand(void)
{
 /*
  * build startup command for DesktopBoot file from resource pathname
  */
  char resourcepath[40];
  _kernel_swi_regs r;

  strcpy(resourcepath, "/");
  strncat(resourcepath, resource_pathname, 38);

 /*
  * remove "." from end of path
  */
  resourcepath[strlen(resourcepath) - 1] = 0;

  r.r[0] = (int) resourcepath;
  r.r[1] = (int) startup_command;
  r.r[2] = 256;
  _kernel_swi( SWI_XOS_Bit | SWI_OS_GSTrans , &r, &r );

}

/******************************************************************************/

static BOOL save_desktop(event_pollblock *event, void *reference)
{
 /*
  * save startup command to DesktopBoot file
  */
  file_handle channel;
  BOOL error;
  message_block message;

  channel = (file_handle) event->data.message.data.words[0];

  error = Error_Check(File_WriteBytes(channel, startup_command, strlen(startup_command)));

  if(!error){
     if(Error_Check(File_WriteByte(channel, '\n')))
        error = TRUE;
  }

  if(error){ /* acknowledge message */

     message.header.yourref = message.header.myref;
     Wimp_SendMessage(event_USERMESSAGEACK, &message, message.header.sender, NULL);
     
  } 

  return(TRUE);
}

