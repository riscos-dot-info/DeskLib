 
#ifndef __dl_hash_h
#define __dl_hash_h
#ifndef __dl_core_h
#include "DeskLib:Core.h"
#endif
typedef struct {
char *name;
} Hash_Obj;
typedef struct
{
Hash_Obj **hash_table;
int size;
int max_entries;
int mask;
int num_entries;
}Hash;
extern Hash_Obj* Hash_GetObj(Hash *hashtable, char *name);
extern Hash_Obj* Hash_PutObj(Hash *hashtable, Hash_Obj *entry);
extern void Hash_InitTable(Hash *hashtable, int initial_table_size);
extern void Hash_CleanTable(Hash *hashtable);
typedef void (*Hash_WorkFunc)(Hash_Obj *);
extern void Hash_WalkTable(Hash *hashtable,Hash_WorkFunc WorkFunc);
extern Hash_Obj* Hash_RemoveObj(Hash *hashtable, char *name);
#endif
