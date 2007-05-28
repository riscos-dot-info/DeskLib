/* -------------------------------------------------- ajps, 27/02/03 ---
 * File: cl.c
 * Abstract: Handles the command line interface stuff.
 * Copyright (c) 2003 Antony Sidwell
 * --------------------------------------------------------------------- */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <ctype.h>

/* Unixlib stuff */
#include "glob.h"
#include "sys/stat.h"

#ifdef RISCOS
#include "unixlib/local.h"
#endif

#include "pcre.h"

#include "haddoc.h"
#include "cl.h"

/* An option or two */
struct hdoc_options options;

FILE *out_stream;
FILE *mess_stream;

/* char *dirpath; */
char *filename;

const char *default_libname = "&lt;unnamed library&gt;";
const char *libname;

const char *infofile;
const char *version = NULL;

static void opt_preopts(void);
static int opt_postopts(int argc, char *argv[]);

static char *outfile;
static char *dir;

int no_excl_patterns = 0;
pcre **excl_patterns;


struct
{
	const char *tok;
	BOOL *swch;
} cl_switches[] =
{
	{"v", &(options.verbose)},
	{"-verbose", &(options.verbose)},
#ifdef RISCOS
	{"-native", &(options.native)},
#endif
	{NULL, NULL}
};

/*
 * Takes the number of arguments remaining, and a pointer to
 * an array of those remaining arguments.  It should return the
 * number of arguments it has "used up", and call cl_error with
 * type cl_FATAL if it does not have sufficient parameters available.
 */
typedef int cl_fn(int argc, char *argv[], int ref);

static cl_fn opt_help;
static cl_fn opt_output;
static cl_fn opt_libname;
static cl_fn opt_infofile;
static cl_fn opt_exclude;
static cl_fn opt_leaf;
static cl_fn opt_version;

struct
{
	const char *tok;
	cl_fn	*fn;
	int ref;
} cl_fns[] =
{
	{"-help", opt_help, 0},
	{"h", opt_help, 0},
	{"-output", opt_output, 0},
	{"o", opt_output, 1},
	{"-name", opt_libname, 0},
	{"n", opt_libname, 1},
	{"-info", opt_infofile, 0},
	{"i", opt_infofile, 1},
	{"-exclude", opt_exclude, 0},
	{"x", opt_exclude, 1},
	{"-leaf", opt_leaf, 0},
	{"l", opt_leaf, 1},
	{"-version", opt_version, 0},
	{"V", opt_version, 1},
	{NULL, NULL, 0}
};


int cl_parse(int argc, char *argv[])
{
	int n;
	int next;
	int i;
	char *ptr;
	BOOL matched = FALSE;

	/* This is the usual behaviour for most utilities. */
	if (argc == 0) opt_help(0, NULL, 0);

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
				cl_error(cl_NONFATAL, "Option '-%s' is not known and has been ignored.\n", ptr);
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
				cl_error(cl_NONFATAL, "Option '-%c' is not known and has been ignored.\n", *ptr);
			}

	   		ptr ++;
	   	}
	}

	next--;
	next += opt_postopts(argc - next, argv + next);

	return (next);
}

/* ------------------------------------------------ ajps, 2003-02-28 ---
 * Output an informational message (eg. something enabled with a
 * "verbose" option) under the control of the format string "format".
 * --------------------------------------------------------------------- */
void cl_info(const char *format, ...)
{
	char temp[1024];
	va_list	argp;

	/* Initialise variable argument handling */
	va_start(argp,format);

	/* Print into the temp string */
	vsnprintf(temp, sizeof temp, format, argp);

	if (format[strlen(format) - 1] == '\n') temp[sizeof temp - 2] = '\n';
	fputs(temp, mess_stream);

	/* We are done with the variable argument stuff */
	va_end(argp);
}

/* ------------------------------------------------ ajps, 2003-02-28 ---
 * Outputs an error message (of type cl_FATAL or cl_NONFATAL) under the
 * control of the given format string.
 * --------------------------------------------------------------------- */
void cl_error(cl_errortype type, const char *format, ...)
{
	char temp[1024];
	va_list	argp;

	/* Initialise variable argument handling */
	va_start(argp,format);

	/* Print into the temp string */
	vsnprintf(temp, sizeof temp, format, argp);

	switch (type)
	{
		case cl_FATAL:
		{
			fprintf(stderr, "Error: ");
			break;
		}

		default:
		{
			fprintf(stderr, "Note: ");
			break;
		}
	}

	/* Ensure a newline even if the string is truncated. */
	if (format[strlen(format) - 1] == '\n') temp[sizeof temp - 2] = '\n';
	fputs(temp, stderr);

	/* We are done with the variable argument stuff */
	va_end(argp);

	switch (type)
	{
		case cl_FATAL:
		{
			fflush(stderr);
			exit(EXIT_FAILURE);
			break;
		}

		default: /* Do nothing for now */ break;
	}
}

static int opt_help(int argc, char *argv[], int ref)
{
	printf("haddoc version 0.23 (2003-06-13) Copyright (c) 2002-2003 Antony Sidwell.\n");
	printf("------------------------------------------------------------------------\n");
	printf("haddoc takes suitably-formatted C header files from a library and \n");
	printf("produces an XML file containing most of the information from those\n");
	printf("header files.  You can then produce whatever type of documentation you\n");
	printf("want using XSLT stylesheets, etc, etc.  See the documentation for\n");
	printf("details of the output format.\n\n");
	printf("Syntax: haddoc [<options>] [-l] <filename> [ [-l] <filename> ... ]\n\n");
	printf("Options:\n");
	printf("-h --help            Print this help text.\n");
	printf("-v --verbose         Print progress information.\n");
	printf("-o --output <file>   Output to the given file, otherwise output\n");
	printf("                     is on stdout.\n");
	printf("-n --name <name>     The name of the library being documented.\n");
	printf("-V --version <text>  A version string for the library.  This must not contain\n");
	printf("                     apostrophes ('s) or the output document will be malformed.\n");
	printf("-i --info <file>     File containing a valid XML information block for\n");
	printf("                     the library (see docs for details).\n");
	printf("-x --exclude <regex> Ignores any *lines* in the header files which match\n");
	printf("                     the given regular expression.  Note that some lines\n");
	printf("                     (like all '#if' lines and 'extern \"C\" {' lines) are\n");
	printf("                     ignored automatically.  You can specify more than\n");
	printf("                     one expression by using the option more than once.\n");
	printf("-l --leaf <number>   Defines how much of the path to a header file will be\n");
	printf("                     used as its name (by default only the leaf will be used).\n");
	printf("                     It gives the number of \"sections\" of the following\n");
	printf("                     filename, counting from the right.  Zero means use the\n");
	printf("                     whole path, no matter how many sections it is made up of.\n");

#ifdef RISCOS
	printf("--native             Use RISC OS filenames instead of UNIX-style ones.\n");
#endif
	exit(EXIT_SUCCESS);

	return 0;
}

static int opt_output(int argc, char *argv[], int ref)
{
	if (outfile != NULL)
	{
		cl_error(cl_FATAL, "You can only specify one output file.\n");
	}

	if (argc < 1)
	{
		cl_error(cl_FATAL, "The %s option requires a filename as an argument\n", (ref == 0) ? "--output" : "-o");
	}

	outfile = argv[0];

	return 1;
}

static int opt_libname(int argc, char *argv[], int ref)
{
	if (libname != default_libname)
	{
		cl_error(cl_FATAL, "You can only specify one name for the library.\n");
	}

	if (argc < 1)
	{
		cl_error(cl_FATAL, "The %s option requires a library name as an argument\n", (ref == 0) ? "--name" : "-n");
	}

	libname = argv[0];

	return 1;
}

static int opt_infofile(int argc, char *argv[], int ref)
{
	if (infofile != NULL)
	{
		cl_error(cl_FATAL, "You can only specify one info file to include in the output.\n");
	}

	if (argc < 1)
	{
		cl_error(cl_FATAL, "The %s option requires a filename as an argument\n", (ref == 0) ? "--info" : "-i");
	}

	infofile = argv[0];

	return 1;
}


static int opt_exclude(int argc, char *argv[], int ref)
{
	const char *error;
	int erroroffset;
	pcre *compiled;

	if (argc < 1)
	{
		cl_error(cl_FATAL, "The %s option requires a regular expression as an argument\n", (ref == 0) ? "--exclude" : "-x");
	}

	compiled = pcre_compile(argv[0], 0, &error, &erroroffset, NULL);

	if (compiled == NULL)
	{
		fprintf(stderr, "Error: Uncompilable regular expression:\n");
		fprintf(stderr, "%s near character %i\n", error, erroroffset);
		fprintf(stderr, "%s\n", argv[0]);
		fprintf(stderr, "%*c\n", erroroffset, '^');
		exit(EXIT_FAILURE);
	}
	else
	{
		/* Add to list of exclusions */
		pcre **temp;

		temp = realloc(excl_patterns, sizeof(*temp) * ++no_excl_patterns);
		if (temp == NULL)
		{
			cl_error(cl_FATAL, "Couldn't claim memory to store exclusion pattern\n");
		}

		excl_patterns = temp;
		excl_patterns[no_excl_patterns - 1] = compiled;
	}

	return 1;
}


static int opt_version(int argc, char *argv[], int ref)
{
	const char *error;
	int erroroffset;
	pcre *compiled;

	if (argc < 1)
	{
		cl_error(cl_FATAL, "The %s option requires a version string as an argument\n", (ref == 0) ? "--version" : "-V");
	}

    version = argv[0];

	return 1;
}

static int glob_errors(const char *msg, int err)
{
	return 1;
}

static void add_files(char *file_pattern, int leaflen)
{
	const char *glob_error[5] =
	{
		"Ran out of memory when matching '%s'\n",
		"Read error when matching '%s'\n",
		"Couldn't find directory when matching '%s'\n",
		"No matching filenames found when matching '%s'\n",
		"Wildcard matching not implemented.\n"
	};

	static int glob_flags = 0;
	glob_t globbed;
	char *glob_pattern = file_pattern;
	struct stat statdata;
	unsigned int n;
	const char *leaf;
	int leafn;
	int glob_ret = 0;

#ifdef RISCOS
	if (options.native) glob_pattern = unixify(file_pattern);
#endif

	glob_ret = glob(glob_pattern, glob_flags, glob_errors, &globbed);

	if (glob_ret != 0)
	{
	  cl_error(cl_FATAL, glob_error[glob_ret], file_pattern);
	}

	for (n = 0; n < globbed.gl_pathc; n++)
	{
		stat(globbed.gl_pathv[n], &statdata);

		if (S_ISDIR(statdata.st_mode))
		{
			if (options.verbose)
				cl_info("'%s' is a directory, and will be ignored.\n", globbed.gl_pathv[n]);
		}
		else
		{
			/* Yes, I know it's bad practise, but I exit if it fails */
			files = realloc(files, sizeof(*files) * (no_files + 1));
			filenames = realloc(filenames, sizeof(*filenames) * (no_files + 1));

			if (files == NULL || filenames == NULL)
				cl_error(cl_FATAL, "Couldn't claim enough memory.\n");

			filenames[no_files] = make_string(globbed.gl_pathv[n]);

			files[no_files].type = obj_FILE;
			files[no_files].definition = NULL;
			files[no_files].description = NULL;
			files[no_files].no_parts = 0;
			files[no_files].contents = NULL;
			files[no_files].part_of = NULL;

			if (leaflen == 0)
			{
				leaf = globbed.gl_pathv[n];
			}
			else
			{
				leafn = leaflen;
				leaf = globbed.gl_pathv[n] + strlen(globbed.gl_pathv[n]);

				while (leafn > 0 && leaf > globbed.gl_pathv[n])
				{
					leaf--;
					if (*leaf == '/') leafn--;
				}

				if (*leaf == '/') leaf++;
			}

			files[no_files].names[0] = make_string(leaf);
			files[no_files].no_names = 1;

			no_files++;
		}
	}

	globfree(&globbed);
}

static int opt_leaf(int argc, char *argv[], int ref)
{
	int leaflen = 0;
	char *end;

	if (argc < 2)
	{
		cl_error(cl_FATAL, "The %s option requires a number and filename as arguments\n", (ref == 0) ? "--leaf" : "-l");
	}

	leaflen = strtoul(argv[0], &end, 10);

	if (end == argv[0])
	{
		cl_error(cl_FATAL, "The %s option requires a number and filename as arguments\n", (ref == 0) ? "--leaf" : "-l");
	}

	add_files(argv[1], leaflen);

	return 2;
}


#ifdef RISCOS
char *unixify(const char *rfilename)
{
	char *new = malloc(strlen(rfilename) * 2);

	if (new == NULL || __unixify(rfilename, 0, new, strlen(rfilename) * 2, 0) == NULL)
	{
		cl_error(cl_FATAL, "Not enough space to convert the filename.\n");
	}

	return new;
}

char *riscosify(const char *rfilename)
{
	char *new = malloc(strlen(rfilename) * 2);

	if (new == NULL || __riscosify(rfilename, 0, __RISCOSIFY_NO_SUFFIX, new, strlen(rfilename) * 2, 0) == NULL)
	{
		cl_error(cl_FATAL, "Not enough space to convert the filename.\n");
	}

	return new;
}
#endif

/* ------------------------------------------------ ajps, 2003-02-28 ---
 * Called after the options have been processed.
 * --------------------------------------------------------------------- */
int opt_postopts(int argc, char *argv[])
{
	int n;

	if (argc <= 0 && no_files == 0)
	{
		cl_error(cl_FATAL, "You must specify a (wildcarded) list of header files to process.\n");
	}

	if (outfile != NULL)
	{
		mess_stream = stdout;

#ifdef RISCOS
		if (options.native)
		{
			filename = unixify(outfile);
		}
		else
#endif
		{
			filename = outfile;
		}
	}

#ifdef RISCOS
	if (infofile != NULL && options.native)
	{
		infofile = unixify(infofile);
	}
#endif

	for (n = 0; n < argc; n++)
	{
		add_files(argv[n], 1);
	}

	return argc;
}


/* ------------------------------------------------ ajps, 2003-02-28 ---
 * Set up defaults for options.
 * --------------------------------------------------------------------- */
static void opt_preopts()
{
	options.verbose = FALSE;
	options.native = FALSE;
	out_stream = stdout;
	mess_stream = stderr;

	libname = default_libname;
}

