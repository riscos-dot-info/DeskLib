////////////////////////////////////////////////////////////////////////////////////////
// Font Demo example for GCC and DeskLib by Shaun Lindsley (fizgog)
// enjoy

#include <stdio.h>
#include <string.h>

#include "main.h"

static BOOL quit = FALSE;

static window_handle window;

static menu_ptr MainMenu;
static menu_ptr FontListMenu;
static menu_ptr FontWidthMenu;
static menu_ptr FontColMenu;

static font2_block *font2;

static char fontname[40];
static char fontwidth[4];
static int fontsize;
static int fontcol;

// Wimp Basic Colours used for font colourtrans
static const int std_colour[] =
{
  0xFFFFFF00, 0xDEDEDE00, 0xBDBDBD00, 0x9C9C9C00,
  0x73737300, 0x52525200, 0x31313100, 0x00000000,
  0x9C410000, 0x00EEEE00, 0x00CD0000, 0x0000DE00,
  0xBDEEEE00, 0x008B5200, 0x00BDFF00, 0xFFBD0000
};

////////////////////////////////////////////////////////////////////////////////////////
// main

int main(void)
{
  Resource_Initialise("FontDemo");

  Event_Initialise("FontDemo");
  EventMsg_Initialise();
  Screen_CacheModeInfo();
  EventMsg_Claim(message_MODECHANGE, event_ANY, Handler_ModeChange, NULL);

  Template_Initialise();
  Template_LoadFile("Templates");

  window = Window_CreateAndShow("Main", template_TITLEDEFAULT, open_WHEREVER);

  Event_Claim(event_CLOSE,  window, event_ANY, CloseWin, NULL);
  Event_Claim(event_REDRAW, window, event_ANY, Redraw, NULL);
  Event_Claim(event_OPEN,   window, event_ANY, Handler_OpenWindow, NULL);
  Event_Claim(event_CLICK,  window, event_ANY, WinClick, NULL);

  font2 = NULL;

  Setup_Menu();

  while (!quit)
  {
    Event_Poll();
  }

  Template_ClearAll();
  return(0);
}

////////////////////////////////////////////////////////////////////////////////////////
// Process Mouse Clicks in Main Window

static BOOL WinClick(event_pollblock *event, void *ref)
{
  if (event->data.mouse.button.data.menu)
  {
    Menu_Show(MainMenu, event->data.mouse.pos.x, event->data.mouse.pos.y);
    Event_Claim(event_MENU, event_ANY, event_ANY, MenuOption, ref);
  }

  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Decode Main Window Menu

static BOOL MenuOption(event_pollblock *event, void *ref)
{
  int *s;

  s = event->data.selection;

  // handle menu items
  switch (event->data.selection[0])
  {
    case MAIN_FONT_WIDTH : // Change Font Width
                           sprintf(fontwidth, "%s", Menu_GetText(FontWidthMenu, 0));
                           fontsize = atoi(fontwidth);
                           if (font2 != NULL) Font2_ReleaseFont(&font2);
                           Font2_ClaimFont(&font2, fontname, fontsize<<4, fontsize<<4);
                           Update();
                           break;

    case MAIN_FONT_COL   : // Change Colour of Font
                           Menu_SetFlags(FontColMenu, fontcol, FALSE, FALSE);
                           fontcol = event->data.selection[1];
                           Menu_SetFlags(FontColMenu, fontcol, TRUE, FALSE);
                           Update();
                           break;

    case MAIN_CLOSE      : // Close Application
                           CloseWin(event, ref);
                           break;

    default              : // Open Font List in a sub menu
                           FontListMenu = Menu_FontMenu(FALSE, fontname);
                           Wimp_CreateSubMenu(FontListMenu,
                                              event->data.message.data.menuwarn.openpos.x,
                                              event->data.message.data.menuwarn.openpos.y);
                           Event_Claim(event_MENU, event_ANY, event_ANY, FMenuChoice, ref);
                           break;
  }
  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Decode menu choice and convert to Correct Font

static BOOL FMenuChoice(event_pollblock *event, void *ref)
{
  int *s;
  char *name;
  font_defn fnt;

  s = event->data.selection;

  // We need selection[1] onwards not selection[0]
  name = Menu_FontMenuDecode(&s[1]);

  if (font2 != NULL) Font2_ReleaseFont(&font2);

  Font2_ClaimFont(&font2, name, fontsize<<4, fontsize<<4);

  // Use correct name for display not one with  \F, \E in
  Font_ReadDefn(font2->handle, &fnt);
  sprintf(fontname, "%s", fnt.name);
  Update();

  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Close Window and shutdown application

static BOOL CloseWin(event_pollblock *e, void *r)
{
  quit = TRUE;
  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Redraw screen on poll event 1

static BOOL Redraw(event_pollblock *e, void *r)
{
  window_redrawblock redraw;
  BOOL more;

  redraw.window = e->data.openblock.window;
  Wimp_RedrawWindow(&redraw, &more);

  while (more)
  {
    Display(&redraw);
    Wimp_GetRectangle(&redraw, &more);
  }

  return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////////////
// Update screen

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

////////////////////////////////////////////////////////////////////////////////////////
// Display font on screen used by Redraw and Update

static void Display(window_redrawblock *block)
{
  int x, y;

  // User Draw so we need to clear the screen
  Wimp_SetColour(colour_WHITE);
  GFX_RectangleFill(block->rect.min.x, block->rect.min.y, block->rect.max.x, block->rect.max.y);

  // Set Font Colour using colourtrans
  ColourTrans_SetFontColours(font2->handle, std_colour[0], std_colour[fontcol], 14);

  // Draw font to screen at x,y
  x = block->rect.min.x + 50;
  y = block->rect.min.y + 50;

  Font_Paint3(font2->handle, fontname, font_plot_OSCOORS, x, y, NULL, NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////////////
// Load messages file and setup menus

static void Setup_Menu(void)
{
  char title[20], menu[100];

  Msgs_LoadFile("Messages");

  Msgs_Lookup("Main.Title", title, 20);
  Msgs_Lookup("Main.Menu",  menu, 100);
  MainMenu = Menu_New(title, menu);

  // Font List can change in size so we add a Menu_Warn message
  // to MainMenu for FontListMenu, make sure '>' is on Font List in messages file
  sprintf(fontname,"Trinity.Medium");

  Menu_Warn(MainMenu, MAIN_FONT_LIST, TRUE, MenuOption, FontListMenu);

  // Attach Font Width Sub Menu and make writable with characters 0 - 9
  fontsize = 12;

  sprintf(fontwidth, "%d", fontsize);
  FontWidthMenu = Menu_New("Width", fontwidth);
  Menu_MakeWritable(FontWidthMenu, 0, fontwidth, 4, "A0-9");
  Menu_AddSubMenu(MainMenu, MAIN_FONT_WIDTH, FontWidthMenu);

  // Attach Font Colour Sub Menu
  fontcol = 7;

  FontColMenu = (menu_ptr) ColourMenu_Create("Colour");
  Menu_AddSubMenu(MainMenu, MAIN_FONT_COL, FontColMenu);
  Menu_SetFlags(FontColMenu, fontcol, TRUE, FALSE);

  return;
}
////////////////////////////////////////////////////////////////////////////////////////
