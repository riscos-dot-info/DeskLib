#ifndef __dl_menudefs_h
#define __dl_menudefs_h

#ifndef __dl_wimp_h
#include "DeskLib:Wimp.h"
#endif

extern void Menu__CountItems(char *description, int *numitems,
                             unsigned int *menuwidth);

extern BOOL Menu__Create(menu_item *item, char *description, int numitems);

#endif
