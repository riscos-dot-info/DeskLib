/* ------------------------------------------------ ajps, 2003-03-04 ---
 * File: output.c
 * Abstract: Deals with outputting the file(s)
 * Copyright (c) 2003 Antony Sidwell
 * --------------------------------------------------------------------- */


#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <assert.h>

#include "libxml:parser.h"

#include "haddoctosh.h"
#include "output.h"
#include "parser.h"

#ifdef RISCOS
  #include "kernel.h"
#endif

#ifdef SIMPLE_OUTPUT


void write_manual(const char *filename)
{
}

IFILE *open_output(const char *filename)
{
  IFILE *tmp;

  tmp = fopen(filename, "w");
  if (tmp == NULL)
  {
    fprintf(stderr, "Error (I/O): Couldn't open file %s for output", filename);
    exit(EXIT_FAILURE);
  }

  return tmp;
}


void close_output(IFILE *file)
{
  fclose(file);
}



int wputs(const char *str, IFILE *stream)
{
  return fputs(str, stream);
}

int wputc(char c, IFILE *stream)
{
  return putc(c, stream);
}

#else /* simple output */

#define WORDALIGN(n) (((n) + 3) & (~3))

struct page
{
  struct page *next;
  char *filename;
  size_t size;
  char *content;
};

struct
{
  size_t real_size;
  size_t size;
  char *content;
} buffer = { 0, 0, NULL };

struct page *pagelist = NULL;

IFILE *open_output(const char *filename)
{
  IFILE *tmp;

  if (buffer.size != 0)
  {
    fprintf(stderr, "Can only write one file at a time (%s)\n", filename);
    exit(EXIT_FAILURE);
  }

  tmp = malloc(sizeof(*tmp));
  if (tmp == NULL)
  {
    fprintf(stderr, "Error (memory): Couldn't alloate memory for file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  /* We transfer this string to a permanent location when the file
   * is "closed" */
  tmp->filename = malloc(strlen(filename) + 1);
  if (tmp->filename == NULL)
  {
    fprintf(stderr, "Error (memory): Couldn't alloate memory for file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  strcpy(tmp->filename, filename);

  return tmp;
}

void close_output(IFILE *file)
{
  struct page *newpage;

  newpage = malloc(sizeof(*newpage));

  if (newpage == NULL)
  {
    fprintf(stderr, "Error (memory): Couldn't allocate memory for file storage\n");
    exit(EXIT_FAILURE);
  }

  /* Whack it at the start of the list */
  newpage->next = pagelist;
  pagelist = newpage;

  newpage->filename = file->filename;
  newpage->size = buffer.size;
  newpage->content = malloc(buffer.size);

  if (newpage->content == NULL)
  {
    fprintf(stderr, "Error (memory): Couldn't allocate memory for file storage\n");
    exit(EXIT_FAILURE);
  }

  memcpy(newpage->content, buffer.content, buffer.size);

  free(file);

  buffer.size = 0;
}

/*
 * Effectively concats the contents of 'to_add' onto the buffer.
 * Note that we don't worry about nul terminators here, as we don't
 * use the resulting buffer as a string at any point.
 */
void buffer_concat(const char *to_add, size_t len)
{
  char *new;
  unsigned int new_size;
  unsigned int cur_len;

  if (len == 0 || strlen(to_add) < len) len = strlen(to_add);

  if (buffer.real_size < buffer.size + len)
  {
    new = realloc(buffer.content, (buffer.size + len) * 2);
    if (new == NULL)
    {
      fprintf(stderr, "Error (memory): Not enough memory to extend text buffer.\n");
      exit(EXIT_FAILURE);
    }

    buffer.content = new;
    buffer.real_size = (buffer.size + len) * 2;
  }

  memcpy(buffer.content + buffer.size, to_add, len);
  buffer.size += len;
}

int wputs(const char *str, IFILE *stream)
{
  buffer_concat(str, 0);
  return 0;
}

int wputc(char c, IFILE *stream)
{
  buffer_concat(&c, 1);
  return 0;
}

/* Simple linked list quicksort */
struct page *sort_list(struct page *list)
{
  struct page *lower = NULL;
  struct page *higher = NULL;
  struct page *pivot;
  struct page *current;

  int comp;

  if (list == NULL) return NULL;
  if (list->next == NULL) return list;

  pivot = list;

  current = list->next;
  while (current != NULL)
  {
    struct page *next = current->next;

    comp = strcmp(pivot->filename, current->filename);

    if (comp > 0)
    {
      current->next = lower;
      lower = current;
    }
    else if (comp < 0)
    {
      current->next = higher;
      higher = current;
    }
/*    else
    {
      fprintf(stderr, "Error: Two identical filenames generated.\n");
      exit(EXIT_FAILURE);
    } */

    current = next;
  }

  lower = sort_list(lower);
  higher = sort_list(higher);

  list = lower;
  current = list;

  /* Concat the lists together, pivot in the middle */
  while (current != NULL && current->next != NULL) current = current->next;

  if (list == NULL) list = pivot;
  else current->next = pivot;

  pivot->next = higher;

  return list;
}

void write_manual(const char *filename)
{
  struct page *current;
  struct page *previous = NULL;

  FILE *fptr;
  unsigned int scratch[14];
  int total_pages = 0;

  /* We can have the alphabet, zero, and ! */
  struct
  {
    char letter;
    struct page *pages;
    int no_pages;
    size_t dir_size;
    size_t files_size;
  } dirs[28];
  int no_dirs = 0;

  char c = 0;

  int n, m;
  size_t offset;

  pagelist = sort_list(pagelist);

  current = pagelist;
  while (current != NULL)
  {
    if (current->filename[0] != c)
    {
      c = current->filename[0];
      dirs[no_dirs].letter = c;
      dirs[no_dirs].pages = current;
      dirs[no_dirs].no_pages = 0;
      /* header space */
      dirs[no_dirs].dir_size = 4;
      dirs[no_dirs].files_size = 0;
      no_dirs++;
    }

    dirs[no_dirs - 1].no_pages++;

    /* Each page adds (size of page + header) and a directory entry */
    dirs[no_dirs - 1].dir_size += 24 + WORDALIGN(strlen(current->filename) + 1);
    if (current->size > 0)
    {
      dirs[no_dirs - 1].files_size += 8 + WORDALIGN(current->size);
    }

    current = current->next;
  }

  fptr = fopen(filename, "wb");
  if (fptr == NULL)
  {
    fprintf(stderr, "Error (I/O): Couldn't open manual for output");
    exit(EXIT_FAILURE);
  }

  /* Magic word "HELP" */
  scratch[0] = 0x504C4548;
  /* Size of root block (always 44) */
  scratch[1] = 44;
  /* Version of Stronghelp which made the manual */
  scratch[2] = 280;
  /* Offset to first free block */
  scratch[3] = -1;
  /* Offset to root dir */
  scratch[4] = 44;
  /* Load & exec addresses */
  scratch[5] = 0xFFFFFD;
  scratch[6] = 0;
  /*
   * Size of root dir block
   * We know this will be a minimum size entry for each dir, because
   * the directory names are all 3 characters long.
   */
  scratch[7] = 4 + (no_dirs * 28);
  /* Flags */
  scratch[8] = (1<<8);
  /* Reserved word */
  scratch[9] = 0;
  /* "$", padded with zeroes */
  scratch[10] = 0x00000024;

  /* Header for the next directory */
  scratch[11] = 0x24524944;
  scratch[12] = 12 + (no_dirs * 28);
  scratch[13] = 12 + (no_dirs * 28);

  fwrite(scratch, 4, 14, fptr);

  /* First location after the root dir */
  offset = 44 + 12 + (no_dirs * 28);

  /* Actually write the root dir */
  for (n = 0; n < no_dirs; n++)
  {
    char dirname[4] = "[ ]";
    scratch[0] = offset;
    scratch[1] = 0xFFF00000;
    scratch[2] = 0;
    scratch[3] = dirs[n].dir_size;
    scratch[4] = (1<<0) | (1<<1) | (1<<4) | (1<<8);
    scratch[5] = 0;

    dirname[1] = dirs[n].letter;
    strncpy((char *) (scratch + 6), dirname, 4);

    /* Write the block to file */
    fwrite(scratch, 4, 7, fptr);

    offset += dirs[n].dir_size + 8 + dirs[n].files_size;
  }

  /* First location after the root dir */
  offset = 44 + 12 + (no_dirs * 28);

  /* Then write the directories (and their contents) */
  for (n = 0; n < no_dirs; n++)
  {
    /* Header for the directory */
    scratch[0] = 0x24524944;
    scratch[1] = dirs[n].dir_size + 8;
    scratch[2] = dirs[n].dir_size + 8;

    fwrite(scratch, 4, 3, fptr);

    offset += dirs[n].dir_size + 8;

    current = dirs[n].pages;
    for (m = 0; m < dirs[n].no_pages; current = current->next, m++)
    {
      size_t filename_len = 0;

      if (current->size == 0)
      {
        scratch[0] = 0;
        scratch[3] = 0;
      }
      else
      {
        scratch[0] = offset;
        scratch[3] = current->size + 8;
        offset += 8 + WORDALIGN(current->size);
      }

      /* They're all text files, so set the filetype to FFF */
      scratch[1] = 0xFFFFFF00;
      scratch[2] = 0x0;
      scratch[4] = (1<<0) | (1<<1) | (1<<4) | (1<<5);
      scratch[5] = 0;

      fwrite(scratch, 4, 6, fptr);

      filename_len = strlen(current->filename);

      fwrite(current->filename, 1, filename_len, fptr);

      /* Write padding */
      scratch[0] = 0;
      fwrite(scratch, 1, WORDALIGN(filename_len + 1) - filename_len, fptr);
    }

    current = dirs[n].pages;
    for (m = 0; m < dirs[n].no_pages; current = current->next, m++)
    {
      if (current->size != 0)
      {
        /* File header */
        fwrite("DATA", 1, 4, fptr);
        scratch[0] = current->size + 8;
        fwrite(scratch, 4, 1, fptr);
        fwrite(current->content, 1, current->size, fptr);

        scratch[0] = 0;
        fwrite(scratch, 1, WORDALIGN(current->size) - current->size, fptr);
      }
    }
  }

  fclose(fptr);

#ifdef RISCOS
  {
    _kernel_swi_regs inout;
    inout.r[0] = 18;
    inout.r[1] = (int) filename;
    inout.r[2] = 0x3d6;
    _kernel_swi(0x8, &inout, &inout);
  }
#endif

}

#endif

