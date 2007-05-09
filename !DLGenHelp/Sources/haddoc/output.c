/* ------------------------------------------------ ajps, 2002/11/19 ---
 * File: output.c
 * Abstract: The "output" side of haddoc, where the stored definitions
 * and descriptions are output into the XML structures.
 * Copyright (c) 2003 Antony Sidwell 
 * --------------------------------------------------------------------- */

/* Only needs to be set when debugging */
#define ALWAYS_FLUSH FALSE

/* ISO stuff */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

/* PCRE stuff */
#include "pcre.h"

/* For this app */
#include "regexps.h"
#include "haddoc.h"
#include "cl.h"

/* Used to give a unique id to each object in the library */
unsigned int object_id = 0;

const char *qualifier_names[NO_QUALIFIERS] =
{
	"const",
	"volatile",
	"restrict",
};

const char *ref_types[NO_OBJECTTYPES] =
{
	"file",
	"function",
	"type", /* fn type - just a type in these terms */
	"global",
	"comment",
	"empty",
	"type",
	"macrofn",
	"macro",
};

void output_function(const char *func, size_t len);

struct xml_escape
{
	const char ch;
	const char *out;
};

#define NO_ESCAPES 3

struct xml_escape xml_escapes[NO_ESCAPES] =
{
	{'<', "&lt;" },
	{'>', "&gt;" },
	{'&', "&amp;" },
/*	{'#', "&hash;" },
	{'\\', "&bslash;" },
	{'{', "&cbl;" },
	{'!', "&pling;" }*/
};

/* ------------------------------------------------ ajps, 2002-11-21 ---
 * Prints like fprintf, but converts XML's "special" character to the
 * proper codes as it prints.
 * --------------------------------------------------------------------- */
void xml_fputc(char ch, FILE *stream)
{
	int n;

	for (n = 0; n < NO_ESCAPES; n++)
	{
		if (ch == xml_escapes[n].ch)
		{
			fputs(xml_escapes[n].out, stream);

#if ALWAYS_FLUSH
			fflush(stream);
#endif
			return;
		}
	}

	fputc(ch, stream);

#if ALWAYS_FLUSH
			fflush(stream);
#endif
}

void xml_fputs(const char *string, FILE *stream)
{
	const char *read = string;
	while (*read != '\0') xml_fputc(*(read++), stream);
}

/* ------------------------------------------------ ajps, 2002-11-21 ---
 * Prints like fprintf, but converts XML's "special" character to the
 * proper codes as it prints.
 * --------------------------------------------------------------------- */
void xml_fprintf(FILE *stream, const char *format, ...)
{
	/* More than 4k at a time will break this */
	char temp[4096];
	char *read;
	BOOL done;
	int n;

	va_list	argp;

	/* Initialise variable argument handling */
	va_start(argp,format);

	/* Print into the temp string */
	vsnprintf(temp, sizeof temp, format, argp);

	xml_fputs(temp, stream);

	/* We are done with the variable argument stuff */
	va_end(argp);

	return;
}

void output_qualifiers(int qualifiers, FILE *fptr)
{
	int n;
	BOOL first_qualifier = TRUE;

	for (n = 0; n < NO_QUALIFIERS; n++)
	{
	    if ((qualifiers & (1<<n)) != 0)
	    {
	        if (first_qualifier)
	        {
	            fputs(" qualifier='", fptr);
	            first_qualifier = FALSE;
	        }
	        else
	        {
	            fputc(' ', fptr);
	        }
	       	fputs(qualifier_names[n], fptr);
	    }
	}

	if (first_qualifier == FALSE) fputc('\'', fptr);
}


/* -------------------------------------------------- ajps, 24/02/03 ---
 * Builds an index of all names which should be used for cross-reference
 * purposes.
 * --------------------------------------------------------------------- */
struct index_item
{
	const char *name;
	int len;
	int file_no;
	int item_no;
} *ref_index = NULL;

int index_len = 0;

#define INDEX_GRAIN 50

void add_to_index(const char *name, size_t len, int file_no, int item_no)
{
	struct index_item *temp;

	if (index_len % INDEX_GRAIN == 0)
	{
		temp = realloc(ref_index, (index_len + INDEX_GRAIN) * sizeof(*temp));

		if (temp == NULL)
		{
			cl_error(cl_FATAL, "Can't reallocate memory for name index (wanted %i bytes)\n", index_len + INDEX_GRAIN);
		}

		ref_index = temp;
	}


	index_len++;

	if (len == 0) ref_index[index_len - 1].name = make_string(name);
	else ref_index[index_len - 1].name = make_string_n(name, len);

	if (ref_index[index_len - 1].name == NULL)
	{
		cl_error(cl_FATAL, "Not enough memory to add %s to the index\n", name);
	}

	ref_index[index_len - 1].len = strlen(ref_index[index_len - 1].name);
	ref_index[index_len - 1].file_no = file_no;
	ref_index[index_len - 1].item_no = item_no;
}

int compare_index_items(const void *str1, const void *str2)
{
	const struct index_item *item1 = str1, *item2 = str2;

	return strcmp(item1->name, item2->name);
}

void build_index(void)
{
	int n, m, o;

	char *enumsearch;
	int deflen;
	int start;

	for (n = 0; n < no_files; n++)
	{
		for (m = 0; m < files[n].no_parts; m++)
		{
			/* The actual names get added without a second thought */
			for (o = 0; o < files[n].contents[m].no_names; o++)
			{
				add_to_index(files[n].contents[m].names[o], 0, n, m);
			}

			enumsearch = files[n].contents[m].definition;

			if (enumsearch == NULL) continue;

			deflen = strlen(enumsearch);
			start = 0;

			while (pcre_exec(regexes_too[re_index_FINDENUM], NULL,
							 enumsearch, deflen, start, 0, ovector,
							 OVECTOR_SIZE) > 0)
			{
				char *text = enumsearch + ovector[2];
				int len = ovector[3] - ovector[2];

				start = ovector[1];

				/* Skip the { */
				text++;
				len--;

				/* Do the whole enum */
				while (pcre_exec(regexes_too[re_output_ENUMVAL], NULL, text,
								 len, 0, 0, ovector,  OVECTOR_SIZE) > 0)
				{
					add_to_index(text + ovector[2], ovector[3] - ovector[2], n, m);

					text += ovector[1];
					len -= ovector[1];
				}
			}
		}
	}

	qsort(ref_index, index_len, sizeof(struct index_item), compare_index_items);

/*	fprintf(stderr, "---\nIndex:\n");
	for (n = 0; n < index_len; n++)
	{
		fprintf(stderr, "%3i. %s\n", n, ref_index[n].name);
	}
	fprintf(stderr, "---\n");*/
}


/* ------------------------------------------------ ajps, 2002-11-27 ---
 * Check if a link starts at the point given by read.  Output the
 * marked-up link and return the amount of 'read' consumed, or return
 * zero if a link is not appropriate.
 * --------------------------------------------------------------------- */
int markup_link(const char *read, FILE *stream)
{
	int n, result;
	int readlen = strlen(read);

	for (n = 0; n < index_len; n++)
	{
        if (readlen < ref_index[n].len) continue;

		/* Check the end point for word-boundary-ness */
		if (isalnum((int) read[ref_index[n].len]) || read[ref_index[n].len] == '_')
		{
			/* We don't waste time if this isn't a word boundary */
			continue;
		}

  		if (ref_index[n].name[0] < read[0]) continue;
		if (ref_index[n].name[0] > read[0]) return 0;

		if (memcmp(ref_index[n].name, read, ref_index[n].len) == 0)
		{
			fprintf(stream, "<link ref='%05i'", files[ref_index[n].file_no].contents[ref_index[n].item_no].id);
			fprintf(stream, " file='%s'", files[ref_index[n].file_no].names[0]);
			fprintf(stream, ">%s</link>", ref_index[n].name);

			return ref_index[n].len;
		}
	}

	return 0;
}

void output_var(const char *var, const char *element, size_t len);

/* ------------------------------------------------ ajps, 2003-02-22 ---
 * Takes the qualifiers, name, text, etc etc and outputs the enum stuff.
 * --------------------------------------------------------------------- */
void output_enum(int qualifiers, const char *name, const char *text, size_t len)
{
	fputs("<enum", out_stream);
	output_qualifiers(qualifiers, out_stream);

	/* We need the name, too */
	if (pcre_exec(regexes_too[re_output_IDENTIFIER], NULL,
				  name, strlen(name), 0, 0,
				  ovector,  OVECTOR_SIZE) > 0)
	{
		fprintf(out_stream, " name='%.*s'", ovector[3] - ovector[2], name + ovector[2]);
	}

	fputs(">\n", out_stream);

	/* Skip the { */
	text++;
	len--;

	/* Do the whole enum */
	while (pcre_exec(regexes_too[re_output_ENUMVAL], NULL, text, len, 0, 0,
				  ovector,  OVECTOR_SIZE) > 0)
	{
		fputs("<enum-val name='", out_stream);
		fprintf(out_stream, "%.*s'", ovector[3] - ovector[2], text + ovector[2]);

		/* Value */
		if (ovector[4] != ovector[5])
		{
			fputc('>', out_stream);
			xml_fprintf(out_stream, "%.*s", ovector[5] - ovector[4], text + ovector[4]);
			fputs("</enum-val>\n", out_stream);
		}
		else
		{
			fputs("/>\n", out_stream);
		}

		text += ovector[1];
		len -= ovector[1];
	}

	fputs("</enum>\n", out_stream);
}

/* -------------------------------------------------- ajps, 22/02/03 ---
 * Takes the type (struct or union), the qualifiers, the actual definition
 * text and the length of that text.  It then outputs it all.
 * --------------------------------------------------------------------- */
void output_structish(const char *type, const char *name, int qualifiers, const char *text, size_t len)
{
	size_t n, brackets = 1;

	/* Opening of element */
	fputc('<', out_stream);
	fputs(type, out_stream);
	output_qualifiers(qualifiers, out_stream);

	if (pcre_exec(regexes_too[re_output_IDENTIFIER], NULL,
				  name, strlen(name), 0, 0,
				  ovector,  OVECTOR_SIZE) > 0)
	{
		fprintf(out_stream, " name='%.*s'", ovector[3] - ovector[2], name + ovector[2]);
	}

	fputs(">\n", out_stream);

	/* Skip the { */
	text++;
	len--;
/*
	fprintf(stderr, "PArsing:%s\n", text);
	fprintf(stderr, "lenght=%i\n", len);
*/
	/* Do the whole struct/union */
	while (*text != '}')
	{
		brackets = 0;
		n = 0;

		while (n <= len && (brackets > 0 || text[n] != ';'))
		{
			if (text[n] == '{') brackets++;
			if (text[n] == '}') brackets--;
			n++;
		}

		if (n > len)
		{
			cl_error(cl_FATAL, "Malformed nested struct or union.\n");
		}

		if (text[n] == ';')
		{
			n++;
			fputs("<member>\n", out_stream);
			output_var(text, "variable", n);
			fputs("</member>\n", out_stream);

/*			fprintf(stderr, "member: %.*s\n", (int) n, text);*/
		}

		text += n;
		len -= n;

		while (isspace(*text)) text++;
	}

	/* Close of element */
	fputs("</", out_stream);
	fputs(type, out_stream);
	fputs(">\n", out_stream);
}

/* ------------------------------------------------ ajps, 2003-02-20 ---
 * Given a pointer to the start of a "type" section, and the length of
 * that section, this outputs the appropriate XML stuff, including dealing
 * with structs, etc through some sort of recursive thingummy.
 * --------------------------------------------------------------------- */
void output_type(const char *type, size_t len)
{
	const char *read = type;
	int no_results;
	int qualifiers = 0;

	if (pcre_exec(regexes_too[re_output_TYPESIMPLE], NULL, type, len, 0, 0,
				  ovector,  OVECTOR_SIZE) <= 0)
	{
		cl_error(cl_FATAL, "Failed to parse a type correctly.\n");
	}

	if (ovector[6] != -1 && ovector[7] != -1)
	{
	    read = type + ovector[5];

		qualifiers = check_qualifier(&read, qualifiers);
	}

	if (ovector[2] != -1 && ovector[3] != -1)
	{
	    read = type + ovector[2];

		qualifiers = check_qualifier(&read, qualifiers);
	}

	/* Check if this is a normal type name */
	if ((strchr(type, '{') == NULL) || (strchr(type, '{') >= type + len))
	{
		fprintf(out_stream, "<type");

	    output_qualifiers(qualifiers, out_stream);

		fprintf(out_stream, ">");

		/* Default to int if there is no type */
	    if (ovector[4] == ovector[5])
	    {
	    	if (markup_link("int", out_stream) == 0)
	    	{
	    		fputs("int", out_stream);
	    	}
	    }
	    else
	    {
	    	int linklen = markup_link(type + ovector[4], out_stream);

	    	fprintf(out_stream, "%.*s", (ovector[5] - ovector[4]) - linklen,
	    	        type + ovector[4]) + linklen;
		}

		fputs("</type>\n", out_stream);
	}
	/* We need to deal with enums, and structs and unions here */
	else
	{
		const char *text;

		/* Parse out the name here */
		text = strchr(type + ovector[4], '{');

		if (text == NULL)
		{
			cl_error(cl_FATAL, "Type wrongly identified as a struct or union\n");
		}

		if (strncmp(read, "struct ", 7) == 0)
		{
			output_structish("struct", read + 7, qualifiers, text, (ovector[5] - ovector[4]) - (text - (type + ovector[4])));
		}
		else if (strncmp(read, "union ", 6) == 0)
		{
			output_structish("union", read + 6, qualifiers, text, (ovector[5] - ovector[4]) - (text - (type + ovector[4])));
		}
		else if (strncmp(read, "enum ", 5) == 0)
		{
			output_enum(qualifiers, read + 5,  text, (ovector[5] - ovector[4]) - (text - (type + ovector[4])));
		}
		else
		{
			cl_error(cl_FATAL, "'{' not after one of struct, union or enum.");
		}
	}
}

enum namelist_itemtype
{
	namelist_EMPTY = 0,
	namelist_POINTER,
	namelist_ARRAY
};

struct namelist_item
{
	struct namelist_item *next;
	struct namelist_item *prev;
	enum namelist_itemtype type;
	union
	{
		char *text;
		int qualifiers;
	} contents;
};

struct namelist
{
	struct namelist_item *start;
   	struct namelist_item *end;
};

/* -------------------------------------------------- ajps, 23/02/03 ---
 * Inserts a list item after the given one.  If 'item' is NULL, then it
 * is added to the top of the list.
 * --------------------------------------------------------------------- */
struct namelist_item *namelist_insert(struct namelist *list,
									  struct namelist_item *item)
{
	struct namelist_item *new;

	new = malloc(sizeof(*new));

	if (new == NULL)
	{
		cl_error(cl_FATAL, "Not enough memory to parse the \"name\"");
	}

	new->type = namelist_EMPTY;

	if (item == NULL)
	{
		new->prev = NULL;
		new->next = list->start;
		list->start = new;

		if (new->next != NULL)
		{
			new->next->prev = new;
		}
	}
	else
	{
		/* Place ourselves after the "item" */
		new->prev = item;
		new->next = item->next;
		item->next = new;

		/* And before the next item, if it exists */
		if (new->next != NULL)
		{
			new->next->prev = new;
		}
	}

	if (new->next == NULL)
	{
		list->end = new;
	}

	return new;
}

/* -------------------------------------------------- ajps, 23/02/03 ---
 * Invoked recursively to build up a linked list of pointer and array
 * elements.
 * --------------------------------------------------------------------- */
void names_parse(const char **text, size_t len, struct namelist *list,
				 struct namelist_item *current, char **bitfield, char **name)
{
	const char *start = *text;

	while (**text != ';' && **text != ',' && *text < (start + len))
	{
		if (**text == '*')
		{
			current = namelist_insert(list, current);

			/* Woo, pointer */
			(*text)++;

			current->type = namelist_POINTER;
			current->contents.qualifiers = check_qualifier(text, 0);
		}
		else if (**text == '(')
		{
			(*text)++;

			names_parse(text, len - (*text - start),
						list, current, bitfield, name);
		}
		else if (**text == ')')
		{
			(*text)++;

			return;
		}
		else if (**text == ':')
		{
			const char *bitfieldstart;

			(*text)++;

			while (isspace(**text)) (*text)++;

			bitfieldstart = *text;

			if (*name == NULL)
			{
				cl_error(cl_FATAL, "You can't have a bitfield before a variable name,\n");
			}

			while (!isspace(**text) && **text != ')' && **text != ';' && **text != ',')
			{
				(*text)++;
			}

			if (*bitfield != NULL)
			{
				cl_error(cl_FATAL, "You can only have a single bitfield specified for variable.\n");
			}

			*bitfield = make_string_n(bitfieldstart, *text - bitfieldstart);
		}
		else if (**text == '[')
		{
			const char *arraystart = *text;

			while (**text != ']' && **text != ';' && **text != ',')
			{
				(*text)++;
			}

			if (**text == ']') (*text)++;

			current = namelist_insert(list, current);

			current->type = namelist_ARRAY;
			current->contents.text = make_string_n(arraystart, *text - arraystart);
		}
		else if (**text == '_' || isalpha(**text))
		{
			const char *namestart = *text;

			while (**text == '_' || isalnum(**text))
			{
				(*text)++;
			}

			if (*name != NULL)
			{
				cl_error(cl_FATAL, "Something appears to have two names\n");
			}

			*name = make_string_n(namestart, *text - namestart);
		}
		else
		{
			(*text)++;
		}
	}

	/* Skip ; or , */
	if (*text < (start + len)) (*text)++;
}

void output_names(const char *names, size_t len, const char *element)
{
	char *bitfield = NULL;
	char *name = NULL;
	const char *text = names;

	struct namelist list;
	struct namelist_item *item;

	/* If there are no names to parse, do nothing */
	while (pcre_exec(regexes_too[re_output_NONAMES], NULL, text, len, 0, 0,
				  ovector,  OVECTOR_SIZE) <= 0)
	{
		list.start = NULL;
		list.end = NULL;
		names_parse(&text, len, &list, NULL, &bitfield, &name);

		if (name == NULL)
		{
			fprintf(out_stream, "<item");
		}
		else
		{
			fprintf(out_stream, "<item name='%s'", name);
		}

		if (bitfield != NULL) xml_fprintf(out_stream, " bitfield='%s'", bitfield);
		fputc('>', out_stream);
#if 0
		/* Debugging aid */
		item = list.start;
		fprintf(stderr, "List contains:\n");
		while (item != NULL)
		{
			fprintf(stderr, "%i", item->type);
			if (item->type == namelist_POINTER)
			{
				fprintf(stderr, "\n");
			}
			else if (item->type == namelist_ARRAY)
			{
				fprintf(stderr, "%s\n", item->contents.text);
			}
			item = item->prev;
		}
		fprintf(stderr, "List ends:\n");
#endif
		/* Scoot to the end of the list */
		item = list.end;

		/* Go backwards through the list */
		while (item != NULL)
		{
			switch (item->type)
			{
				case namelist_POINTER:
				{
					fputs("<ptr", out_stream);
	                output_qualifiers(item->contents.qualifiers, out_stream);
					fputc('>', out_stream);

					break;
				}

				case namelist_ARRAY:
				{
					if (item->contents.text == NULL)
					{
						cl_error(cl_FATAL, "Array without size specifier\n");
					}

					fputs("<array size='", out_stream);
					xml_fprintf(out_stream, "%s'", item->contents.text);
					fputc('>', out_stream);

					break;
				}

				default:
				{
					cl_error(cl_FATAL, "Error: empty section in the name parsing list.\n");
				}
			}

			item = item->prev;
		}

		item = list.start;
		while (item != NULL)
		{
			switch (item->type)
			{
				case namelist_POINTER:
				{
					fputs("</ptr>", out_stream);
					break;
				}

				case namelist_ARRAY:
				{
					fputs("</array>", out_stream);
					break;
				}

				default:
				{
					cl_error(cl_FATAL, "Error: empty section in the name parsing list.\n");
				}
			}

			item = item->next;
		}

		fputs("</item>\n", out_stream);

	    /* Cleaning up, freeing temporarily allocated strings, etc. */
		item = list.start;
		while (item != NULL)
		{
			struct namelist_item *temp;

			temp = item->next;

			if (item->type == namelist_ARRAY && item->contents.text != NULL)
			{
				free(item->contents.text);
			}

			free(item);

			item = temp;
		}

		free(bitfield);
		bitfield = NULL;

		free(name);
		name = NULL;

	    len -= text - names;
	}
}

/* -------------------------------------------------- ajps, 18/02/03 ---
 * Given a single statement consisting of a type and one or more names
 * with associated qualifiers, etc, it writes out the correct definition,
 * enclosing the "name" part in "element" tags.
 *
 * ie. you pass in "char *fred, const *bill;" and "typedef", you get out
 *     <type>char</type>
 *     <typedef name="fred"><ptr /></typedef>
 *     <typedef name="bill"><ptr qualifier="const" /></typedef>
 *
 * or something similar, as appropriate.
 *
 * The length is the maximum length to scan in the string, this is used
 * to enable you to process nested structs and unions, for instance.
 * --------------------------------------------------------------------- */
void output_var(const char *var, const char *element, size_t len)
{
	const char *type = strchr(var, '{');
	size_t type_len;
	const char *param_start = find_params(var, len);

	/* First check if it is a function pointer */
	if (param_start != NULL)
	{
		/* Extract name information and similar here */
		if (pcre_exec(regexes_too[re_names_FN], NULL,
							   var,
							   param_start - var + 1,
							   0, 0, ovector, OVECTOR_SIZE) > 0)
		{
			if (ovector[3] != ovector[2])
			{
				fprintf(out_stream, "<function>\n");
				output_function(var, len);
				fprintf(out_stream, "</function>\n");

				/* The rest of the function is of no interest */
				return;
			}
		}
	}

	/* For non-funcion pointers, first isolate the "type" part */
	if (pcre_exec(regexes_too[re_output_TYPE], NULL, var, len, 0, 0,
					ovector,  OVECTOR_SIZE) <= 0)
	{
		cl_error(cl_FATAL, "Failed to parse a type correctly.\n");
		return;
	}

	/*
	 * We need to store these because the result vector is a global variable,
	 * and the other functions invoked from here may corrupt it
	 */
	type = var + ovector[2];
	type_len = ovector[2 + 1] - ovector[2];

	/*
	 * We pass a pointer to the start of the type block and the length
	 * of the block to the function which outputs the type.
	 */
	output_type(type, type_len);

	/* Pass a pointer to the names part, and accompanying length. */
	output_names(type + type_len, len - type_len, element);
}

/* -------------------------------------------------- ajps, 23/02/03 ---
 * Outputs a function definition.
 * --------------------------------------------------------------------- */
void output_function(const char *func, size_t len)
{
	const char *params = find_params(func, len);
	char *read;
	int type_len = 0;

	if (params == NULL)
	{
		cl_error(cl_FATAL, "Failed to find function parameters\n");
	}

	/* Skip the opening parenthesis */
	params++;

	/* Get the location of return type and name */
	if (pcre_exec(regexes_too[re_output_FUNC], NULL, func,
					 params - func, 0, 0, ovector,  OVECTOR_SIZE) <= 0)
	{
		cl_error(cl_FATAL, "Failed to find return type and name of function\n");
	}

	/* Store in case of ovector being used again */
	type_len = ovector[3];

	/* Output the return type, including pointery bits where applicable */
	fprintf(out_stream, "\n<returns>\n");

	read = strchr(func, '*');

	if (read == NULL || read >= func + type_len)
	{
		/* The simple case where the return type is not a pointer */
		output_type(func, type_len);
	}
	else
	{
		/* Output the type, then the pointery bits */
		output_type(func, read - func);
		output_names(read, func + type_len - read, "unused");
	}

	fprintf(out_stream, "</returns>\n\n");


	/* Now the function name, and its pointer status */
	output_names(func + type_len, params - (func + type_len) - 1, "unused");

	/* Skip whitespace */
	while (isspace(*params)) params++;

	/* Oo, parameters! */
	if (*params == ')')
	{
		fprintf(out_stream, "\n<parameter>\n<type>void</type></parameter>");
	}
	else
	{
		if (strncmp(params, "void", 4) == 0)
		{
			const char *p = params + 4;
			while (isspace(*p)) p++;
			if (*p == ')')
			{
				/* fn(void), so just output and exit the function */
				fprintf(out_stream,  "\n<parameter>\n<type>void</type></parameter>");
				return;
			}
		}

		/* Output the function parameters */
		while (pcre_exec(regexes_too[re_output_FUNCPARAMS], NULL, params,
		                 strlen(params), 0, 0, ovector, OVECTOR_SIZE) > 0)
		{
			int size = ovector[1];

			fprintf(out_stream, "\n<parameter>\n");

			output_var(params + ovector[2], "empty", ovector[3] - ovector[2] + 1);

			params += size;

			fprintf(out_stream, "</parameter>\n");
		}
	}
}

void output_definition(object_info *object)
{
	const char *read = object->definition;

	switch (object->type)
	{
		case obj_FN:
		{
			while (isspace(*read)) read++;
			if (strncmp(read, "extern ", 7) == 0) read += 7;

			fprintf(out_stream, "<function>\n");
			output_function(read, strlen(read));
			fprintf(out_stream, "</function>\n");

			break;
		}

		case obj_FNTYPE:
		{
			while (isspace(*read)) read++;
			if (strncmp(read, "typedef ", 8) == 0) read += 8;

			fprintf(out_stream, "<typedef>\n<function>\n");
			output_function(read, strlen(read));
			fprintf(out_stream, "</function>\n</typedef>\n");
			break;
		}

		case obj_TYPE:
		{
			while (isspace(*read)) read++;

			if (strncmp(read, "typedef", 7) == 0)
			{
				read += 7;
			}
			else
			{
				if (pcre_exec(regexes_too[re_output_INCOMPLETESTRUCT],
							  NULL, read, strlen(read), 0, 0,
							  ovector,  OVECTOR_SIZE) > 0)
				{
					fprintf(out_stream, "<struct name='%.*s' />\n",
							ovector[3] - ovector[2], read + ovector[2]);

					break;
				}
			}

			fprintf(out_stream, "<typedef>\n");
	        output_var(read, "unused", strlen(read));
			fprintf(out_stream, "</typedef>\n");

			break;
		}

		case obj_VARIABLE:
		{
			while (isspace(*read)) read++;

			if (strncmp(read, "extern", 7) == 0) read += 7;

			fprintf(out_stream, "<variable>\n");
	        output_var(read, "unused", strlen(read));
			fprintf(out_stream, "</variable>\n");

			break;
		}

		default:
		{
			fprintf(out_stream, "Not implemented\n");
		}
	}
}

/* ------------------------------------------------ ajps, 2003-02-16 ---
 * Writes the macro definition.  Not the most complicated of functions.
 * --------------------------------------------------------------------- */
void output_macro(char *text)
{
	fputs("<macro>", out_stream);
	xml_fputs(text, out_stream);
	fputs("</macro>\n", out_stream);
}

/* -------------------------------------------------- ajps, 10/02/03 ---
 * Output the given description, with paragraphing and references marked
 * up, characters converted to XML entities where appropriate, etc.
 * --------------------------------------------------------------------- */
BOOL output_description(char *comment, const char *title)
{
	BOOL in_paragraph = FALSE;
	char *read = comment;

	/* We deal with non-existent comments like this */
	if (comment == NULL) return TRUE;

	fprintf(out_stream, "<description");
	if (title != NULL) fprintf(out_stream, " title='%s'", title);
	fprintf(out_stream, ">\n");

	while (*read != '\0')
	{
		BOOL line_empty = TRUE;
		char *line_start;

		line_start = read;

		if (*read == '\n') read++;

		/* Check for this being an empty line */
		while (line_empty && *read != '\n' && *read != '\0')
		{
			if (!isspace(*read)) line_empty = FALSE;

			read++;
		}

		if (line_empty)
		{
			if (in_paragraph) fprintf(out_stream, "</p>\n");
			in_paragraph = FALSE;

			continue;
		}
		else
		{
			read = line_start;

			if (*read == '\n')
			{
				read++;
				if (in_paragraph) fprintf(out_stream, " ");
			}

#if 0
			/* Future list code? */
			if (*read == ' ')
			{
			}
			else
#endif
			{
				if (!in_paragraph)
				{
					fprintf(out_stream, "<p>");
#if ALWAYS_FLUSH
					fflush(out_stream);
#endif
					in_paragraph = TRUE;
				}

				/* Write the line */
				while (*read != '\n' && *read != '\0')
				{
					int link_len = 0;

					if (!isalpha(*read) && *read != '_')
					{
						xml_fputc(*(read++), out_stream);
					}
					else
					{
						/* Output a link here if appropriate */
						link_len = markup_link(read, out_stream);

						if (link_len == 0)
						{
							/* Output up to the end of this piece of text */
							while (isalnum(*read) || *read == '_')
							{
								xml_fputc(*(read++), out_stream);
							}
						}
						else
						{
							/* Skip the text that has been marked up */
							read += link_len;
						}
					}
				}
			}
		}
	}

	if (in_paragraph) fprintf(out_stream, "</p>\n");

	fprintf(out_stream, "</description>\n");

	return TRUE;
}

/* -------------------------------------------------- ajps, 10/02/03 ---
 * Writes information about the given file in our XML format.
 * --------------------------------------------------------------------- */
BOOL output_file(object_info *file)
{
	int n;

	fprintf(out_stream, "<file name='%s'>\n", file->names[0]);
	fprintf(out_stream, "\n");

	if (file->description != NULL)
	{
		output_description(file->description, "overview");
		fprintf(out_stream, "\n");
	}

	for (n = 0; n < file->no_parts; n++)
	{
		int m = 0;

		if (file->contents[n].type == obj_EMPTY ||
			file->contents[n].type == obj_COMMENT) continue;

		fprintf(out_stream, "<object type='%s' id='%05i'>\n",
				ref_types[file->contents[n].type], file->contents[n].id);

		/*
		 * If the items have names, write them all out in a 'names' attribute,
		 * separated by semicolons.
		 */
		if (file->contents[n].no_names > 0)
		{
			for (m = 0; m < file->contents[n].no_names; m++)
			{
				fprintf(out_stream, "<name>%s</name>\n", file->contents[n].names[m]);
			}

		}

		/* Output definition here */
		fprintf(out_stream, "<definition>\n");

		if (file->contents[n].type == obj_MACRO ||
			file->contents[n].type == obj_MACROFN)
		{
			output_macro(file->contents[n].definition);
		}
		else
		{
			output_definition(&file->contents[n]);
		}

		fprintf(out_stream, "</definition>\n");

		/* And any description */
		if (file->contents[n].description != NULL)
		{
			output_description(file->contents[n].description, NULL);
		}

		fprintf(out_stream, "</object>\n");

		fprintf(out_stream, "\n");
	}

	fprintf(out_stream, "</file>\n\n");

	return TRUE;
}

