 
#ifndef __Import_h
#define __Import_h
#ifndef __dl_wimp_h
#include "DeskLib:Wimp.h"
#endif
typedef void (*import_fileimporter)( message_dataload *data, void *reference);
typedef void *import_ramimporter;
BOOL Import_RegisterFileLoader( 
import_fileimporter fn, 
import_ramimporter ramfn,
window_handle window, 
icon_handle icon, 
void *reference
);
#endif
