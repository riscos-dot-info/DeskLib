/* note: to build this test program you will need to link
 * with o.error as well as desklib
 */
#include "DeskLib:Hash.h"

#include <stdio.h>

typedef struct {
  char *name; /* could be Hash_Obj item; as well */
  int data;
  } obj;

static void print_entry(Hash_Obj *entry)
{
obj *item = (obj *) entry;

if (entry != 0) {
  printf("name: %s\nnumber %d\n",item->name,item->data);
  }
else {
  printf("called with null pointer\n");
  }
}

#define setup(a,b,c) entry[(a)].data = (b);\
        entry[(a)].name = (c);\
        Hash_PutObj(&table,(Hash_Obj *)&entry[(a)]);\
        print_entry((Hash_Obj *)&entry[(a)]);

/* example output from macro called with setup(0,2,"two")
 * entry[0].data = 2;
 * entry[0].name = "two";
 * Hash_PutObj(&table,(Hash_Obj *)&entry[0]);
 * print_entry((Hash_Obj *)&entry[0]);
 */

int main(void)
{
Hash table;
obj entry[10];
obj *found;

Hash_InitTable(&table,8);

setup(0,2,"two")
setup(1,5,"five")
setup(2,12,"twelve")

found = (obj *) Hash_GetObj(&table,"five");
printf("The following entry was found: ");
print_entry((Hash_Obj *)found);

setup(3,65,"sixty Five")
setup(4,21,"twenty one")

entry[5].data = 12345;
entry[5].name = "A big number";
Hash_PutObj(&table,(Hash_Obj *)&entry[5]);

setup(9,101,"##this should have been removed")
setup(6,321,"three two 1")
setup(7,77,"seven seven")
setup(8,99,"ninety ninety nine")

Hash_RemoveObj(&table,"##this should have been removed");

printf("Listing all entries\n");
Hash_WalkTable(&table,print_entry);

}
