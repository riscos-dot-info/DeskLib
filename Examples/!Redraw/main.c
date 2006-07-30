/////////////////////////////////////////////////////////////////////////////
// Simple Redraw example for GCC and DeskLib by Shaun Lindsley (fizgog)
// To test cover part of the window with other windows and
// click anywhere in window to see how the RISCOS draws
// enjoy

#include <stdio.h>
#include <string.h>

#include "main.h"

static BOOL quit = FALSE;

static window_handle window;

////////////////////////////////////////////////////////////////////////////////
// main

int main(void)
{
  Resource_Initialise("Redraw");
  Event_Initialise("Redraw");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);

  Template_Initialise();
  Template_LoadFile("Templates");

  window = Window_CreateAndShow("Main", template_TITLEDEFAULT, open_WHEREVER);

  Event_Claim(event_CLOSE, window, event_ANY, CloseWin, NULL);
  Event_Claim(event_REDRAW, window, event_ANY, Redraw, NULL);
  Event_Claim(event_OPEN, window, event_ANY, Handler_OpenWindow, NULL);
  Event_Claim(event_CLICK, window, event_ANY, ClickWin, NULL);

  while (!quit)
  {
    Event_Poll();
  }

  Template_ClearAll();
  return(0);
}

/////////////////////////////////////////////////////////////////////////////
// Close Window and shutdown application

static BOOL CloseWin(event_pollblock *event, void *ref)
{
  quit = TRUE;
  return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Click Window

static BOOL ClickWin(event_pollblock *event, void *ref)
{
  Update();

  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
// Redraw screen on poll event 1

static BOOL Redraw(event_pollblock *event, void *ref)
{
  window_redrawblock redraw;
  BOOL more;

  redraw.window = event->data.openblock.window;
  Wimp_RedrawWindow(&redraw, &more);

  while (more)
  {
    // Draw Code goes here
    Wimp_SetColour(rand() % 15);
    GFX_RectangleFill(redraw.cliprect.min.x,
                      redraw.cliprect.min.y,
                      redraw.cliprect.max.x,
                      redraw.cliprect.max.y);

    Wimp_GetRectangle(&redraw, &more);
  }

  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
//  Update Screen

static BOOL Update(void)
{
  BOOL more;

  window_info winfo;
  window_redrawblock redraw;

  Window_GetInfo(window, &winfo);

  //Coord_RectToWorkArea(&winfo.block.screenrect, (convert_block *)&winfo.block);
  redraw.window = winfo.window;
  redraw.rect = winfo.block.workarearect;

  Wimp_UpdateWindow(&redraw, &more);

  while (more)
  {
    // Draw Code goes here
    Wimp_SetColour(rand() % 15);
    GFX_RectangleFill(redraw.cliprect.min.x,
                      redraw.cliprect.min.y,
                      redraw.cliprect.max.x,
                      redraw.cliprect.max.y);

   Wimp_GetRectangle(&redraw, &more);
  }
  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////

