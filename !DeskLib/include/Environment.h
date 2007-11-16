/*
 * This file is part of DeskLib, the C library for RISC OS.
 * Please see accompanying documentation for terms of use.
 *
 *       http://www.riscos.info/index.php/DeskLib
 *
 *
 * Module:  Environment
 * File:    Environment.h
 * Author:  Adam Richardson
 * Purpose: Interact with desktop environment.
 *
 * Version History
 * 18/08/2007: Creation
 * 25/10/2007: Moved SysBeep function in
 * 31/10/2007: Added OSVersion
 *
 */

/* Abstract */
/*
   This module provides functions to analyse and interact with the desktop
   environment, including logging, manipulating system variables and examining
   the active desktop processes.

   Most of the functions are self contained. For normal logging just use the
   dl_Environment_LogMessage function. (See function description for other
   functions.)
*/

#ifndef __dl_environment_h
#define __dl_environment_h

#include "DeskLib:Core.h"
#include "DeskLib:Wimp.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
  dl_log_EMERGENCY   = 20,
  dl_log_ALERT       = 30,
  dl_log_CRITICAL    = 40,
  dl_log_ERROR       = 60,
  dl_log_WARNING     = 80,
  dl_log_NOTICE      = 100,
  dl_log_INFORMATION = 150,
  dl_log_DEBUG       = 200
} dl_log_LEVEL;
/*
   You can set the log level to any number from 0 to 255. To quote the
   original Doggysoft SysLog help file:

   "Day-to-day information should probably be logged at around level 100.
    Certainly avoid levels of less than around 50 for non-critical messages.
    Level 10 means that the message is of similar importance to "Government
    declares war". Level 0 is of similar importance to "Armageddon imminent."
    Level 255 is of similar importance to "Water detected in ocean". The
    logging output of your program is much more useful if prorities are
    used well."
*/


typedef struct
{
  int handle;
  int slotsize;
  BOOL ismodule;
  BOOL candragbar;
} dl_task_info;
/*
   This struct contains the information returned from the taskmanager about
   active tasks. The slotsize is given in bytes. ismodule is TRUE if the
   task is a module and candragbar is TRUE is the slotsize bar in the
   taskmanager window can be dragged.
*/

BOOL dl_Environment_TaskIsActive(const char *appname);
/*
   This returns TRUE if another task called "appname" is already active.
*/

os_error *dl_Environment_TaskList(char ***ptrlist, int *numtasks);
/*
   This functon generates a list of the currently active tasks. It returns
   an error if it fails and the number of tasks in the numtasks variable.
   You could access the list using something like:

    char **listofptrs;
    int i, num;

    dl_Environment_TaskList(&listofptrs, &num);

    for (i = 0; i < num; i++) printf("Item %d:%s", i, listofptrs[i]);

   The memory used for the list is dynamically allocalted using malloc() so
   after you've finished using the list, you should free the memory using
   the dl_Environment_TaskListFree function.
*/

void dl_Environment_TaskListFree(char **ptrlist, int numtasks);
/*
   This function frees the memory used to generate a list of task names with
   dl_Environment_TaskList. You should pass it the parameters used with
   dl_Environment_TaskList.
*/

os_error *dl_Environment_TaskNameFromHandle(task_handle task, char *buf, int bufsize);
/*
   This will return the name of a task, given its handle. The result (limited
   to "bufsize" characters) is placed in the buffer you provide, buf.
*/

os_error *dl_Environment_TaskInfo(const char *appname, dl_task_info *taskinfo);
/*
   This function returns information about other tasks. Pass the name of the
   task you are interested in as appname and the values of taskinfo will be
   filled, if the task is active. If the task is not active, an error will
   be returned.
*/

os_error *dl_Environment_LogMessage(int level, const char *message, ...);
/*
   Call this function to log a message via SysLog. "message" is formatted
   in the same way as printf and the expanded length islimited to 1024 characters.
   "level" may be any number from 0 to 255. A number of predefined values are
   provided in dl_log_LEVEL to get you started.

   This function uses the name passed to Event_Initialise for the log name
   (truncated to ten characters). If you wish to use a different name or
   use the function before you've called Event_Initialise, you should call
   dl_Environment_LogInitialise first. If you want to specify the log name
   for every call, use dl_Log_MessageWithName.
*/

BOOL dl_Environment_LogInitialise(const char *name);
/*
   This function is called automatically to set the log name to be the first ten
   characters of the task name passed to Event_Initialise. (If Event_Initialise
   has not been called then "DeskLibLog" is used.)

   You should only call this function explicitly if you wish to override this
   default behaviour. The name is updated every time this function is called.

   This function also tests for the presence of the SysLog module. If it exists
   it returns TRUE, if it doesn't it returns FALSE and subsequent log calls will
   return an error.
*/

os_error *dl_Environment_LogMessageWithName(const char *logname, int level, const char *message);
/*
   This function is called by dl_Log_Message. You should only call it directly if
   you need to change the logging name with every message.
*/

BOOL dl_Environment_ModuleIsActive(const char *modulename);
/*
   This function returns TRUE if the named module is active and FALSE otherwise.
*/


BOOL OS_ReadVarVal(const char *varname, char *buf, int bufsize);
/*
  This reads the value of the system variable whose name is 'varname',
  placing the text in the buffer 'buf' of size 'bufsize'. It returns
  true is the variable exists and false otherwise.

  If bufsize is 0 then only the presence of the variable will be tested
  and the buffer will not be altered.
*/

typedef enum
{
  dl_sysvar_STRING = 0,
  dl_sysvar_NUMBER,
  dl_sysvar_MACRO,
  dl_sysvar_EXPANDED,
  dl_sysvar_LITERAL,
} dl_sysvar_type;
/*
   This is used to pass meaningful names to dl_Environment_SetSysVar.

   A string will be evaluated before the value is set.
   A macro will be evaluated each time the system variable is used.
   A literal will not be evaulated when set, nor when used.
   "Expanded" means the value will be passed to the OS_EvaluateExpression SWI.
   If the type is a number the string value passed to dl_Environment_SetSysVar
   will be evaluated to an integer.
*/

os_error *dl_Environment_SysVarSet(const char *varname, const char *value, dl_sysvar_type type);
/*
   This function will set a system variable, of type 'type'. The name
   of the system variable should be passed in 'varname'. The name can be
   an existing system variable, for updating, or a new system variable,
   to be created. If the name contains wildcards (*, #) it is assumed
   an update is required and if no existing system variable matches the
   expression, an error will be returned. Only the first system variable
   which matches the expression will be set.

   The value for the system variable is passed in 'value'. The
   interpretation of 'value' will depend on the 'type'.

   The function returns NULL if successful and an error if not.
*/

os_error *dl_Environment_SysVarDelete(const char *varname);
/*
   This function takes one argument, which is the (case insensitive)
   name of the system variable you wish to delete. The name can be
   wildcarded using * and #. Only the first match for a wildcarded
   expression will be deleted.

   The function returns NULL if successful (e.g. a variable is found
   and deleted) and an error if not.
*/

os_error *OS_GSTrans(const char *string, char *buffer,
                                      int size, char **end);
/*
  This is a veneer to the OS_GSTrans SWI, and converts the string 'string'
  using GS operations, placing the resulting string in 'buffer' whose size
  is given by 'size'.  It also sets 'end' to point to the character after
  the terminator.  This expands system variable references in the source
  string.

  It returns NULL if there is no error.
*/

os_error *OS_CLI(const char *cmd);
/*
  This is a veneer for the OS_CLI SWI, executing the given star command.
*/

extern void Sound_SysBeep(void);
/*
  Simply writes a character 7 (system beep) to the VDU stream, in order
  to sound a system beep. It sorta goes 'beep', really.
*/

int dl_Environment_OSVersion(void);
/*
   This function returns information on the OS version. Currently
   it will only return one of the following values: 0 (Unknown), 100
   (Arthur), 200 (RISC OS 2), 300 (RISC OS 3), 400 (RISC OS 4), 430
   (RISC OS Select, Adjust and 6) or 500 (RISC OS 5).

   However, the function may be extended in future to return the exact
   OS version, so you should account for that eventuality in your
   program design.
*/

#ifdef __cplusplus
}
#endif

#endif
