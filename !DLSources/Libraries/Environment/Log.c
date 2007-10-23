/*
 * This file is part of DeskLib, the C library for RISC OS.
 * Please see accompanying documentation for terms of use.
 *
 *       http://www.riscos.info/index.php/DeskLib
 *
 *
 * Module:  Environment
 * File:    Log.c
 * Author:  Adam Richardson
 * Purpose: Provide logging facilities (using SysLog SWIs)
 *
 * Version History
 * 27/08/2007: Creation
 *
 */
#include "DeskLib:Debug.h"
#include "DeskLib:Core.h"
#include "DeskLib:Environment.h"
#include "DeskLib:SWI.h"
#include "DeskLib:Error.h"
#include "DeskLib:Event.h" /* For event_taskname declaration */
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

/* Globals */
BOOL dl_environment__syslogpresent = FALSE;
BOOL dl_environment__loginitialised = FALSE;
char dl_environment__logname[11] = "DeskLibLog"; /* DoggySoft limitiation: 10 chars (we add space for terminator) */

BOOL dl_Environment_LogInitialise(const char *name)
{
  /* SysLog names can only be 10 characters long */
  strncpy(dl_environment__logname, name, sizeof(dl_environment__logname)-1);
  dl_environment__logname[sizeof(dl_environment__logname)-1] = '\0';

  /* Check for presence of module. Used in logging fn proper to prevent calls to non-existant SWI */
  dl_environment__syslogpresent = dl_Environment_ModuleIsActive("Syslog");

  dl_environment__loginitialised = TRUE;

  return dl_environment__syslogpresent;
}

os_error *dl_Environment_LogMessage(int level, const char *message, ...)
{
  os_error *error;
  char buffer[1024]; /* Doggysoft limitiation on size */
  va_list argptr;

  if (!dl_environment__loginitialised)
  { /* Initialise function not called yet - do so now. */

    if (strcmp(event_taskname, ""))
      /* event_taskname has been filled with something (e.g. Event_Initialise has been called) */
      dl_Environment_LogInitialise(event_taskname);
    else
      /* Default log name (dl_environment__logname initialised to this anyway) */
      dl_Environment_LogInitialise("DeskLibLog");
  }

  va_start(argptr, message);
  vsnprintf(buffer, sizeof(buffer), message, argptr);
  va_end(argptr);

  /* Ensure correct termination (in case where message is longer than buffer) */
  buffer[sizeof(buffer)-1] = '\0';

  error = dl_Environment_LogMessageWithName(dl_environment__logname, level, buffer);

  return error;
}

os_error *dl_Environment_LogMessageWithName(const char *logname, int level, const char *message)
{
  os_error *error;
  char namebuffer[11], messagebuffer[1024];

  /* Note 10 char limit for syslog name and 1024 limit for message */
  strncpy(namebuffer, logname, sizeof(namebuffer) - 1);
  namebuffer[sizeof(namebuffer) - 1] = '\0';

  strncpy(messagebuffer, message, sizeof(messagebuffer)-1);
  messagebuffer[sizeof(messagebuffer) - 1] = '\0';

  /* level must be between 0 and 255 */
  if (level < 0)   level = 0;
  if (level > 255) level = 255;

  if (dl_environment__syslogpresent)
  {
    error = SWI(3, 0, SWI_SysLog_LogMessage | XOS_Bit, namebuffer, messagebuffer, level);
  }
  else
  {
    error_global.errnum = 1;
    strncpy(error_global.errmess, "SysLog module not present", sizeof(error_global.errmess) - 1);
    error = &error_global;
  }

  return error;
}

