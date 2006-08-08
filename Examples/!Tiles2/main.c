/////////////////////////////////////////////////////////////////////////////
// Simple Tiles2 example for GCC and DeskLib by Shaun Lindsley (fizgog)
// To test click and drag with select to draw green tiles and click
// and drag with adjust to draw with white tiles
// enjoy

#include <stdio.h>
#include <string.h>

#include "main.h"

#define TILESIZE 50       // 25 x 25 doubled

static BOOL quit = FALSE;
static BOOL drag = FALSE;

static window_handle window;
static window_handle debug;
static sprite_area tileset;

unsigned char TileMap[40][20];

////////////////////////////////////////////////////////////////////////////////
// main

int main(void)
{
  Resource_Initialise("Tiles2");
  Event_Initialise("Tiles2");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);

  Template_Initialise();
  Template_LoadFile("Templates");

  tileset = Sprite_LoadFile("<Tiles2$Dir>.Sprites");

  window = Window_CreateAndShow("Main", template_TITLEDEFAULT, open_WHEREVER);
  Event_Claim(event_CLOSE, event_ANY, event_ANY, CloseWin, NULL);
  Event_Claim(event_REDRAW, window, event_ANY, Redraw, NULL);
  Event_Claim(event_OPEN, event_ANY, event_ANY, Handler_OpenWindow, NULL);
  Event_Claim(event_CLICK, window, event_ANY, ClickWin, NULL);
  Event_Claim(event_NULL, event_ANY, event_ANY, NullEvent, NULL);

  memset(TileMap, 0, sizeof(TileMap));

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
// Null Event - used for dragging

static BOOL NullEvent(event_pollblock *event, void *ref)
{
  if (drag)
    DrawTile();

  return(TRUE);
}

/////////////////////////////////////////////////////////////////////////////
// Click Window

static BOOL ClickWin(event_pollblock *event, void *ref)
{
  switch (event->data.mouse.button.value)
  {
    case button_SELECT     :
    case button_ADJUST     : DrawTile();
                             break;

    case button_DRAGSELECT :
    case button_DRAGADJUST : drag = TRUE;
                             break;

  }

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

static BOOL Update(int tilex, int tiley)
{
  BOOL more;

  char tilespr[2];

  wimp_point pos;
  convert_block cnv;

  window_info winfo;
  window_redrawblock redraw;

  Window_GetCoords(window, &cnv);

  Window_GetInfo3(window, &winfo);

  redraw.window = winfo.window;
  redraw.rect = winfo.block.workarearect;

  pos.x = (tilex*TILESIZE);
  pos.y = (tiley * -TILESIZE) - TILESIZE;

  Wimp_UpdateWindow(&redraw, &more);

  while (more)
  {
    sprintf(tilespr, "%02d", TileMap[tilex][tiley]);
    Sprite_WimpPlot(tileset, tilespr, &pos, &cnv, 0);

    Wimp_GetRectangle(&redraw, &more);
  }
  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
//

static void Display(window_redrawblock *block)
{
  wimp_point pos;
  convert_block cnv;
  window_info winfo;

  Window_GetInfo3(window, &winfo);

  int x, y;
  int x1, y1, x2, y2;
  char tilespr[2];

  pos.x = winfo.block.workarearect.min.x;
  pos.y = winfo.block.workarearect.max.y;

  Window_GetCoords(window, &cnv);

  for (y = 0; y < 20; y++)
  {
    pos.x = winfo.block.workarearect.min.x;
    pos.y-=TILESIZE;
    for (x = 0; x < 40; x++)
    {
      sprintf(tilespr, "%02d", TileMap[x][y]);
      Sprite_WimpPlot(tileset, tilespr, &pos, &cnv, 0);
      pos.x+=TILESIZE;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Draw a single tile at mouse coords

static void DrawTile(void)
{
  int mousex, mousey;
  int vminx, vmaxy;
  int scrollx, scrolly;
  int tilex, tiley;

  window_info winfo;
  Window_GetInfo3(window, &winfo);

  mouse_block mblk;
  Wimp_GetPointerInfo(&mblk);

  if (mblk.button.value == button_SELECT || mblk.button.value == button_ADJUST)
  {
    // Mouse x, y position
    mousex = mblk.pos.x;
    mousey = mblk.pos.y;

    // Visible Window top left position
    vminx = winfo.block.screenrect.min.x;
    vmaxy = winfo.block.screenrect.max.y;

    // Scroll x, y
    scrollx = winfo.block.scroll.x;
    scrolly = winfo.block.scroll.y;

    // Calculate tile x, y position
    tilex = (mousex + (scrollx - vminx)) / TILESIZE;
    tiley = ((mousey + (scrolly - vmaxy)) / TILESIZE) * -1;

    // Only draw tile 1 if select used and tile not 1
    if (mblk.button.value == button_SELECT && TileMap[tilex][tiley] != 1)
    {
      TileMap[tilex][tiley] = 1;
      Update(tilex, tiley);
    }

    // Only draw tile 0 if adjust used and tile not 0
    else if(mblk.button.value == button_ADJUST && TileMap[tilex][tiley] != 0)
    {
      TileMap[tilex][tiley] = 0;
      Update(tilex, tiley);
    }
  }
  else drag = FALSE;
}
////////////////////////////////////////////////////////////////////////////////
