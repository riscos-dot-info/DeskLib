/////////////////////////////////////////////////////////////////////////////
// Simple Tiles example for GCC and DeskLib by Shaun Lindsley (fizgog)
// To test click a tile to toggle on or off
// enjoy

#include <stdio.h>
#include <string.h>

#include "main.h"

#define TILESIZE 50       // 25 x 25 doubled

static BOOL quit = FALSE;

static window_handle window;
static window_handle debug;
static sprite_area tileset;

unsigned char TileMap[40][20];

static void Display(window_redrawblock *block);


////////////////////////////////////////////////////////////////////////////////
// main

int main(void)
{
  Resource_Initialise("Tiles");
  Event_Initialise("Tiles");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);

  Template_Initialise();
  Template_LoadFile("Templates");

  tileset = Sprite_LoadFile("<Tiles$Dir>.Sprites");

  window = Window_CreateAndShow("Main", template_TITLEDEFAULT, open_WHEREVER);
  Event_Claim(event_CLOSE, event_ANY, event_ANY, CloseWin, NULL);
  Event_Claim(event_REDRAW, window, event_ANY, Redraw, NULL);
  Event_Claim(event_OPEN, event_ANY, event_ANY, Handler_OpenWindow, NULL);
  Event_Claim(event_CLICK, window, event_ANY, ClickWin, NULL);

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
// Click Window

static BOOL ClickWin(event_pollblock *event, void *ref)
{
  int mousex, mousey;
  int vminx, vmaxy;
  int scrollx, scrolly;
  int tilex, tiley;

  window_info winfo;
  Window_GetInfo3(window, &winfo);

  // Mouse x, y position
  mousex = event->data.mouse.pos.x;
  mousey = event->data.mouse.pos.y;

  // Visible Window top left position
  vminx = winfo.block.screenrect.min.x;
  vmaxy = winfo.block.screenrect.max.y;

  // Scroll x, y
  scrollx = winfo.block.scroll.x;
  scrolly = winfo.block.scroll.y;

  // Calculate tile x, y position
  tilex = (mousex + (scrollx - vminx)) / TILESIZE;
  tiley = ((mousey + (scrolly - vmaxy)) / TILESIZE) * -1;

  // Toggle tile on or off
  TileMap[tilex][tiley] = !TileMap[tilex][tiley];

  Update(tilex, tiley);

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

static void Display(window_redrawblock *block)
{
  wimp_point pos;
  convert_block cnv;
  window_info winfo;

  Window_GetInfo3(window, &winfo);

  int x, y;
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
