/* ------------------------------------------------ ajps, 2003-03-04 ---
 * File: haddoctosh.c
 * Abstract: The central functions for the xmltosh (xml to stronghelp)
 * conversion program.
 * Copyright (c) 2003 Antony Sidwell
 * --------------------------------------------------------------------- */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "libxml/parser.h"

#include "hparser.h"
#include "haddoctosh.h"

const char *ifilename = NULL;
const char *ofilename = NULL;

static void opt_preopts(void);
static int opt_postopts(int argc, char *argv[]);

struct
{
  const char *tok;
  BOOL *swch;
} cl_switches[] =
{
  {NULL, NULL}
};

/*
 * Takes the number of arguments remaining, and a pointer to
 * an array of those remaining arguments.  It should return the
 * number of arguments it has "used up", and exit(EXIT_FAILURE)
 * if anything is wrong, preferably with an appropriate error
 * message output to stderr.
 */
typedef int cl_fn(int argc, char *argv[], int ref);

static cl_fn opt_help;
static cl_fn opt_input;

struct
{
  const char *tok;
  cl_fn *fn;
  int ref;
} cl_fns[] =
{
  {"-help", opt_help, 0},
  {"h", opt_help, 0},
  {"-input", opt_input, 0},
  {"i", opt_input, 1},
  {NULL, NULL, 0}
};


int cl_parse(int argc, char *argv[])
{
  int n;
  int next;
  int i;
  char *ptr;
  BOOL matched = FALSE;

opt_preopts();

  for (n = 0, next = 1; n < argc && argv[n][0] == '-'; n = next, next++)
  {
    ptr = argv[n] + 1;

    /* Double-dashed long option */
    if (*ptr == '-')
    {
      matched = FALSE;

      for (i = 0; cl_fns[i].tok != NULL && !matched; i ++)
      {
        if (strcmp(ptr, cl_fns[i].tok) == 0)
        {
          next += (cl_fns[i].fn)(argc - next, argv + next, cl_fns[i].ref);

          matched = TRUE;
        }
      }

      for (i = 0; cl_switches[i].tok != NULL && !matched; i ++)
      {
        if (strcmp(ptr, cl_switches[i].tok) == 0)
        {
          *(cl_switches[i].swch) = TRUE;
          matched = TRUE;
        }
      }

      if (matched == FALSE)
      {
         fprintf(stderr, "Option '-%s' is not known and has been ignored.\n", ptr);
         exit(EXIT_FAILURE);
      }
    }
    else while (*ptr)
    {
      matched = FALSE;

      for (i = 0; cl_fns[i].tok != NULL && !matched; i ++)
      {
        if (*ptr == cl_fns[i].tok[0])
        {
          next += cl_fns[i].fn(argc - next, argv + next, cl_fns[i].ref);

          matched = TRUE;
        }
      }

      for (i = 0; cl_switches[i].tok != NULL && !matched; i ++)
      {
        if (*ptr == cl_switches[i].tok[0])
        {
          *(cl_switches[i].swch) = TRUE;
          matched = TRUE;
        }
      }

      if (matched == FALSE)
      {
        fprintf(stderr, "Option '-%c' is not known and has been ignored.\n", *ptr);
        exit(EXIT_FAILURE);
      }

      ptr ++;
    }
  }

  next--;
  next += opt_postopts(argc - next, argv + next);

  return (next);
}


static int opt_help(int argc, char *argv[], int ref)
{
  printf("haddoctosh version 0.10 (2003-06-14) Copyright (c) 2003 Antony Sidwell.\n");
  printf("-----------------------------------------------------------------------\n");
  printf("haddoctosh takes the XML output generated by haddoc to produce a \n");
  printf("Stronghelp manual describing the library.\n");
  printf("Syntax: haddoctosh [<options>] <filename>\n\n");
  printf("Options:\n");
  printf("-h --help            Print this help text.\n");
  printf("-i --input <file>    Read input from the given file, otherwise it is read.\n");
  printf("                     from stdin.\n");

  exit(EXIT_SUCCESS);

  return 0;
}



static int opt_input(int argc, char *argv[], int ref)
{
  if (argc <= 0)
  {
    fprintf(stderr, "The %s option requires a filename\n", ref == 0 ? "--input" : "-i");
    exit(EXIT_FAILURE);
  }

  ifilename = argv[0];

  return 1;
}

/* ------------------------------------------------ ajps, 2003-02-28 ---
 * Called after the options have been processed.
 * --------------------------------------------------------------------- */
int opt_postopts(int argc, char *argv[])
{
  int n;

  if (argc <= 0)
  {
    fprintf(stderr, "You must specify an output filename.\n");
    exit(EXIT_FAILURE);
  }

  ofilename = argv[0];

  return 1;
}


/* ------------------------------------------------ ajps, 2003-02-28 ---
 * Set up defaults for options.
 * --------------------------------------------------------------------- */
static void opt_preopts()
{
}


int main(int argc, char *argv[])
{
  xmlDoc *parsed;

  cl_parse(argc - 1, argv + 1);

  if (ifilename == NULL)
  {
    char *buffer = NULL;
    size_t size = 0;
    size_t read = 0;
    size_t toread = 0;

    while (read == toread)
    {
      toread = size + 2048;
      buffer = realloc(buffer, size + toread);
      if (buffer == NULL)
      {
        fprintf(stderr, "Error (memory): Couldn't extend buffer when reading from stdin.\n");
        exit(EXIT_FAILURE);
      }

      read = fread(buffer + size, 1, toread, stdin);
      size += read;
    }

    if (size > 0)
    {
      parsed = xmlParseMemory(buffer, size);
      if (parsed == NULL)
      {
        fprintf(stderr, "Error (I/O): Couldn't parse stdin\n");
        exit(EXIT_FAILURE);
      }

      free(buffer);
    }
    else
    {
      fprintf(stderr, "Error (I/O): No data to parse.\n");
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    parsed = xmlParseFile(ifilename);
    if (parsed == NULL)
    {
      fprintf(stderr, "Error (I/O): Couldn't parse file '%s'\n", ifilename);
      exit(EXIT_FAILURE);
    }
  }

  assemble_manual(parsed, ofilename);

  return 0;
}


