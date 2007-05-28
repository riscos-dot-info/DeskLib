/* ------------------------------------------------ ajps, 2002/11/19 ---
 * File: haddoc.c
 * Abstract: Program to generate xml documentation based on the contents
 * of Desklib-style h_doc header files.
 * Copyright (c) 2003 Antony Sidwell 
 * --------------------------------------------------------------------- */
/* Compile-time options */
/* #define ANALYSE_FILES */
/* #define ONE_FILE */

/* ISO stuff */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* PCRE stuff */
#include "pcre.h"

/* For this app */
#include "regexps.h"
#include "haddoc.h"
#include "input.h"
#include "output.h"
#include "cl.h"

#ifdef RISCOS_TIMINGS
  #include "DeskLib:SWI.h"
#endif

/* Unique id for each object we store */
int global_id = 0;

/* We may as well make these globals, it saves passing them all over the place */
char linebuff[LINEBUFF_SIZE];
int ovector[OVECTOR_SIZE];

/* We'll assign this to be an array of files */
object_info *files = NULL;
int no_files = 0;
const char **filenames = NULL;

/*
 * Add a sub-object to the given object.
 */
int add_object(object_info *object, object_type type)
{
	object_info *new;

	new = realloc(object->contents, (object->no_parts + 1) * sizeof(object_info));
	if (new == NULL)
	{
		cl_error(cl_FATAL, "Couldn't claim memory to add object.\n");
	}
	object->contents = new;

	object->contents[object->no_parts].type = type;
	object->contents[object->no_parts].no_names = 0;
	object->contents[object->no_parts].definition = NULL;
	object->contents[object->no_parts].description = NULL;
	object->contents[object->no_parts].no_parts = 0;
	object->contents[object->no_parts].contents = NULL;
	object->contents[object->no_parts].part_of = object;
	object->contents[object->no_parts].id = global_id++;

	return object->no_parts++;
}

/*
 * Effectively concats the contents of 'to_add' onto the end of 'text',
 * allocating more space if needed.
 */
BOOL add_text_n(char **text, const char *to_add, size_t len)
{
	char *new;
	unsigned int new_size;
	unsigned int cur_len;

/*	fprintf(stderr, "Adding '%s'\n", to_add); */

	if (len == 0 || strlen(to_add) < len) len = strlen(to_add);

	if (*text == NULL)
	{
		cur_len = 0;
		new_size = len + 1;
	}
	else
	{
		cur_len = strlen(*text);
		new_size = (cur_len + len + 1);
	}

	new = realloc(*text, new_size);
	if (new == NULL) return FALSE;

	if (*text == NULL) strcpy(new, "");

	*text = new;

	strncat(*text, to_add, len);
	(*text)[cur_len + len] = '\0';

	return TRUE;
}

/*
 * Adds a name to the list of those stored for this object.
 */
void add_name_n(object_info *object, const char *name, size_t len)
{
	if (len == 0) len = strlen(name);

	if (object->no_names >= MAX_NAMES_PER_OBJECT)
	{
		cl_error(cl_FATAL, "Number of names for an object exceeded (limit is %i names)\nFailed at object name '%s'\n", MAX_NAMES_PER_OBJECT, name);
	}

	object->names[object->no_names] = make_string_n(name, len);

	if (object->names[object->no_names] == NULL)
	{
		cl_error(cl_FATAL, "Couldn't allocate memory for object name ('%s')\n", name);
	}

	object->no_names++;
}


/*
 * Allocate memory for a string, and copy said string into the allocated
 * memory.  In this version, we copy the first n characters, and then
 * zero-terminate.
 */
char *make_string_n(const char *string, size_t n)
{
	size_t len = strlen(string);
	char *ptr;

	/* Copying an empty string is surprisingly easy */
	if (string == NULL) return (NULL);

	if (n != 0 && n < len) len = n;

	ptr = malloc(len + 1);
	if (ptr == NULL) return (NULL);

	memcpy(ptr, string, len);
	ptr[len] = '\0';

	return ptr;
}

char *make_result_string(int number)
{
	return (make_string_n(linebuff + ovector[number * 2], ovector[(number * 2) + 1] - ovector[number * 2]));
}

void copy_result_string(const char *result, int number, char *string)
{
	if (ovector[number * 2] != -1 && ovector[(number * 2) + 1] != -1)
	{
		strncpy(string, result + ovector[number * 2], ovector[(number * 2) + 1] - ovector[number * 2]);
		string[ovector[(number * 2) + 1] - ovector[number * 2]] = '\0';
	}
	else
	{
		strcpy(string, "");
	}
}


/*
 * Deallocate memory for a string.
 */
void free_string(char *ptr)
{
	/* Free the memory */
	free(ptr);
}

/* deallocate a text block */
void free_text(char **ptr)
{
	/* Free the memory */
	free(*ptr);

	*ptr = NULL;
}


/* ------------------------------------------------ ajps, 2002-11-20 ---
 * Removes comments and condenses whitespace in a block of text.
 * --------------------------------------------------------------------- */
void condense_whitespace(char *text)
{
	char *read;
	char *write;
	BOOL in_comment = FALSE;
	BOOL in_space = FALSE;

	read = text;
	write = text;

	while (*read != '\0')
	{
		if (read[0] == '/' && read[1] == '*')
		{
			read += 2;

			while (*read != '\0' && !(read[0] == '*' && read[1] == '/'))
			{
				read++;
			}

			if (read[0] == '*' && read[1] == '/') read += 2;
		}
		else
		{
			*(write++) = *(read++);
		}
	}

	*write = '\0';

	/* Now squash the spaces */
	read = text;
	write = text;

	while (*read != '\0' && isspace(*read) != 0) read++;

	while (*read != '\0')
	{
		if (isspace(*read) != 0)
		{
			*(write++) = ' ';
			read++;
			while (*read != '\0' && isspace(*read) != 0) read++;
		}
		else
		{
			*(write++) = *(read++);
		}
	}
	*write = '\0';
}

/* -------------------------------------------------- ajps, 23/02/03 ---
 * Given a function/function type declaration, it finds the start of the
 * parameters - in fact, the location of the bracket marking the start
 * of the parameters.  If it cannot be found, NULL is returned.
 * --------------------------------------------------------------------- */
const char *find_params(const char *definition, size_t len)
{
	int brackets = 1;
	const char *end = definition + len - 1;
	const char *start;

	while (end != NULL && *end != ')' && end > definition)
	{
		if (!isspace(*end) && *end != ';' && *end != ',')
		{
			end = NULL;
		}
		else
		{
			end--;
		}
	}

	start = end;

	if (start != NULL)
	{
	    start--;

	    while (brackets > 0 && start > definition)
	    {
	    	if (*start == ')')
	    	{
	    		brackets++;
	    	}
	    	else if (*start == '(')
	    	{
	    		brackets--;
	    	}

	    	start--;
	    }
	}

	if (start == definition) start = NULL;


	/* We want to return a pointer to the '(' itself */
	return (start == NULL) ? NULL : start + 1;
}

/* -------------------------------------------------- ajps, 15/01/03 ---
 * This checks if there for qualifiers at *text, sets the appropriate
 * bits in the return value and advances *text past any qualifiers.
 * Bits are set by being bitwise-ORed with "qualifiers".
 * --------------------------------------------------------------------- */
int check_qualifier(const char **text, int qualifiers)
{
  int no_results;
  int n;
  int res_vector[OVECTOR_SIZE];

  while (pcre_exec(regexes_too[re_QUALIFIERS], NULL, *text,
				   strlen(*text), 0, 0, res_vector, OVECTOR_SIZE) > 0)
  {
	(*text) += (res_vector[1] - res_vector[0]);

	for (n = 0; n < NO_QUALIFIERS; n++)
	{
		if (res_vector[(n + 1) * 2] != -1 && res_vector[(n + 1) * 2 + 1] != -1)
		{
			qualifiers |= 1 << n;
		}
	}
  }

  while (isspace(**text)) (*text)++;

  return qualifiers;
}

void insert_file(FILE *in, FILE *out)
{
	int in_c = getc(in);

	while (in_c != EOF)
	{
		if (putc(in_c, out) == EOF)
		{
			fclose(in);
			fclose(out);
			cl_error(cl_FATAL, "Unable to insert the info file into the output.\n");
		}
		in_c = getc(in);
	}

	fputc('\n', out);
}

#ifdef RISCOS_TIMINGS
  #define SWI_Timer_Start                         0x0490C0
  #define SWI_Timer_Stop                          0x0490C1
  #define SWI_Timer_Value                         0x0490C2
#endif

int main(int argc, char **argv)
{
	FILE *info_f = NULL;

	/* Counters */
	int n = 0, m = 0;

	if (argc > 0) n = cl_parse(argc - 1, argv + 1);

	compile_patterns();

#ifdef RISCOS_TIMINGS
	SWI(0, 0, SWI_Timer_Start);
#endif

	for (n = 0; n < no_files; n++)
	{
		readin_file(&files[n], filenames[n]);
	}

#ifdef RISCOS_TIMINGS
	{
		int s, ms;
		SWI(0, 2, SWI_Timer_Stop, &s, &ms);
		cl_info("Reading took %i.%06i seconds\n", s, ms);
		SWI(0, 0, SWI_Timer_Start);
	}
#endif

	if (filename == NULL)
	{
		out_stream = stdout;
	}
	else
	{
		out_stream = fopen(filename, "w");
		if (out_stream == NULL)
		{
		    cl_error(cl_FATAL, "Couldn't open file %s for writing.\n", filename);
		}
	}

	if (infofile != NULL)
	{
		info_f = fopen(infofile, "r");
		if (info_f == NULL)
		{
		    cl_error(cl_FATAL, "Couldn't open the \"info\" file for reading.\n", infofile);
		}
	}

	fprintf(out_stream, "<?xml version='1.0'?>\n");

	if (options.verbose) cl_info("Building cross-reference index\n");

	build_index();

#ifdef RISCOS_TIMINGS
	{
		int s, ms;
		SWI(0, 2, SWI_Timer_Stop, &s, &ms);
		cl_info("Building index took %i.%06i seconds\n", s, ms);
		SWI(0, 0, SWI_Timer_Start);
	}
#endif

	fprintf(out_stream, "<library name='%s'", libname);

    if (version != NULL) fprintf(out_stream, " version='%s'", version);

    fprintf(out_stream, ">\n\n");

	if (info_f != NULL)
	{
		insert_file(info_f, out_stream);
		fclose(info_f);
	}

	for (n = 0; n < no_files; n++)
	{
		if (options.verbose)
		{
			cl_info("Writing '%s'\n",  files[n].names[0]);
		}
	    output_file(&files[n]);
	}

	fprintf(out_stream, "</library>\n");

	if (options.verbose) cl_info("XML output complete.\n");

#ifdef RISCOS_TIMINGS
	{
		int s, ms;
		SWI(0, 2, SWI_Timer_Stop, &s, &ms);
		cl_info("Writing took %i.%06i seconds\n", s, ms);
	}
#endif

	if (out_stream != stdout) fclose(out_stream);

	free(files);

	return EXIT_SUCCESS;
}
