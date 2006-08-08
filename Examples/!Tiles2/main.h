#ifndef MAIN_H
#define MAIN_H

#include "DeskLib:WimpSWIs.h"          /* Low-level WIMP commands         */
#include "DeskLib:Window.h"            /* Window handling automation      */
#include "DeskLib:Event.h"             /* Event despatcher                */
#include "DeskLib:EventMsg.h"          /* Wimp Message event dispatcher   */
#include "DeskLib:GFX.h"               /* Graphics routines (GFX_Wait)    */
#include "DeskLib:Handler.h"           /* Default/example event handlers  */
#include "DeskLib:Resource.h"          /* Handles finding resource files  */
#include "DeskLib:Screen.h"            /* Getting screen size info, etc   */
#include "DeskLib:Template.h"          /* Template loading and caching    */

static BOOL NullEvent(event_pollblock *event, void *ref);
static BOOL CloseWin(event_pollblock *event, void *ref);
static BOOL ClickWin(event_pollblock *event, void *ref);
static BOOL Redraw(event_pollblock *event, void *ref);
static BOOL Update(int tilex, int tiley);

static void Display(window_redrawblock *block);
static void DrawTile(void);

#endif
