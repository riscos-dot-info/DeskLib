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


#ifndef __dl_hash_h
#define __dl_hash_h

/*T*************************************************************************/
typedef struct {
  char *name;
  } Hash_Obj;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  Purpose:  an abstract holder for the data the user is storing
  Fields:   only name is required for the hash library to work at the mo.
            This can be placed in the first element of a struct to hold
            more data. An alternative that works at the moment is to name
            the first element of you structure char *something. This
            definition is for your convenience.
  See:      Example of use in HashTest.c
****************************************************************************/


/*T*************************************************************************/
typedef struct
{
  Hash_Obj **hash_table;
  int size;
  int max_entries;
  int mask;
  int num_entries;
}Hash;
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  Purpose:  All the data required for the Hash table functions to work.
  Fields:   hash_table is a pointer to an array of Hash_Objs (the hash table)
            mask is the bit mask used to limit the size of rehashed numbers
            max_entries, size of hash table (power of 2)
            num_entries, guess
****************************************************************************/


/*F*************************************************************************/
extern Hash_Obj* Hash_GetObj(Hash *hashtable, char *name);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  find an Hash_Obj in the hashtable from the search name
  Inputs:   hashtable - yep you guessed it
            name - the name to be searched for
  Returns:  Hash_Obj * - the Hash_Obj found in the hashtable or NULL if failed
  Errors:   None
  SeeAlso:  Hash_put; Hash_remove;
****************************************************************************/



/*F*************************************************************************/
extern Hash_Obj* Hash_PutObj(Hash *hashtable, Hash_Obj *entry);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  Put an Hash_Obj into the hash table
  Inputs:   hashtable - guess again.
            entry - Hash_Obj to be placed in hash tabel, entry->name
            must already be set up.
  Returns:  equivalent Hash_Obj already in hashtable or NULL if no others
  Errors:   Memory errors handled by Malloc()
****************************************************************************/



/*F*************************************************************************/
extern void Hash_InitTable(Hash *hashtable, int initial_table_size);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  Initialise the hash table to set size
  Inputs:   hashtable - uninitialised hash table object
            initial_table_size - size of the hash table, this should
            be a power of 2 but any number will be rounded up to the
            nearest power of 2, the minimum table size will be rounded
            up to 4 Hash_Objs.
  Errors:   Out of Memory (handled by Malloc)
  SeeAlso:  Malloc() in Malloc.h
****************************************************************************/



/*F*************************************************************************/
extern void Hash_CleanTable(Hash *hashtable);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  tidy up memory and free everything using Malloc.h lib
  Inputs:   hashtable - all Hash_Obj->name, Hash_Obj and hastable are freed up
  Errors:   Nasty unexpected things if you pass in an incorrect pointer
****************************************************************************/


/*T*************************************************************************/
typedef void (*Hash_WorkFunc)(Hash_Obj *);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  Pointer to a function that uses the Hash)Obj passed to it
****************************************************************************/


/*F*************************************************************************/
extern void Hash_WalkTable(Hash *hashtable,Hash_WorkFunc WorkFunc);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  call Work function for each Hash_Obj in hash table
  Inputs:   hashtable
            WorkFunc() this function should do something with the
              Hash_Obj passed into it.
  Outputs:  Calls Work function
  Errors:   Bad things if incorrect pointers are passed in
****************************************************************************/


/*F*************************************************************************/
extern Hash_Obj* Hash_RemoveObj(Hash *hashtable, char *name);
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

  Purpose:  find a Hash_Obj in the hashtable from the search name and
            remove it from the table. The found object will be returned
            so the user can free the Hash_Obj's memory.
  Inputs:   hashtable - yep you guessed it
            name - the name to be searched for
  Returns:  Hash_Obj * - the Hash_Obj found in the hashtable or NULL if failed
  Errors:   None
  SeeAlso:  Hash_put; Hash_get;
****************************************************************************/


#endif
