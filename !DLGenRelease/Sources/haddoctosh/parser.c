/* ------------------------------------------------ ajps, 2003-03-04 ---
 * File: parser.c
 * Abstract: Deals with the parser itself.
 * Copyright (c) 2003 Antony Sidwell
 * --------------------------------------------------------------------- */

/*
 * A Note or Two:
 * I don't know how useful this will be to anyone else - frankly, this
 * code is probably tending towards the unmaintainable.  Ho hum.
 *
 * The filename prefix h_01_ is used for header files, to avoid name clashes
 * with actual library objects.
 *
 * The prefix 01 is used for anything else which isn't a valid C identifier.
 * This saves us worrying about clashes between valid identifiers which may
 * need to be looked up, and invalid (but transformed) ones which in theory
 * shouldn't be.
 *
 * The filename prefix 02 is used for anonymous enums, so that we can give
 * them a name (for accounting purposes, as it were).
 *
 * There is some complicated stuff to ensure that any objects with the same
 * name are output on the same page, with all possible names then pointing
 * to that page.
 */

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include <assert.h>


#include "libxml/parser.h"
#include "libxml/parserInternals.h"
#ifdef RISCOS
//#include "libxml:riscos.h"
#endif
#include "libxml/tree.h"


#include "haddoctosh.h"
#include "output.h"
#include "hparser.h"

void write_structorunion(xmlNode *object, IFILE *out);
void write_enum(xmlNode *node, IFILE *out);
void write_item(xmlNode *item, IFILE *out);
void output_symlink(const xmlChar *from, const xmlChar *to);

const char *objtype_names[objtype_SIZE] =
{
  "Function",
  "Function-like macro",
  "Macro",
  "Enumerated constant",
  "Type",
  "Variable",
  "Anonymous enum",
  "Unknown"
};


int stricmp(const char *s1, const char *s2)
{
  char ch1 = 0,
       ch2 = 0;

  for(;;)
  {
    if ((*s1 == 0) && (*s2 == 0))
      return 0; /* s1 and s2 are equal */

    ch1 = toupper((unsigned char) *s1);
    ch2 = toupper((unsigned char) *s2);

    if (ch1 != ch2) return (int) (ch1 - ch2);
    s1++;
    s2++;
  }
}

/* ------------------------------------------------ ajps, 2003-04-05 ---
 * Takes a node and the name of the attribute of which we wish to extract
 * the contents.  If there is no attribute of that name, it returns NULL.
 * --------------------------------------------------------------------- */
const xmlChar *get_attr(const xmlNode *node, const char *attr_name)
{
  xmlAttr *attr = node->properties;

  while (attr != NULL)
  {
    if (strcmp(attr_name, attr->name) == 0)
    {
      return attr->children->content;
    }

    attr = attr->next;
  }

  return NULL;
}

xmlNode *get_nextnode(xmlNode *node, const char *element_name)
{
  while (node != NULL && strcmp(node->name, element_name) != 0)
  {
    node = node->next;
  }

  return node;
}


int compare_filenames(const void *one, const void *two)
{
  xmlNode *const *one_x = one;
  xmlNode *const *two_x = two;

  const char *name1 = get_attr(*one_x, "name");
  const char *name2 = get_attr(*two_x, "name");

  return strcmp(name1, name2);
}

enum objtype get_objecttype(xmlNode *object)
{
  const xmlChar *type = get_attr(object, "type");

  if      (streq(type, "function")) return objtype_FUNC;
  else if (streq(type, "macrofn"))  return objtype_MACROFN;
  else if (streq(type, "macro"))    return objtype_MACRO;
  else if (streq(type, "type"))     return objtype_TYPE;
  else if (streq(type, "global"))   return objtype_GLOBAL;

  return objtype_UNKNOWN;
}


int compare_objectnames(const void *one, const void *two)
{
  const struct object_record *record[2] = {one, two};
  int result;
  const xmlChar *type[2];
  int n;

  result = record[0]->type - record[1]->type;

  /* Same type, compare name. */
  if (result == 0) result = strcmp(record[0]->name, record[1]->name);

  return result;
}

int compare_allobjects(const void *one, const void *two)
{
  const struct object_record *record[2] = {one, two};
  int result;
  const xmlChar *type[2];
  int ranks[2];
  int n;

  ranks[0] = (record[0]->type == objtype_ENUMVAL) ? objtype_UNKNOWN : record[0]->type;
  ranks[1] = (record[1]->type == objtype_ENUMVAL) ? objtype_UNKNOWN : record[1]->type;

  result = ranks[0] - ranks[1];

  /* Same type, compare name. */
  if (result == 0) result = strcmp(record[0]->name, record[1]->name);

  return result;
}

char *filename_makesafe(const unsigned char *src, unsigned char *dest, size_t dest_size)
{
  unsigned int n;

  /* We haven't had to change any characters (yet) */
  int intact = 1;

  for (n = 0; n < dest_size - 1 && *src; src++)
  {
    if (isalnum(*src) || *src == '_')
    {
      dest[n++] = tolower(*src);
    }
    else
    {
      if (intact)
      {
        int copy_size = (n > dest_size - 4) ? dest_size - 4 : n;

        intact = 0;

        memmove(dest + 3, dest, copy_size);
        dest[0] = '0';
        dest[1] = '1';
        dest[2] = '_';

        n += 3;

        if (n > dest_size - 1) n = dest_size - 1;
      }

      switch (*src)
      {
        case ',':
        case ':':
        {
          dest[n++] = '-';
          break;
        }

        case '$':
        case ' ':
        {
          dest[n++] = '_';
          break;
        }
      }
    }
  }

  dest[n] = '\0';

  return dest;
}


void write_safetext(const xmlChar *text, IFILE *out)
{
  while (*text)
  {
    switch (*text)
    {
      case '{':
      case '#':
      case '<':
      case '\\':
      case '_':
      case '/':
      case '*':
      {
        wputc('\\', out);
        wputc(*text, out);
        break;
      }

      default:
      {
        wputc(*text, out);
        break;
      }
    }
    text ++;
  }
}

/*
 * Takes a single piece of text and writes out the link text for it.
 */
void write_link(const xmlChar *from, const xmlChar *to, IFILE *out)
{
  char safe_filename[MAX_FILENAME];

  if (from == NULL) from = to;

  wputc('<', out);

  write_safetext(from, out);
  wputs("=>", out);

  filename_makesafe(to, safe_filename, sizeof safe_filename);
  wputs(safe_filename, out);

  wputc('>', out);
}

void write_description(xmlNode *description, int no_tabs, IFILE *out)
{
  int first_p = 0;
  int n;

  if (description != NULL)
  {
    xmlNode *current = description->children;
    while (current != NULL)
    {
      if (strcmp(current->name, "p") == 0)
      {
        xmlNode *contents = current->children;

        /* Put empty lines between paragraphs */
        if (first_p == 0) first_p = 1;
        else
        {
          wputc('\n', out);
          for (n = 0; n < no_tabs; n++)
          {
            wputc('\t', out);
            if (n + 1 < no_tabs) wputc(' ', out);
          }
        }

        while (contents != NULL)
        {
          if (streq(contents->name, "text"))
          {
            if (contents->content != NULL)
            {
              write_safetext(contents->content, out);
            }
          }
          else if (streq(contents->name, "link"))
          {
            write_link(NULL, contents->children->content, out);
          }

          contents = contents->next;
        }
        wputc('\n', out);
      }
      else if (strcmp(current->name, "list") == 0)
      {
        xmlNode *contents = current->children;
        int first_item = 1;

        /* Put empty lines between paragraphs */
        if (first_p == 0) first_p = 1;
        else wputc('\n', out);

        wputs("#Indent 2\n", out);
        while (contents != NULL)
        {
          if (streq(contents->name, "item"))
          {
            if (first_item) first_item = 0;
            else wputc('\n', out);

            for (n = 0; n < no_tabs; n++)
            {
              wputc('\t', out);
              if (n + 1 < no_tabs) wputc(' ', out);
            }
            wputs("�\t", out);
            write_description(contents, no_tabs + 1, out);
          }
          contents = contents->next;
        }
        wputs("#Indent\n", out);
      }
      current = current->next;
    }
  }
}

#define write_typename(type, out) write_safetext(objtype_names[type], out)


int write_root(xmlNode *library, xmlNode *files[], int no_files, IFILE *out)
{
  const char *libname = get_attr(library, "name");
  const char *version = get_attr(library, "version");
  xmlNode *info = get_nextnode(library->children, "info");
  int n;

  if (libname == NULL) libname = "Anonymous library";

  wputs(libname, out);
  wputs(" manual\n", out);
  /* Parent?  Nah. */

  /* Heading on page */
  wputs("{fH3}", out);
  write_safetext(libname, out);
  wputs("{f}\n", out);

  /* Can put in version extraction, etc. if we want here */
  if (version)
  {
    wputs("{/}Version ", out);
    write_safetext(version, out);
    wputs("{/}\n", out);
  }

  wputs("#Line\n", out);

  wputs("#Table Columns 3\n", out);

  /* List the contents */
  for (n = 0; n < no_files; n++)
  {
    const char *filename = get_attr(files[n], "name");
    char safe_filename[MAX_FILENAME];

    filename_makesafe(filename, safe_filename, sizeof safe_filename);

    wputc('<', out);
    wputs(filename, out);
    wputs("=>h_", out);
    wputs(safe_filename, out);
    wputs(">\n", out);
  }

  wputs("#EndTable\n", out);

  /* Wow!  A footer!  Can I have one too? */
  wputs("#Line\n", out);
  wputs("#Align Centre\n", out);
  wputs("<About this manual=>!about>\n", out);

  /* Any other <info> related stuff */
  if (info != NULL)
  {
    xmlNode *history = get_nextnode(info->children, "history");

    wputs("<About the library=>!aboutlib>\n", out);
    if (history) wputs("<Changelog=>!history>\n", out);
  }

  return 0;
}


int write_header(xmlNode *library, xmlNode *file, struct object_record *objects, int no_objects, IFILE *out)
{
  const char *libname = get_attr(library, "name");
  const char *filename = get_attr(file, "name");

  xmlNode *overview;

  enum objtype current_type;

  int n;

  if (filename == NULL)
  {
    fprintf(stderr, "Error (screwup): file element has no name attribute\n");
    exit(EXIT_FAILURE);
  }

  if (libname == NULL) libname = "Anonymous library";

  wputs(libname, out);
  wputc(':', out);
  wputs(filename, out);
  wputc('\n', out);

  /* Parent is root. */
  wputs("#Parent !root\n", out);

  /* Heading on page */
  wputs("{fH3}", out);
  write_safetext(libname, out);
  wputc(':', out);
  write_safetext(filename, out);
  wputs("{f}\n", out);

  /* Can put in version extraction, etc. if we want here */

  wputs("#Line\n", out);

  overview = file->children;
  while (overview != NULL)
  {
    if (strcmp(overview->name, "description") == 0)
    {
      const char *title = get_attr(overview, "title");
      if (title != NULL && strcmp(title, "overview") == 0) break;
    }
    overview = overview->next;
  }

  write_description(overview, 0, out);
  wputc('\n', out);

  for (n = 0; n < no_objects; n++)
  {
    if (objects[n].type != objtype_ANON)
    {
      if (objects[n].type != current_type)
      {
        current_type = objects[n].type;

        if (n != 0)
        {
          wputs("#EndTable\n", out);
          wputs("#Indent\n", out);
          wputc('\n', out);
        }

        write_typename(objects[n].type, out);

        /* Check for pluralisation */
        if (n + 1 < no_objects && objects[n].type == objects[n + 1].type)
          wputc('s', out);

        wputc('\n', out);

        wputs("#Indent 3\n", out);
        wputs("#Table Columns 2\n", out);
      }

      write_link(NULL, objects[n].name, out);
      wputc('\n', out);
    }
  }

  if (no_objects > 0)
  {
    wputs("#EndTable\n", out);
    wputs("#Indent\n", out);
  }

  return 0;
}

void write_type(xmlNode *type, IFILE *out)
{
  const xmlChar *qualifier = get_attr(type, "qualifier");

  if (type->children == NULL)
  {
    fprintf(stderr, "Error (situation not handled): empty type element\n");
    exit(EXIT_FAILURE);
  }

  if (qualifier != NULL)
  {
    write_safetext(qualifier, out);
    wputc(' ', out);
  }

  if (strcmp(type->children->name, "link") == 0)
  {
    type = type->children;

    write_link(NULL, type->children->content, out);
  }
  else
  {
    write_safetext(type->children->content, out);
  }
}

void write_function(xmlNode *func, IFILE *out)
{
  xmlNode *current;
  xmlNode *node;
  xmlNode *item;
  int is_fn_ptr = 0;

  current = get_nextnode(func->children, "returns");
  if (current == NULL)
  {
    fprintf(stderr, "Error (situation not handled): function element with no returns element.\n");
    exit(EXIT_FAILURE);
  }

  /* Write return type */
  node = get_nextnode(current->children, "type");
  if (node != NULL)
  {
    write_type(node, out);
    wputc(' ', out);
  }

  node = get_nextnode(current->children, "item");
  if (node != NULL) write_item(node, out);

  /* Write the function "name", and any pointery stuff */
  item = get_nextnode(func->children, "item");
  if (item != NULL)
  {
    node = get_nextnode(item->children, "ptr");
    if (node != NULL) is_fn_ptr = 1;
  }

  if (is_fn_ptr) wputc('(', out);
  if (item != NULL) write_item(item, out);
  if (is_fn_ptr) wputc(')', out);

  wputs(" (", out);

  current = get_nextnode(func->children, "parameter");

  while (current != NULL)
  {
    node = get_nextnode(current->children, "type");
    if (node != NULL)
    {
      write_type(node, out);
    }

    node = get_nextnode(current->children, "item");
    if (node != NULL)
    {
      wputc(' ', out);
      write_item(node, out);
    }

    current = get_nextnode(current->next, "parameter");

    if (current != NULL) wputs(", ", out);
  }

  wputs(");", out);
}

#define write_typedef(node, out) write_member(node, out)
#define write_variable(node, out) write_member(node, out)

/* ------------------------------------------------ ajps, 2003-04-09 ---
 * Works for members if unions and structs, but also for typedefs and
 * variables.
 * --------------------------------------------------------------------- */
void write_member(xmlNode *member, IFILE *out)
{
  xmlNode *current = member->children;

  if (strcmp(member->name, "typedef") == 0)
  {
    if (get_nextnode(current, "item") != NULL ||
        get_nextnode(current, "function") != NULL)
    {
      write_safetext("typedef ", out);
    }
  }
  else if (strcmp(member->name, "variable") == 0)
  {
    write_safetext("extern ", out);
  }

  while (current != NULL)
  {
    if (strcmp(current->name, "item") == 0)
    {
      /* ignore items here, deal with them later */
    }
    else if (strcmp(current->name, "function") == 0)
    {
      write_function(current, out);
    }
    else
    {
      if (strcmp(current->name, "enum") == 0)
      {
        write_enum(current, out);
      }
      else if (strcmp(current->name, "type") == 0)
      {
        write_type(current, out);
        wputc(' ', out);
      }
      else if (strcmp(current->name, "struct") == 0 ||
               strcmp(current->name, "union") == 0)
      {
        write_structorunion(current, out);
      }
      else if (strcmp(current->name, "text") == 0)
      {
        /* Should all be empty, and can be ignored */
      }
      else
      {
        fprintf(stderr, "Error (situation not handled): illegal element '%s' in %s element\n", current->name, member->name);
        exit(EXIT_FAILURE);
      }
    }

    current = current->next;
  }

  current = get_nextnode(member->children, "item");
  while (current != NULL)
  {
    if (strcmp(current->name, "item") == 0)
    {
      wputc(' ', out);
      write_item(current, out);

      current = get_nextnode(current->next, "item");

      if (current != NULL)
      {
        wputs(",", out);
      }
      else
      {
        wputc(';', out);
      }
    }
  }

}

void write_item(xmlNode *item, IFILE *out)
{
  xmlNode *current = item->children;
  const xmlChar *attr;
  int in_ptr = 0;

  if (current != NULL)
  {
    /*
     * We write the * and ( on the lhs of the name.
     * This means we go into the nest of <ptr> and <array> elements
     * and then apply rules based on the closing tags.
     */
    while (current->children != NULL)
    {
      current = current->children;
    }

    while (current != item)
    {
      if (streq(current->name, "ptr"))
      {
        write_safetext("*", out);
        attr = get_attr(current, "qualifier");
        if (attr != NULL)
        {
          write_safetext(attr, out);
          wputc(' ', out);
        }
      }
      else if (streq(current->name, "array"))
      {
        /* Check if there is a parent "ptr" element */
        xmlNode *ancestor = current->parent;

        while (ancestor != item && strcmp(ancestor->name, "ptr") != 0)
        {
          ancestor = ancestor->parent;
        }

        if (ancestor != item)
        {
          wputc('(', out);
        }
      }

      current = current->parent;
    }
  }

  /* Output the actual variable/function/whatever name */
  attr = get_attr(item, "name");

  /* We *do* have items without names, for return values, etc. */
  if (attr != NULL) write_safetext(attr, out);

  /* Write a bitfield specifier if present */
  attr = get_attr(item, "bitfield");
  if (attr != NULL)
  {
    write_safetext(" : ", out);
    write_safetext(attr, out);
  }

  /* Now write the closing brackets and array specifiers */
  current = item->children;

  while (current != NULL)
  {
    if (strcmp(current->name, "array") == 0)
    {
      if (in_ptr)
      {
        wputc(')', out);
      }

      /* The actual array specifier - the [] bit */
      attr = get_attr(current, "size");
      if (attr != NULL)
      {
        write_safetext(attr, out);
      }
    }

    current = current->children;
  }
}

void write_macro(xmlNode *macro, IFILE *out)
{
  wputs("{fCode}", out);
  write_safetext(macro->children->content, out);
  wputs("{f}", out);
}

void write_structorunion(xmlNode *object, IFILE *out)
{
  const xmlChar *name = get_attr(object, "name");
  xmlNode *child;

  /* Either struct or union */
  write_safetext(object->name, out);

  /* Put type name where appropriate */
  if (name != NULL)
  {
    wputc(' ', out);
    write_safetext(name, out);
  }

  write_safetext("\n{", out);
  wputs("\n#Indent +3\n", out);

  /* Output the members */
  child = get_nextnode(object->children, "member");
  while (child != NULL)
  {
    write_member(child, out);

    child = get_nextnode(child->next, "member");

    if (child != NULL)
    {
      wputs("\n", out);
    }
  }

  wputs("\n#Indent\n", out);
  write_safetext("}", out);
}

void write_enumval(xmlNode *enumval, IFILE *out)
{
  const xmlChar *name = get_attr(enumval, "name");
  xmlNode *child = enumval->children;

  if (name == NULL)
  {
    fprintf(stderr, "Error (situation not handled): enum-val without name\n");
    exit(EXIT_FAILURE);
  }

  write_safetext(name, out);

  if (child != NULL) wputs(" = ", out);

  while (child != NULL)
  {
    if (streq(child->name, "text")) write_safetext(child->content, out);
    child = child->next;
  }
}

void write_enum(xmlNode *node, IFILE *out)
{
  const xmlChar *name = get_attr(node, "name");
  xmlNode *child;

  wputs("enum", out);

  /* Put type name where appropriate */
  if (name != NULL)
  {
    wputc(' ', out);
    write_safetext(name, out);
  }

  wputs("\n\\{\n#Indent +3\n", out);

  child = get_nextnode(node->children, "enum-val");

  /* Output the enumvals */
  while (child != NULL)
  {
    write_enumval(child, out);

    child = get_nextnode(child->next, "enum-val");

    if (child != NULL)
    {
      wputs(",\n", out);
    }
  }

  wputs("\n#Indent\n\\}", out);
}


void write_definition(xmlNode *object, IFILE *out)
{
  xmlNode *node = object->children;

  while (node != NULL)
  {
    if (strcmp(node->name, "function") == 0)
    {
      wputs("Syntax:\t", out);
      write_function(node, out);
    }
    else if (strcmp(node->name, "macro") == 0)
    {
      wputs("#Wrap Off\n", out);
      write_macro(node, out);
      wputs("\n#Wrap", out);
    }
    else if (strcmp(node->name, "typedef") == 0)
    {
      write_typedef(node, out);
  }
    else if (strcmp(node->name, "variable") == 0)
    {
      wputs("#Wrap Off\n", out);
      write_variable(node, out);
      wputs("\n#Wrap", out);
    }

    node = node->next;
  }

}

/* ------------------------------------------------ ajps, 2003-04-10 ---
 * Takes two "unsafe" names and outputs a link from "from" to "to".
 * --------------------------------------------------------------------- */
void output_symlink(const xmlChar *from, const xmlChar *to)
{
  IFILE *fptr;
  char filename[(MAX_FILENAME * 2) + 1] = "";

  if (from == NULL || to == NULL) return;

  /* No need to link from something to itself */
  if (stricmp(from, to) == 0) return;

  filename_makesafe(from, filename, MAX_FILENAME);
  strcat(filename, ">");
  filename_makesafe(to, filename + strlen(filename), MAX_FILENAME);

  fptr = open_output(filename);
  close_output(fptr);
}

void mark_done(struct object_record *objects, int no_objects, int which)
{
  objects[which].output = TRUE;
}

void output_object(struct object_record *objects, int no_objects, int first)
{
  IFILE *output;
  int n;

  xmlChar *ptr;
  int is_identifier;

  xmlNode *current;
  const xmlChar *real_name = NULL;

  char safe_filename[MAX_FILENAME] = "";
  const char *filename;

  int *object_list = NULL;
  int no_list_objects = 0;

  int no_enumvals = 0;

  /* Change this to be a safe way of getting the sensible object name */
  real_name = objects[first].objectname;

  object_list = malloc(sizeof *object_list);
  if (object_list == NULL)
  {
    fprintf(stderr, "Error (memory): Couldn't allocate memory for object list.\n");
    exit(EXIT_FAILURE);
  }

  mark_done(objects, no_objects, first);
  object_list[0] = first;
  if (objects[first].type == objtype_ENUMVAL) no_enumvals++;
  no_list_objects = 1;

  /* Build up a list of the array numbers which we should output on this page */
  for (n = 0; n < no_list_objects; n++)
  {
    int m;
    for (m = 0; m < no_objects; m++)
    {
      if (objects[m].output == FALSE)
      {
        if (stricmp(objects[m].name, objects[object_list[n]].name) == 0/* ||
            objects[m].object == objects[object_list[n]].object*/)
        {
/*          fprintf(stderr, "%s matches %s\n", objects[m].name, objects[object_list[n]].name);*/

          /* Add it to the list, move on */
          no_list_objects++;

          object_list = realloc(object_list, sizeof *object_list * no_list_objects);

          if (object_list == NULL)
          {
            fprintf(stderr, "Error (memory): Couldn't allocate memory for object list.\n");
            exit(EXIT_FAILURE);
          }

          object_list[no_list_objects - 1] = m;

          if (objects[m].type == objtype_ENUMVAL) no_enumvals++;

          /* This stops us comparing objects we've already matched */
          mark_done(objects, no_objects, m);
        }
      }
    }
  }

  if (no_list_objects == 1 && no_enumvals == 1)
  {
    output_symlink(objects[first].name, real_name);
  }
  else
  {
    if (no_enumvals > 0 && no_enumvals == no_list_objects)
    {
      real_name = objects[first].name;
    }
    else
    {
      /* Output the "symlinks" */
      for (n = 0; n < no_list_objects; n++)
      {
        output_symlink(objects[object_list[n]].name, real_name);
      }
    }

    /* Output the actual page of the manual */
    filename_makesafe(real_name, safe_filename, sizeof safe_filename);
    output = open_output(safe_filename);

    if (objects[first].type != objtype_ANON) wputs(real_name, output);
    else wputs("Anonymous enum", output);
    wputc('\n', output);

    filename = get_attr(objects[first].object->parent, "name");

    if (filename)
    {
      strcpy(safe_filename, "h_");
      filename_makesafe(filename, safe_filename + 2, (sizeof safe_filename) - 2);

      wputs("#Parent ", output);
      wputs(safe_filename, output);
      wputc('\n', output);
    }

    for (n = 0; n < no_list_objects; n++)
    {
      wputs("{fh6}", output);
      write_typename(objects[object_list[n]].type, output);

      filename = get_attr(objects[object_list[n]].object->parent, "name");

      if (filename)
      {
        strcpy(safe_filename, "h_");
        filename_makesafe(filename, safe_filename + 2, (sizeof safe_filename) - 2);

        wputs(" from header ", output);
        write_link(filename, safe_filename, output);
      }

      wputs(":{f}", output);

      wputc('\n', output);
      wputc('\n', output);

      if (objects[object_list[n]].type == objtype_ENUMVAL)
      {
        write_safetext(objects[object_list[n]].name, output);
        write_safetext(" is defined as part of ", output);

        if (get_nextnode(objects[object_list[n]].object->children, "name") == NULL)
        {
          write_link("an anonymous enum", objects[object_list[n]].objectname, output);
        }
        else
        {
          write_link(NULL, objects[object_list[n]].objectname, output);
        }

        wputc('\n', output);
      }
      else if (objects[object_list[n]].type != objtype_UNKNOWN)
      {
        current = get_nextnode(objects[object_list[n]].object->children,
                               "definition");

        write_definition(current, output);

        wputs("\n", output);

        current = get_nextnode(objects[object_list[n]].object->children,
                               "description");
        if (current != NULL)
        {
          wputs("\n", output);
          write_description(current, 0, output);
        }
      }

      /* Stuff to go between different objects on the same page */
      if (n < no_list_objects - 1)
      {
        /* Reset tab stops - there's no reason for them to carry between objects */
        wputs("#Tab\n", output);

        /* Line as a spacer */
        wputs("#Line\n", output);
      }
    }

    close_output(output);
  }

  free(object_list);
}



int get_objects(xmlNode *r00t, xmlNode *start, int *n, struct object_record *objects)
{
  int no_names = 0;

  while (start != NULL)
  {
    if (streq(start->name, "name"))
    {
      if (objects != NULL)
      {
        objects[*n].name = start->children->content;
        objects[*n].object = r00t;
        objects[*n].type = get_objecttype(r00t);
        objects[*n].output = FALSE;
      }
      (*n)++;
      no_names++;
    }
    else if (streq(start->name, "enum-val"))
    {
      if (objects != NULL)
      {
        objects[*n].name = get_attr(start, "name");
        objects[*n].object = r00t;
        objects[*n].type = objtype_ENUMVAL;
        objects[*n].output = FALSE;
      }
      (*n)++;
    }
    else if (start->children)
    {
      no_names += get_objects(r00t, start->children, n, objects);
    }

    start = start->next;
  }

  return no_names;
}


/* ------------------------------------------------ ajps, 2003-04-05 ---
 * Parse the file.
 * --------------------------------------------------------------------- */
int assemble_manual(xmlDoc *parsed, const char *ofilename)
{
  xmlNode *library;
  xmlNode *info;
  IFILE *output;

  int no_files = 0;
  xmlNode **files;
  xmlNode *cur_file;

  int n;

  int no_objects = 0;
  struct object_record *objects = NULL;
  struct object_record *temp;

  library = parsed->children;

  /* Find the first "library" element in the file - remove this later */
  while (library != NULL && strcmp(library->name, "library"))
  {
    library = library->next;
  }

  if (library == NULL) return 1;

  info = get_nextnode(library->children, "info");

  /* Count the number of files in the library */
  cur_file = library->children;
  while (cur_file != NULL)
  {
    if (strcmp(cur_file->name, "file") == 0)
    {
      no_files++;

      if (get_attr(cur_file, "name") == NULL)
      {
        fprintf(stderr, "Error (situation not handled): file element without name\n");
        exit(EXIT_FAILURE);
      }
    }

    cur_file = cur_file->next;
  }

  if (no_files == 0)
  {
    fprintf(stderr, "Error (situation not handled): No file elements in the library\n");
    exit(EXIT_FAILURE);
  }

  /* Allocate space for the list of files */
  files = malloc(no_files * sizeof *files);
  if (files == NULL)
  {
    fprintf(stderr, "Error (memory): Couldn't claim memory for file list\n");
    exit(EXIT_FAILURE);
  }

  /* Build the array of file nodes */
  for (n = 0, cur_file = library->children;
       n < no_files;
       n++, cur_file = cur_file->next)
  {
    while (cur_file != NULL && strcmp(cur_file->name, "file"))
    {
      cur_file = cur_file->next;
    }

    if (cur_file == NULL)
    {
      fprintf(stderr, "Error (screwup): Appear to have miscounted the number of file elements in the library\n");
      exit(EXIT_FAILURE);
    }

    files[n] = cur_file;
  }

  /* Sort the files into alphabetical order */
  qsort(files, no_files, sizeof(xmlNode *), compare_filenames);

  /* The root (index page) */
  output = open_output("!root");
  write_root(library, files, no_files, output);
  close_output(output);

  /* All of the header file pages - we can build an object index as we go. */
  for (n = 0; n < no_files; n++)
  {
    int no_file_objects = 0;
    int m;

    xmlNode *cur_object;

    const char *file = get_attr(files[n], "name");
    char safe_filename[MAX_FILENAME] = "h_";

    filename_makesafe(file, safe_filename + 2, (sizeof safe_filename) - 2);

    /* Count the number of names for objects in this file */
    cur_object = files[n]->children;
    while (cur_object != NULL)
    {
      if (streq(cur_object->name, "object"))
      {
        if (get_attr(cur_object, "type") == NULL)
        {
          fprintf(stderr, "Error (situation not handled): object without type\n");
          exit(EXIT_FAILURE);
        }

        if (get_objects(cur_object, cur_object->children, &no_file_objects, NULL) == 0)
        {
          no_file_objects++;
        }
      }

      cur_object = cur_object->next;
    }

/*    printf("%i different names so far\n", no_objects);
    printf("%i different names in the file\n", no_file_objects);*/

    temp = realloc(objects, (no_objects + no_file_objects) * sizeof *objects);
    if (temp == NULL)
    {
      fprintf(stderr, "Error (memory): No space to store object index (at object %i)\n", (no_objects + no_file_objects));
      exit(EXIT_FAILURE);
    }

    objects = temp;

    /* Fill in the details of the objects */
    cur_object = files[n]->children;
    m = no_objects;
    while (cur_object != NULL)
    {
      if (streq(cur_object->name, "object"))
      {
        int prev_m = m;
        int realname = m;

        if (get_objects(cur_object, cur_object->children, &m, objects) == 0)
        {
          const xmlChar *id = get_attr(cur_object, "id");
          char *newname;

          if (id == NULL)
          {
            fprintf(stderr, "Error (malformed source): Anonymous object has no 'id' attribute\n");
            exit(EXIT_FAILURE);
          }

          newname = malloc(strlen(id) + 4);
          if (newname == NULL)
          {
            fprintf(stderr, "Error (memory): No space to store object object name\n");
            exit(EXIT_FAILURE);
          }

          strcpy(newname, "02_");
          strcat(newname, id);

          realname = m;
          objects[m].name = newname;
          objects[m].object = cur_object;
          objects[m].type = objtype_ANON;
          objects[m].output = FALSE;
          m++;
        }

        for (;prev_m < m; prev_m++) objects[prev_m].objectname = objects[realname].name;
      }

      cur_object = cur_object->next;
    }

    /* Sort the objects from this file by type, then by name */
    if (no_file_objects > 0)
    {
      qsort(objects + no_objects, no_file_objects, sizeof *objects,
            compare_objectnames);
    }

    output = open_output(safe_filename);
    write_header(library, files[n], objects + no_objects, no_file_objects, output);
    close_output(output);



    no_objects += no_file_objects;
  }

  /* Sort all the objects by type (enumvals last), then by name */
  if (no_objects > 0)
  {
    qsort(objects, no_objects, sizeof *objects, compare_allobjects);
  }


  for (n = 0; n < no_objects; n++)
  {
    /* Enumvals are a special case, as they should only be links to the enum */
    if (objects[n].output == FALSE/* && objects[n].type != objtype_ENUMVAL*/)
    {
/*      fprintf(stderr, "Outputting object %s (%s)\n", objects[n].name, objects[n].objectname);*/
      output_object(objects, no_objects, n);
    }
  }

  /* Write the "about this manual" bit */
  output = open_output("!about");
  wputs("About this manual\n", output);
  wputs("#Parent !root\n", output);
  wputs("This manual is generated automatically from the library's header files using haddoc (to parse the headers into XML) and haddoctosh (to take this XML and output a Stronghelp manual).\n\n", output);
  wputs("Both of these utilities and their source code can be obtained from the <DeskLib SVN repository=>#URL http://www.riscos.info/index.php/Source_repositories_overview#DeskLib>.\n\n", output);
  wputs("More information about haddoc, haddoctosh and DeskLib can be found at the <DeskLib website=>#URL http://www.riscos.info/index.php?title=DeskLib>.\n", output);
  close_output(output);


  /* Any other <info> related stuff */
  if (info != NULL)
  {
    xmlNode *history = get_nextnode(info->children, "history");
    xmlNode *section = NULL;

    const char *libname = get_attr(library, "name");
    if (libname == NULL) libname = " the library";

    output = open_output("!aboutlib");
    wputs("About ", output);
    write_safetext(libname, output);
    wputc('\n', output);

    wputs("#Parent !root\n", output);

    section = get_nextnode(info->children, "contact");
    if (section)
    {
      const char *name, *email;
      name = get_attr(section, "name");
      email = get_attr(section, "address");

      wputs("{fH4}Contact{f}\n", output);
      wputs("If you find any bugs or problems in ", output);
      write_safetext(libname, output);
      wputs(", or if you have any other comments or suggestions to make, then please contact <", output);
      write_safetext(name, output);
      wputs("=>#URL mailto:", output);
      wputs(email, output);
      wputs(">.\n", output);
    }

    section = get_nextnode(info->children, "section");
    while (section != NULL)
    {
      const char *title = get_attr(section, "title");
      wputs("\n{fH4}", output);
      write_safetext(title, output);
      wputs("{f}\n", output);

      write_description(section, 0, output);

      section = get_nextnode(section->next, "section");
    }

    close_output(output);

    if (history)
    {
      output = open_output("!history");
      wputs("History\n", output);
      wputs("#Parent !root\n", output);

      section = get_nextnode(history->children, "revision");
      while (section != NULL)
      {
        const char *title = get_attr(section, "title");
        xmlNode *change = get_nextnode(section->children, "change");

        wputs("{fH4}", output);
        write_safetext(title, output);
        wputs("{f}\n", output);
        wputs("#Indent 2\n", output);

        while (change != NULL)
        {
          xmlNode *contents;

          wputs("�\t", output);

          contents = get_nextnode(change->children, "text");
          while (contents != NULL)
          {
            if (contents->content != NULL)
            {
              write_safetext(contents->content, output);
            }
            contents = get_nextnode(contents->next, "text");
          }

          wputc('\n', output);
          change = get_nextnode(change->next, "change");
        }

        wputs("#Indent\n", output);

        section = get_nextnode(section->next, "revision");
        if (section) wputc('\n', output);
      }

      close_output(output);
    }
  }

  write_manual(ofilename);

  return 0;
}
