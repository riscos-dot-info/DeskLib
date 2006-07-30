/////////////////////////////////////////////////////////////////////////////
// Simple Timer example for GCC and DeskLib by Shaun Lindsley (fizgog)
// enjoy

#include <stdio.h>
#include <string.h>

#include "main.h"

static BOOL quit = FALSE;
static int counter = 0;
static window_handle window;

////////////////////////////////////////////////////////////////////////////////
// main

int main(void)
{
  Resource_Initialise("Timer");
  Event_Initialise("Timer");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);

  Template_Initialise();
  Template_LoadFile("Templates");

  window = Window_CreateAndShow("Main", template_TITLEDEFAULT, open_WHEREVER);
  // Claim NULL event and call Timer function - has to be event_ANY
  // and not specific window
  Event_Claim(event_NULL, event_ANY, NULL, Timer, NULL);

  Event_Claim(event_CLOSE, window, event_ANY, CloseWin, NULL);
  Event_Claim(event_REDRAW, window, event_ANY, Redraw, NULL);
  Event_Claim(event_OPEN, window, event_ANY, Handler_OpenWindow, NULL);

  while (!quit)
  {
    Event_PollIdle(100);  // 1000 milliseconds
  }

  Template_ClearAll();
  return(0);
}

/////////////////////////////////////////////////////////////////////////////
// Increment counter and update screen display

static BOOL Timer(event_pollblock *event, void *ref)
{
  counter += 1;
  Update();
}

/////////////////////////////////////////////////////////////////////////////
// Close Window and shutdown application

static BOOL CloseWin(event_pollblock *event, void *ref)
{
  quit = TRUE;
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
    Display(&redraw);
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

  Coord_RectToWorkArea(&winfo.block.screenrect, (convert_block *)&winfo.block);
  redraw.window = winfo.window;
  redraw.rect = winfo.block.screenrect;

  Wimp_UpdateWindow(&redraw, &more);

  while (more)
  {
    Display(&redraw);
    Wimp_GetRectangle(&redraw, &more);
  }
  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
// display counter on screen used by Redraw and Update

static void Display(window_redrawblock *block)
{
  int x, y;
  char buffer[80];

  sprintf(buffer, "Counter = %d", counter);

  // Clear background
  Wimp_SetColour(colour_WHITE);
  GFX_RectangleFill(block->rect.min.x, block->rect.min.y, block->rect.max.x, block->rect.max.y);

  Wimp_SetColour(colour_BLACK);
  x = block->rect.min.x + 50;
  y = block->rect.max.y - 50;

  GFX_Move(x, y);
  GFX_Write0(buffer);
}
////////////////////////////////////////////////////////////////////////////////

