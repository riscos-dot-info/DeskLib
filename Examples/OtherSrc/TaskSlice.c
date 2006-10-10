/*
    ####             #    #     # #
    #   #            #    #       #          The FreeWare C library for 
    #   #  ##   ###  #  # #     # ###             RISC OS machines
    #   # #  # #     # #  #     # #  #   ___________________________________
    #   # ####  ###  ##   #     # #  #                                      
    #   # #        # # #  #     # #  #    Please refer to the accompanying
    ####   ### ####  #  # ##### # ###    documentation for conditions of use
    ________________________________________________________________________

    File:    TaskSlice.c
    Author:  Copyright © 1992 Jason Williams
    Version: 1.00 (11 Apr 1992)
    Purpose: Easy and accurate way of allocating time to a "subtask"

 *  NOTES:
 *
 *  This is the subtask timeslice "manager" used by DSEdit II. It works
 *  extremely well in practice.
 *
 *  My definition of a "subtask" is a function within a main program that is
 *  run in a multitasking manner so several such functions may be executing
 *  alongside each other (e.g. A sound echo generator operating on a sound
 *  sample can be left to do its job in the background while the user
 *  selects and edits another portion of the sound sample)
 *  Several echo functions could be applied to several sound samples, in
 *  which case, each time a NULL poll is received by the program, ONE of
 *  subtasks will be called and allowed to execute for a small (5cs)
 *  timeslice.
 *  A subtask CANNOT and DOES NOT EVER call Wimp_Poll{Idle}. It should
 *  never do anything that might page out the CallAfter code!
 *
 *  This code provides a method for running a cooperative subtask for a
 *  timeslice. It is not preemptive, but so long as the subtask is "good"
 *  it will exit very close to the desired time.
 *
 *  It operates along the following lines:
 *  When you start the subtask running, you place some ARM code on the
 *  OS_CallAfter event, where the CallAfter is set to the time that you want
 *  to give to the subtask (e.g. 5 centiseconds)
 *  Simply put, you generate some code that will be called when it's time
 *  for the subtask to stop processing, and the code sets a flag to indicate
 *  to the subtask that its time is up.
 *
 *  You then call the subtask routine, which should check the passed in
 *  variable reasonably often: when the variable becomes non-zero, it should
 *  stop processing, and return. (It can set the variable inside itself to
 *  force termination if it wishes)
 *
 *  i.e. the code should be something like:
 *    BOOL SubTask(int *endflag)
 *    {
 *      while (!*endflag)
 *      {
 *        ...process...
 *      }
 *      return(error_flag);
 *    }
 *
 *  Your function (and Task_Slice) return a boolean value. This is intended
 *  as a method for errors to be passed back, or as a "task completed" flag.
 *
 *  Anyway, you will need to modify Task_Slice in order to pass in any
 *  information that is necessary to the subtask, so I have included this
 *  routine as SOURCE CODE ONLY. It is not part of the DeskLib Library.
 *  It is basically a working example of how to cleanly timeslice subtasks.
 *
 *  The main-program code is therefore something like:
 *    When we get a NULL event
 *      get the next task to be run
 *      Call Task_Slice();
 *
 *
 *  CallAfter code currently used:
 *    STR R12, [R12]     ; set flag to non-zero value
 *    MOV PC, R14        ; return
 *
 *  Note that this code took a humungous effort on the part of a dedicated
 *  team of 1 people, and took over 20 seconds to develop (including poking
 *  at SWI OS_CallAfter in order to find out how it worked). Just remember
 *  that when you go to use it, sitting back in your easy-chair, and think
 *  how lucky you are that you didn't have to go to all that bother ;-)
 */

#include "DeskLib:Core.h"
#include "DeskLib:SWI.h"


static int CallAfterCode[2] = {0xe58cc000, 0xe1a0f00e};
static int CallAfterFlag    = 0;

#define XOS_CallAfter         0x0002003b
#define XOS_RemoveTickerEvent 0x0002003d

typedef BOOL (*taskfunction)(int *)



extern BOOL Task_Slice(taskfunction taskfn, int slicetime)
/* Timeslices the given function for "slicetime" centiseconds. It is up to
 * the function itself to check the timeslice termination flag, and therefore
 * the function itself determines how accurately it cooperates with us...
 *
 * Timeslices of less than 1 cs or greater than 100cs (1 second) are not
 * allowed (To multitask nicely, a timeslice should be between 1cs and 20cs)
 * A value of 10cs is recommended, though in some cases you will find other
 * values work better.
 *
 * The interesting thing to note is this:
 * - Under about 5cs slices, the overhead for polling and your task
 *   management tends to soak up a lot of the time, so it is very
 *   inefficient.
 * - Over about 20cs, the overhead for polling is minimal, and very little
 *   advantage can be gained by increasing the timeslice - The processing
 *   operation does not become noticably faster, but the number of polls per
 *   second becomes low enough for normal desktop operation to be disturbed
 *
 * IMPORTANT
 * If your subtask takes LESS time than its allocated slice, then it is very
 * dangerous to Wimp_Poll, as after your task has been switched out, the
 * CallAfter comes around...
 * Thus, after calling the task function, the event is removed to ensure your
 * computer's continued good health (using RemoveTickerEvent)
 */
{                                   
  BOOL result;

  if (slicetime < 1) slicetime = 1;
  if (slicetime > 100) slicetime = 100;

  CallAfterFlag = 0;
  SWI(3, 0, XOS_CallAfter, slicetime, &(CallAfterCode[0]), &CallAfterFlag);

  result = taskfn(&CallAfterFlag);

  if (!CallAfterFlag)
    SWI(2, 0, XOS_RemoveTickerEvent, &(CallAfterCode[0]), &CallAfterFlag);

  return(result);
}
