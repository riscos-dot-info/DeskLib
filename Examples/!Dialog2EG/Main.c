#include "DeskLib:Dialog2.h"
#include "DeskLib:Event.h"
#include "DeskLib:Resource.h"
#include "DeskLib:Template.h"
#include "DeskLib:Icon.h"
#include "DeskLib:Error.h"
#include "DeskLib:Menu2.h"
#include "DeskLib:Debug.h"



typedef struct	{

	struct	{
		dialog2_block	*one, 
				*two, 
				*info;
		}
		dialogs;
	
	icon_handle	baricon;
	menu2_handle	iconbarmenu;
	}
	app_block;
	/* This holds all our application's global data.	*/






static menu2_handle	IconbarSubmenuHandler( int item, event_pollblock *event, void *reference)
/* Called when a submenu is needed	*/
{
app_block	*app = (app_block *) reference;

UNUSED( item);	/* It must be item 0	*/

Dialog2_OpenDialogMenuLeaf( event, app->dialogs.info);
	/* Could check which menuitem this menuwarn is for, and open	*/
	/* the apropriate dialog2 box.					*/

Debug_Printf( "Just opened the info window...\n");

return NULL;	/* We don't want a menu2_handle to be opened as	*/
		/* a submenu, so return NULL.			*/
}




static void	IconbarMenuChoiceHandler( int item, void *reference)
/* Called when a menu-choice is made.	*/
{
app_block	*app = (app_block *) reference;

Debug_Printf( "Iconbar menu item '%i' has just been chosen\n", item);

     if ( item==1)	Dialog2_OpenDialogStatic(	app->dialogs.one, open_WHEREVER);
else if ( item==2)	Dialog2_OpenDialogMenu(		app->dialogs.one, open_CENTERED);
else if ( item==3)	Dialog2_OpenDialogStatic(	app->dialogs.two, open_WHEREVER);
else if ( item==4)	Dialog2_OpenDialogMenu(		app->dialogs.two, open_CENTERED);
else if ( item==5)	Event_CloseDown();

}







static void	OpenFn( dialog2_block *dialog2)
/* This is called whenever a dialog is opened.	*/
{
UNUSED( dialog2);

Debug_Printf( "Dialog %p has just opened\n", dialog2);
/*
Could call Event_Claim( event_CLICK, dialog2->window, event_ANY, functionname, (void *) dialog2)
to get clicks in this dialog window.
*/
}




int	main( void)
{
static app_block	app;

Event_Initialise( "Dialog2 example");
Resource_Initialise( "Dialog2EG");
Template_Initialise();
Template_LoadFile( "Templates");


app.dialogs.one		= Dialog2_CreateDialogBlock( "one", 0, 1, OpenFn, NULL, NULL);
app.dialogs.two		= Dialog2_CreateDialogBlock( "two", 0, 1, OpenFn, NULL, NULL);
app.dialogs.info	= Dialog2_CreateDialogBlock( "three", 0, 1, OpenFn, NULL, NULL);
	/* Info window.	*/

app.baricon = Icon_BarIcon( "switcher", iconbar_RIGHT);

app.iconbarmenu = Menu2_Create( 
	"Test", 
	">Info,Open one static,Open one menu, Open two static, Open two menu ,Quit",
	NULL,	/* Custom menu-maker	*/
	NULL,	/* Flags function	*/
	IconbarSubmenuHandler,
	IconbarMenuChoiceHandler,
	&app
	);

Menu2_AttachMenu( window_ICONBAR, app.baricon, app.iconbarmenu, button_MENU);

for(;;)	Event_Poll();

return 0;
}
