#ifndef __dl_Core_h
#include "DeskLib:Core.h"
#endif


typedef struct msg_def *msgdefptr;

typedef struct msg_def
{
  msgdefptr   next;
  union
  {
    char        *text;
    msgdefptr   taglist;
  } data;
  char        tag[10];
} msg_def;

extern msgdefptr msgs_grouplist;
extern BOOL Msgs__MatchToken(char *tag1, char *tag2, BOOL wcallowed);

extern msgdefptr Msgs__Find(msgdefptr *liststart, char *tag,
                            BOOL create, BOOL wcallowed);

#define MatchTokenI(t1, t2) (((int*)t1)[0]==((int*)t2)[0] && ((int*)t1)[1]==((int*)t2)[1])
