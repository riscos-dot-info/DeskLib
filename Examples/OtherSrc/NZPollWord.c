/*  A quick test of Wimp_PollWords under RISC OS 3, by Jason Williams
 *
 *  This sets up a bit of code in the RMA which after 5 seconds will set a
 *  word of memory (also in the RMA) to a non-zero value. This should hopefully
 *  result in our program getting an event 13 (non-zero pollword), at which
 *  point, we'll beep and exit.
 *  (i.e. if you run this and you don't get a beep after ~5 seconds, panic!)
 *
 *  BTW, please excuse this messy code, but as I said, it's a quick test!
 *
 *  Oh, and a warning - this allocates 16 bytes in the RMA, but never frees
 *  it, so if you run it over 4000 times in a row, you may notice your RMA
 *  growing! ;-)
 */


#include <stdlib.h>
#include <stdio.h>
#include "string.h"

#include "DeskLib:WimpSWIs.h"
#include "DeskLib:SWI.h"
#include "DeskLib:Sound.h" 


/*  Code pinched from OtherSrc.TaskSlice.c:
 *    STR R12, [R12]:MOV PC, R14
 *  (Called after 5 seconds to tell our task to quit via a Wimp PollWord)
 */
int callaftercode[4] = {0xe58cc000, 0xe1a0f00e};

#define XOS_Module            0x0002001e
#define XOS_CallAfter         0x0002003b
#define XOS_RemoveTickerEvent 0x0002003d


int main()
{
  unsigned int    version = 300;
  task_handle     taskhandle;
  int             *memory, *callafterflag;
  event_pollmask  mask;
  event_pollblock event;
  int             quit = FALSE;

  Wimp_Initialise(&version, "Test", &taskhandle, NULL);

  SWI(4,3, XOS_Module, 6, NULL, NULL, 16, /* TO */ NULL, NULL, &memory);

  callafterflag = memory + 2;                      /* int pointer arithmetic */

  memcpy(memory, callaftercode, 8);            /* Copy our code into the RMA */
  *callafterflag = 0;
  SWI(3, 0, XOS_CallAfter, 500, memory, callafterflag);

  mask.value = 1; /* Mask out NULLs. */

  mask.data.nonzeropollword = FALSE;    /* Allow the event to get to us */
  mask.data.r3ispollwordptr = TRUE;     /* Inform WIMP that r3 is valid */

  quit = FALSE;
  while (!quit)
  {
    Wimp_Poll3(mask, &event, callafterflag);

    switch(event.type)
    {
      case event_NONZEROPOLLWORD:
        Sound_SysBeep();
        quit = TRUE;
        break;

      case event_USERMESSAGE:
      case event_USERMESSAGERECORDED:
        if (event.data.message.header.action == 0)   /* quit from taskwindow */
          quit = TRUE;
        break;
    }
  }

  exit(0);
}
