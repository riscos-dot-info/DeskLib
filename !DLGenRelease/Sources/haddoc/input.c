/* ------------------------------------------------ ajps, 2002/11/19 ---
 * File: input.c
 * Abstract: Reads in the header files, parsing them into "objects", with
 * definitions and descriptions stored as part of the objects.
 * Copyright (c) 2003 Antony Sidwell
 * --------------------------------------------------------------------- */

/* ISO stuff */
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Unixlib stuff */
#include "dirent.h"

/* PCRE stuff */
#include "pcre.h"

/* For this app */
#include "regexps.h"
#include "haddoc.h"
#include "cl.h"

#define process_FAILED -1
#define process_DONE 0
#define process_NOTDONE 1

/* Used to give unnamed enums a reference code, unique within a file */
int enum_counter = 0;

#define READING_ON 0
#define READING_OFF -1

/*
 * The amount of memory to allocate at a time for reading
 * in definitions, comments, etc.
 */
#define TEXT_GRAIN 4

/* Used to turn on or off documentation */
int reading = READING_ON;

char *(read_line)(FILE *fptr);

/* ------------------------------------------------ ajps, 2002-12-06 ---
 * Reads up to LINEBUFF_SIZE of a line into linebuff from the given file.
 * Returns NULL if something goes wrong, or we are at the end of the
 * file.
 *
 * It handles the turning on and off of reading data in, by dicarding
 * everything after and including the "off comment" up until the next
 * "on comment".  For lines we aren't supposed to deal with, an empty
 * line of some sort will be placed in linebuff.
 *
 * I'm also introducing an "ignore line" comment, which will assume that
 * each call to this function actually does read in a whole line.
 * --------------------------------------------------------------------- */
char *read_line(FILE *fptr)
{
	int no_results;
	char *cptr;
	int n;
	int line_len;

	if (fgets(linebuff, LINEBUFF_SIZE, fptr) == NULL) return NULL;

	line_len = strlen(linebuff);

	if (reading == READING_ON)
	{
		no_results = pcre_exec(regexes_too[re_DOCOFF], NULL, linebuff, line_len,
							   0, 0, ovector, OVECTOR_SIZE);
		if (no_results > 0)
		{
			strcpy(linebuff, "");
			reading = READING_OFF;
		}
		else
		{
			no_results = pcre_exec(regexes_too[re_DOCSKIP], NULL, linebuff,
								   line_len, 0, 0, ovector, OVECTOR_SIZE);
			if (no_results > 0)
			{
				strcpy(linebuff, "");
				reading = 1;
			}
			else
			{
				no_results = pcre_exec(regexes_too[re_IGNORED], NULL, linebuff,
									   line_len, 0, 0, ovector,  OVECTOR_SIZE);
			    if (no_results > 0)
			    {
					/* Skip multiline conditionals, too. */
			    	while (linebuff[line_len - 3] != '\\' &&
			    		   linebuff[line_len - 2] == '\\' &&
			    		   linebuff[line_len] != '\n')
			    	{
			    		if (fgets(linebuff, LINEBUFF_SIZE, fptr) == NULL)
			    		{
							return NULL;
			    		}
			    		line_len = strlen(linebuff);
			    	}
					strcpy(linebuff, "");
			    }
			    /* User-entered ignore expressions */
			    else
			    {
					for (n = 0; n < no_excl_patterns && no_results <= 0; n++)
			    	{
				    	no_results = pcre_exec(excl_patterns[n], NULL,
				    						   linebuff, line_len,
				    						   0, 0, ovector,
				    						   OVECTOR_SIZE);
				    }

					if (no_results > 0)
					{
						strcpy(linebuff, "");
					}
			    }
			}
		}
	}
	/* Stopped reading altogether */
	else if (reading == READING_OFF)
	{
		no_results = pcre_exec(regexes_too[re_DOCON], NULL, linebuff, strlen(linebuff), 0, 0, ovector, OVECTOR_SIZE);
 		if (no_results > 0)
		{
			strcpy(linebuff, "");
			reading = READING_ON;
		}
		else
		{
			strcpy(linebuff, "");
		}
	}
	/* Skipping a line or lines */
	else if (reading > 0)
	{
		reading--;
		strcpy(linebuff, "");
	}
	/* Something is very wrong */
	else
	{
		reading = READING_ON;
	}

	/* Return a pointer to the first non-whitespace character */
	cptr = linebuff;
	while (isspace(*cptr)) cptr++;

	return cptr;
}

/* ------------------------------------------------ ajps, 2003-02-07 ---
 * This processes the comment into a well-defined form - the left margin
 * is calculated and removed, so that text starts at the beginning of
 * each line (or not, if we support indented lists in the future).
 *
 * The opening and closing of comments are removed, as are any leading
 * or trailing spaces.
 *
 * This should make the processing at the output stage much simpler, and
 * could facilitate extra information being extracted from comments with
 * simple regexes (eg. file version number or author).
 * --------------------------------------------------------------------- */
void clip_comment_margin(char *comment)
{
	int no_results;
	char *read;
	char *write;
	char *start = NULL;
	int margin = 0;
	int n;
	BOOL in_text = FALSE;

	/* We deal with non-existent comments like this */
	if (comment == NULL) return;

	read = comment;

	/* Skip the start of comment */
	while (*read != '\0')
	{
		if (isspace(*read))
		{
			read++;
			margin++;
		}
		else if (read[0] == '/' && read[1] == '*')
		{
			read += 2;
			margin += 2;
			break;
		}
	}

	/*
	 * We carry on until the end of the comment, or until we break
	 * out of the loop having found the margin size.
	 */
	while (*read != '\0' && !(read[0] == '*' && read[1] == '/'))
	{
		if (*read == '\n')
		{
			margin = 0;
		}
		else if (*read != '*' && !isspace(*read))
		{
			/* Remember the location of the first piece of "content" */
			start = read;
			break;
		}
		else
		{
			margin++;
		}
		read++;
	}

	/* If we reached the end of the text, assume the comment is empty */
	if (*read == '\0' || (read[0] == '*' && read[1] == '/'))
	{
		comment[0] = '\0';
		return;
	}

	/* Find the end-of-comment and zero-terminate there */
	while (*read != '\0' && !(read[0] == '*' && read[1]=='/'))
	{
		read++;
	}
	*read = '\0';

	/* If there is *no* content */
	if (start == NULL) comment[0] = '\0';

	/*
	 * Note that all of the below relies on read being "ahead" of write
	 * in the string - we are fine as long as we are shortening, not
	 * lengthening
	 */
	read = start;
	write = comment;

	while (*read != '\0')
	{
		while (*read != '\n' && *read != '\0')
		{
			*(write++) = *(read++);
		}

		if (*read == '\n')
		{
			*(write++) = '\n';
			read++;
		}

		for (n = 0; n < margin && *read != '\n' && *read != '\0'; n++)
		{
			read++;
		}
	}

	/* Terminate immediately after the last character */
	while (write > comment && isspace(*(write - 1)))
	{
		write--;
	}

	*write = '\0';
}

/* ------------------------------------------------ ajps, 2002-11-19 ---
 * Reads in a comment block.  If a description already exists, then
 * it is freed, and this one used instead.
 *
 * Only returns process_FAILED if we run out of file having opened up a
 * comment block and not closed it again.
 * ie. the absence of a comment block will not cause it to return
 * process_FAILED.
 * --------------------------------------------------------------------- */
int read_comment(char **description, FILE *fptr)
{
	int no_results;
	BOOL first_line = TRUE;

	/* look for a comment on this line */
	no_results = pcre_exec(regexes_too[re_SIGCOMMOPEN], NULL, linebuff,
						   strlen(linebuff), 0, 0, ovector, OVECTOR_SIZE);

	if (no_results <= 0)
	{
		/* Read another line if there wasn't one there */
		if (read_line(fptr) == NULL)
		{
			return process_DONE;
		}

		first_line = FALSE;

		no_results = pcre_exec(regexes_too[re_COMMOPEN], NULL, linebuff,
							   strlen(linebuff), 0, 0, ovector, OVECTOR_SIZE);
	}

	if (no_results > 0)
	{
	    /* Check if there is already a description. */
		if (*description) free_text(description);

		do
		{
			if (first_line == TRUE)
			{
				/* Should process *all* comment lines, including start and end */
				if (add_text(description, linebuff + ovector[0]) == FALSE)
				{
					cl_error(cl_FATAL,
							 "Couldn't claim memory for object description.\n");
				}
				first_line = FALSE;
			}
			else
			{
				/* Should process *all* comment lines, including start and end */
				if (add_text(description, linebuff) == FALSE)
				{
					cl_error(cl_FATAL,
							 "Couldn't claim memory for object description.\n");
					exit(EXIT_FAILURE);
				}
			}

			no_results = pcre_exec(regexes_too[re_COMMCLOSE], NULL, linebuff,
								   strlen(linebuff), 0, 0, ovector,
								   OVECTOR_SIZE);
			if (no_results > 0)
			{
				clip_comment_margin(*description);
				return process_DONE;
			}

		} while (read_line(fptr) != NULL);
	}
	else
	{
		return process_NOTDONE;
	}

	return process_FAILED;
}


int process_abstract(object_info *file, FILE *fptr)
{
	if (read_line(fptr) != NULL)
	{
		return read_comment(&(file->description), fptr);
	}
	else
	{
		return process_FAILED;
	}
}

int read_statement(char **definition, FILE *fptr)
{
	char *ptr;
	int layer = 0;
	BOOL struct_done = FALSE;

	do
	{
		if (add_text(definition, linebuff) == FALSE)
		{
			cl_error(cl_FATAL, "Couldn't claim memory for object definition.\n");
		}

		/* Check for nesting */
		ptr = linebuff;
		while (*ptr != '\0')
		{
			if (*ptr == '{')
			{
				layer++;
			}

			if (*ptr == '}') layer--;
			ptr++;
		}

		if (layer == 0)
		{
		  if (strchr(linebuff, ';') != NULL) struct_done = TRUE;
		}

		if (!struct_done && read_line(fptr) == NULL) return process_FAILED;

	} while (!struct_done);

	if (definition != NULL) condense_whitespace(*definition);

	return process_DONE;
}

int read_structlike(object_info *object, FILE *fptr)
{
	int no_results;

	if (read_statement(&object->definition, fptr) == process_FAILED)
	{
		return process_FAILED;
	}

	/* Extract name information and similar here */
	no_results = pcre_exec(regexes_too[re_names_STRUCTLIKE], NULL,
						   object->definition, strlen(object->definition),
						   0, 0, ovector, OVECTOR_SIZE);

	if (no_results > 0)
	{
	    add_name_n(object, object->definition + ovector[0],
	    		   ovector[1] - ovector[0]);
	}

	return process_DONE;
}

int read_typedef(object_info *object, FILE *fptr)
{
	const char *cptr;
	int no_results;
	int start;

	if (read_statement(&object->definition, fptr) == process_FAILED)
	{
		return process_FAILED;
	}

	cptr = object->definition;

	while (isspace(*cptr)) cptr++;

	if (strncmp(cptr, "typedef", 7) == 0) cptr += 7;
	else return process_FAILED;

	/* Skip any qualifier which may be present */
	check_qualifier(&cptr, 0);

/*	while (isspace(*cptr)) cptr++;*/

	if ((strchr(cptr, '{') != NULL) &&
		(strncmp(cptr, "struct", 6) == 0 ||
		 strncmp(cptr, "union", 5) == 0 ||
		 strncmp(cptr, "enum", 4) == 0))
	{
		/* Extract name information and similar here */
		no_results = pcre_exec(regexes_too[re_names_TYPEDSTRUCTLIKE], NULL,
							   object->definition, strlen(object->definition),
							   0, 0, ovector, OVECTOR_SIZE);

		object->type = obj_TYPE;

		if (ovector[2] != -1 && ovector[3] != -1 &&
			ovector[4] != -1 && ovector[5] != -1)
		{
	        add_name_n(object, object->definition + ovector[2],
					   ovector[3] - ovector[2]);
		}

		if (ovector[6] != -1 && ovector[7] != -1)
		{
			/*
			 * Note that this section really needs sub-parsing to extract
			 * possible multiple names.
			 */
			add_name_n(object, object->definition + ovector[6],
					   ovector[7] - ovector[6]);
		}
	}
	else
	{
		/* Check if function, else "normal" */
	    const char *param_start = find_params(object->definition, strlen(object->definition));

		if (param_start != NULL)
		{
			/* Extract name information and similar here */
			no_results = pcre_exec(regexes_too[re_names_FN], NULL,
								   object->definition, param_start - object->definition + 1,
								   0, 0, ovector, OVECTOR_SIZE);

			if (ovector[4] != -1 && ovector[5] != -1)
			{
				object->type = obj_FNTYPE;

				add_name_n(object, object->definition + ovector[4],
						   ovector[5] - ovector[4]);

				return process_DONE;
			}
		}

		/* We only reach here if it is not a function type */
		object->type = obj_TYPE;

		start = 0;

		while (pcre_exec(regexes_too[re_names_VARIABLE], NULL,
						 object->definition, strlen(object->definition),
						 start, 0, ovector, OVECTOR_SIZE) > 0)
		{
			if (ovector[2] != -1 && ovector[3] != -1)
			{
		        add_name_n(object, object->definition + ovector[2],
			    		   ovector[3] - ovector[2]);
			}

			start = ovector[1];
		}
	}

	return process_DONE;
}

/* ------------------------------------------------ ajps, 2003-02-16 ---
 * Trim whitespace, comments, etc. from the macro definition while
 * keeping the formatting as correct as possible.
 * --------------------------------------------------------------------- */
void compress_macro(char *text)
{
	int len;
	char *comment = text;

	/* First strip out any comments */
	comment = strstr(comment, "/*");
	while (comment != NULL)
	{
		char *write = comment;
		char *read = strstr(comment, "*/");

		if (read != NULL)
		{
			/* Skip the comment marker */
			read += 2;

			while (*read != '\0')
			{
				*(write++) = *(read++);
			}

			*(write) = '\0';
		}

		comment = strstr(comment, "/*");
	}

	/* Truncate to the position of the last printable character */
	len = strlen(text);
	while (isspace(text[len - 1]) && len > 0) len--;
	text[len] = '\0';
}

int read_macro(object_info *object, FILE *fptr)
{
	BOOL finished = FALSE;
	int len;
	int no_results;

	do
	{
		if (add_text(&object->definition, linebuff) == FALSE)
		{
			cl_error(cl_FATAL,
					 "Couldn't claim memory for object definition.\n");
		}

		len = strlen(linebuff);

		if (linebuff[len - 1] != '\n' || linebuff[len - 2] != '\\')
		{
			finished = TRUE;
		}

		if (!finished && read_line(fptr) == NULL) return process_FAILED;

	} while (!finished);

	compress_macro(object->definition);

	/* Extract name information and similar here */
	no_results = pcre_exec(regexes_too[re_names_MACRO], NULL,
						   object->definition, strlen(object->definition),
						   0, 0, ovector, OVECTOR_SIZE);

	if (ovector[2] != -1 && ovector[3] != -1)
	{
		object->type = obj_MACRO;

	    add_name_n(object, object->definition + ovector[2],
	    		   ovector[3] - ovector[2]);
	}
	else
	{
		no_results = pcre_exec(regexes_too[re_names_MACROFN], NULL,
							   object->definition, strlen(object->definition),
							   0, 0, ovector, OVECTOR_SIZE);

		if (ovector[2] != -1 && ovector[3] != -1)
		{
			object->type = obj_MACROFN;

	        add_name_n(object, object->definition + ovector[2],
	        		   ovector[3] - ovector[2]);
		}
	}

	return process_DONE;
}

int read_extern(object_info *object, FILE *fptr)
{
	int no_results;
	const char *param_start;
	int start = 0;

	if (read_statement(&object->definition, fptr) == process_FAILED)
		return process_FAILED;

	param_start = find_params(object->definition, strlen(object->definition));

	if (param_start != NULL)
	{
		/* Extract name information and similar here */
		no_results = pcre_exec(regexes_too[re_names_FN], NULL,
							   object->definition, param_start - object->definition + 1,
							   0, 0, ovector, OVECTOR_SIZE);

		if (ovector[4] != -1 && ovector[5] != -1)
		{
			if (ovector[3] == ovector[2])
			{
				object->type = obj_FN;
			}
			else
			{
				object->type = obj_VARIABLE;
			}

			add_name_n(object, object->definition + ovector[4],
					   ovector[5] - ovector[4]);

			return process_DONE;
		}
	}


	while (pcre_exec(regexes_too[re_names_VARIABLE], NULL,
					 object->definition, strlen(object->definition),
					 start, 0, ovector, OVECTOR_SIZE) > 0)
	{
		if (ovector[2] != -1 && ovector[3] != -1)
		{
			object->type = obj_VARIABLE;

			add_name_n(object, object->definition + ovector[2],
   		    		   ovector[3] - ovector[2]);
		}

		start = ovector[1];
	}

	return process_DONE;
}

void readin_file(object_info *file, const char *path)
{
	FILE *fptr;
	char *firstchar;
	int process_result = process_DONE;
	int curr_obj;

	reading = READING_ON;

	linebuff[LINEBUFF_SIZE - 1] = 0;

#ifdef RISCOS
        if (options.native) path = riscosify(path);
#endif

	fptr = fopen(path, "r");
	if (fptr == NULL)
	{
		cl_error(cl_FATAL, "Couldn't open file '%s' for reading\n", path);
	}

	if (options.verbose)
	{
		cl_info("Reading '%s'\n",  path);
	}

	/* We'll start off with an object */
	curr_obj = add_object(file, obj_EMPTY);

	while (process_result != process_FAILED)
	{
		int no_results;

		if (process_result == process_NOTDONE)
		{
			firstchar = linebuff;
			while (isspace(*firstchar)) firstchar++;
		}
		else
		{
			firstchar = read_line(fptr);
			if (firstchar == NULL)
			{
				process_result = process_FAILED;
				continue;
			}
		}

		if (*firstchar == '\0')
		{
			/*
			 * An empty line delimits one object from the next.
			 * To avoid waste, we only start a new object if the
			 * previous one wasn't empty.
			 */
			if (file->contents[curr_obj].type != obj_EMPTY)
			{
				curr_obj = add_object(file, obj_EMPTY);
			}

			process_result = process_DONE;

			continue;
		}

		/* The significant "abstract" comment */
		no_results = pcre_exec(regexes_too[re_DOCABSTRACT], NULL, linebuff,
							   strlen(linebuff), 0, 0, ovector,  OVECTOR_SIZE);
		if (no_results > 0)
		{
			process_result = process_abstract(file, fptr);
			continue;
		}

		/* An ordinary comment */
		if (firstchar[0] == '/' && firstchar[1] == '*')
		{
			if (file->contents[curr_obj].type == obj_EMPTY)
			{
				file->contents[curr_obj].type = obj_COMMENT;
			}
			else if (file->contents[curr_obj].type == obj_COMMENT)
			{
				curr_obj = add_object(file, obj_COMMENT);
			}

			process_result = read_comment(&file->contents[curr_obj].description,
										  fptr);
			continue;
		}

		if (strncmp(firstchar, "typedef", 7) == 0)
		{
			process_result = read_typedef(&file->contents[curr_obj], fptr);

			if (process_result == process_FAILED) continue;

			process_result = read_comment(&file->contents[curr_obj].description,
										  fptr);

			curr_obj = add_object(file, obj_EMPTY);

			continue;
		}

		if (strncmp(firstchar, "struct", 6) == 0 ||
			strncmp(firstchar, "union", 5) == 0 ||
			strncmp(firstchar, "enum", 4) == 0)
		{
			file->contents[curr_obj].type = obj_TYPE;

			process_result = read_structlike(&file->contents[curr_obj], fptr);

			if (process_result == process_FAILED) continue;

			process_result = read_comment(&file->contents[curr_obj].description,
										  fptr);

			curr_obj = add_object(file, obj_EMPTY);

			continue;
		}

		if (strncmp(firstchar, "#define", 7) == 0)
		{
			file->contents[curr_obj].type = obj_MACRO;

			process_result = read_macro(&file->contents[curr_obj], fptr);

			if (process_result == process_FAILED) continue;

			process_result = read_comment(&file->contents[curr_obj].description,
										  fptr);

			curr_obj = add_object(file, obj_EMPTY);

			continue;
		}

		if (strncmp(firstchar, "extern", 6) == 0)
		{
			process_result = read_extern(&file->contents[curr_obj], fptr);

			if (process_result == process_FAILED) continue;

			process_result = read_comment(&file->contents[curr_obj].description,
										  fptr);

			curr_obj = add_object(file, obj_EMPTY);

			continue;
		}

		/* Ignore #includes or anything else which gets this far - for now. */
		if (firstchar[0] == '#')
		{
			process_result = process_DONE;
			continue;
		}

		/* This is for everything else - we are only interested in a function */
		{
			const char *param_start;

			process_result = read_statement(&file->contents[curr_obj].definition,
										 fptr);

			if (process_result == process_FAILED) continue;

			/* Extract function name if this is a function. */
	        param_start = find_params(file->contents[curr_obj].definition,
	        						  strlen(file->contents[curr_obj].definition));

			if (param_start != NULL)
			{
				/* Extract name information and similar here */
				no_results = pcre_exec(regexes_too[re_names_FN], NULL,
									   file->contents[curr_obj].definition,
									   param_start - file->contents[curr_obj].definition + 1,
									   0, 0, ovector, OVECTOR_SIZE);

				if (ovector[4] != -1 && ovector[5] != -1)
				{
					if (ovector[3] == ovector[2])
					{
						file->contents[curr_obj].type = obj_FN;
					}
					else
					{
						file->contents[curr_obj].type = obj_VARIABLE;
					}

					add_name_n(&file->contents[curr_obj],
							   file->contents[curr_obj].definition + ovector[4],
							   ovector[5] - ovector[4]);

					process_result = read_comment(&file->contents[curr_obj].description,
												  fptr);

					curr_obj = add_object(file, obj_EMPTY);
				}
			}
			continue;
		}

		/* Stop it from processing "NOTDONE" lines forever */
		if (process_result == process_NOTDONE)
		{
			process_result = process_DONE;
		}
	}

	fclose(fptr);
}
