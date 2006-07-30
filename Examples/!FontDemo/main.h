#ifndef MAIN_H
#define MAIN_H

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Dialog.h"            /* High-level dialogue windows     */
#include "DeskLib:DragASpr.h"          /* DragASprite veneers             */
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
#include "DeskLib:Pane.h"              /* Pane window handling            */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Screen.h"            /* Getting screen size info, etc   */
#include "DeskLib:Sound.h"             /* Sound System control            */
#include "DeskLib:Template.h"          /* Template loading and caching    */
#include "DeskLib:Sprite.h"
#include "DeskLib:Font.h"
#include "DeskLib:Font2.h"
#include "Desklib:Colourtran.h"

#define MAIN_FONT_LIST  0
#define MAIN_FONT_WIDTH 1
#define MAIN_FONT_COL   2
#define MAIN_CLOSE      3

static BOOL CloseWin(event_pollblock *event, void *ref);
static BOOL Redraw(event_pollblock *event, void *ref);
static BOOL WinClick(event_pollblock *event, void *ref);
static BOOL MenuOption(event_pollblock *event, void *ref);
static BOOL FMenuChoice(event_pollblock *event, void *ref);
static BOOL Update(void);
static void Display(window_redrawblock *block);
static void Setup_Menu(void);

#endif
