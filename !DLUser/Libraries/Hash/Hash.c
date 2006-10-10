/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    Hash.h
    Author:  Copyright © 1995 Simon Truss
    Version: 1.00 (19 May 1995)
    Purpose: A Hash Table Object, Quick lookup of text strings
*/

/*
 * These are hash-routines from a standard library, stripped down
 * and specialized. The search loops look like they might spin
 * forever, but they proveably terminate due to an algebraic property
 * of the REHASH function. I don't want to go into it now. Trust me.
 *
 * SMT May 95. I don't know who wrote this or where it came from but I've
 * found it useful so I've expanded on it for release with DeskLib
 */
#include <string.h>

#include "DeskLib:Hash.h"
/*#include "DeskLib:Malloc.h"*/

/* Malloc will not return if it fails to get enough memory it will try to
 * call a global error handler.
 */
/*#define emalloc(x) Malloc(x,"Hash Library")*/
#include <stdlib.h>
#define emalloc malloc



static int str_hash(char *str);
static Hash_Obj **new_table(int size);
static int round_up_to_pwr2(int initial_table_size);

void Hash_InitTable(Hash *obj, int initial_table_size)
{
    if(initial_table_size < 4) initial_table_size = 4;
    obj->size = round_up_to_pwr2(initial_table_size);
    obj->num_entries = 0;
    obj->max_entries = obj->size/2 - 1;
    obj->mask = obj->size - 1;
    obj->hash_table = new_table(obj->size);
}

void Hash_CleanTable(Hash *obj)
{
    int i;
    Hash_Obj **ht = obj->hash_table;
    
    for(i = 0; i < obj->size; i++) {
        Hash_Obj *dc = ht[i];
        if(dc) {
            free((char*)dc->name);
            free((char*)dc);
        }
    }

    free((char*)obj->hash_table);
}


static void hash_overflow(Hash *obj);

#define HASH(cont) ((str_hash(cont)) & obj->mask )
#define REHASH(num)  (((((num)+1)*3) & obj->mask) )

/* Put a new entry into the table. If there was previously an
 * equivalent entry in the table, return it.
 * If there was not, return NULL.
 */
Hash_Obj* Hash_PutObj(Hash *obj, Hash_Obj *entry)
{

    int bucket_number;
    Hash_Obj **bucket;

    bucket_number = HASH(entry->name);
    
    while(1)  {

        bucket = obj->hash_table + bucket_number;
        
        if ( *bucket == (Hash_Obj*)0 )  { 
            *bucket = entry;
            obj->num_entries++;
            if ( obj->num_entries > obj->max_entries )
              hash_overflow(obj);
            return (Hash_Obj*)0;  /* <======== added new entry */
      }
      
      if ( strcmp( entry->name, (*bucket)->name ) != 0) { 
          bucket_number = REHASH(bucket_number);
          continue; /* <====== search some more (collision) */
      }
        
        /* Found old Hash_Obj. Replace. */
      { 
          Hash_Obj *old = *bucket;
          *bucket = entry;
          return old; /* <============== replaced entry */
      }
        
    }
    
}


/* Find an equivalent entry.  If there is none, return NULL.
 * Do not change the table.
 */
Hash_Obj* Hash_GetObj(Hash *obj, char *name)
{

   int bucket_number;
   Hash_Obj** bucket;

   bucket_number = HASH(name);

   while(1)    {
       bucket = obj->hash_table + bucket_number;
       
       if ( *bucket == (Hash_Obj*)0 ) { 
          return (Hash_Obj*)0; /* <====== entry not found */
        }
      
       if ( strcmp( name, (*bucket)->name) != 0)        { 
           bucket_number = REHASH(bucket_number);
           continue; /* <====== search some more (collision) */
       }
       
       return *bucket; /* <====== found old Hash_Obj */
   }
   
}

/* private routine doubles the size of the table.
 */
static void hash_overflow(Hash *obj)
{
  Hash_Obj** old_hash = obj->hash_table;
  int old_size = obj->size;
  int recno;
  
  obj->max_entries = obj->size - 1;
  obj->size= obj->size * 2;
  obj->mask= obj->size - 1;
  obj->hash_table = new_table(obj->size);
  
  /* Take everything that was in the old table, and put it
   ** into the new one.
   */
  
  for (recno = 0; recno < old_size; recno++) {
      Hash_Obj **mem = old_hash + recno;
      if ( *mem != 0 ) { 
          int bucket_number = HASH((*mem)->name);
          while(1) {
              Hash_Obj** bucket = obj->hash_table + bucket_number;
              if ( *bucket == 0 ) { 
                  *bucket = *mem;
                  break; /* <==== copied it */
              }
              
              /* search some more */
              bucket_number = REHASH(bucket_number);
              
          }
      }
  }

  free((char*)old_hash);

}



/* private routine creates new hash-table.
 */
static Hash_Obj** new_table(int size)
{
    
    Hash_Obj** table =
      (Hash_Obj**)emalloc(sizeof(Hash_Obj)*size);
    Hash_Obj** cursor = table;
    Hash_Obj** end = table+size;
    while(cursor < end)  *cursor ++ = 0;
    
    return table;
}



static int round_up_to_pwr2(int initial_table_size)
{
    int size = 4; /* algorithm does not work with 1 or 2 */
    
    while(size < initial_table_size ) {
        size*= 2;
    }
    return size;
}



static int str_hash(char *str)
{
    int retval = 0;
    while(*str) retval += retval + (unsigned char)(*str++);
    return retval;
}

void Hash_WalkTable(Hash *hashtable,Hash_WorkFunc WorkFunc)
{
int i;
Hash_Obj **ht = hashtable->hash_table;

  for(i = 0; i < hashtable->size; i++) {
    Hash_Obj *dc = ht[i];
    if(dc) {
      WorkFunc(dc);
      }
    }
}

/*
 * Find and entry in hash table, remove it then return found object
 * for cleaning up of memory
 */
Hash_Obj *Hash_RemoveObj(Hash *obj,char *name)
{

   int bucket_number;
   Hash_Obj** bucket;
   Hash_Obj *found;

   bucket_number = HASH(name);

   while(1)    {
       bucket = obj->hash_table + bucket_number;
       
       if ( *bucket == (Hash_Obj*)0 ) { 
          return (Hash_Obj*)0; /* <====== entry not found */
        }
      
       if ( strcmp( name, (*bucket)->name) != 0)        { 
           bucket_number = REHASH(bucket_number);
           continue; /* <====== search some more (collision) */
       }

       found = *bucket;
       *bucket = (Hash_Obj*)0; /* remove Hash_Obj from table */
       return found; /* <====== found old Hash_Obj */
   }

}
