/* ---------------------------------------------------------------------
 * File: regexps.c
 * Abstract: Stores the regular expressions used in the rest of haddoc,
 * and has the function to compile them, too.
 * Copyright (c) 2003 Antony Sidwell 
 * --------------------------------------------------------------------- */

#include <stdio.h>

/* Compile-time options */
/* #define SHOW_RE_COMPILES */

#include "pcre.h"
#include "regexps.h"

const char *patterns_too[NO_PATTERNS_TOO] =
{
	/* File filter to ignore "CVS", ".", and ".." */
	"^(CVS|\\.|\\.\\.)$",

    /* "Special" lines */
	"(?i)^\\s*/\\*\\s*haddoc ignore off\\s*\\*/",
	"(?i)^\\s*/\\*\\s*haddoc ignore on\\s*\\*/",
	"(?i)^\\s*/\\*\\s*haddoc ignore line\\s*\\*/",
	"(?i)^\\s*/\\*\\s*(?:abstract|overview)\\s*\\*/",

	/* Opening a comment.  Is this really necessary? */
	"/\\*",
	"^\\s*/\\*",
	"\\*/",

	/* Lines we ignore whatever happens */
	"^\\s*(#\\s*if|#\\s*else|#\\s*endif|\\s*extern\\s*\"C\")",

	/* Qualifiers */
	"^\\s*(?:(const)|(volatile)|(restrict))\\s*",

	/* Grabbing the names from "crushed" descriptions */
	"^\\s*((?:struct|enum|union) [A-Za-z_][A-Za-z0-9_]*)",   /* struct-like */
	"^\\s*typedef\\s+((?:struct|enum|union)( [A-Za-z_][A-Za-z0-9_]*)?) ?{.*}\\s*(.*);",
	"^\\s*#\\s*define\\s*([A-Za-z_][A-Za-z0-9_]*)\\(",
	"^\\s*#\\s*define\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*",

	"^.*?(\\(\\s*\\*.*?)?([A-Za-z_][A-Za-z0-9_]*)(?:\\s*|\\))*\\($",

	"^.*?([A-Za-z_][A-Za-z0-9_]*)\\s*(?:(?::|\\[|\\)).*?)?\\s*(?:,|;)",

	/*
	 * Grabbing types from a <type> <name> [,<name>]; scenario
	 */
	"^("
	/* The contents of a structlike thing  OR */
	".*}.*?"  "|"
	/* Any characters at all */
	".*?"  ")"
	/* Either of these up to any character (or sequence of characters)
	   guaranteed to be after the "type" part and part of the "name" part */
	"(?:[*\\(;,)]|[A-Za-z_][A-Za-z0-9_]*\\s*[;,:[)])",

	/*
	 * Splitting qualifiers from the "type" part of a definition.
	 */

	/* We match any number of type qualifiers */
	"^\\s*(?:extern)?\\s*((?:\\s*const|\\s*volatile|\\s*restrict)*)\\s*"
	/* Then the type text itself */
	"(.*?)"
	/* And then any number of type qualifiers */
	"((?:\\s*const|\\s*volatile|\\s*restrict)*)\\s*$",

	/*
	 * Finding the enumerated values in an enum
	 */
	"^\\s*([A-Za-z_][0-9A-Za-z_]*)\\s*(?:=\\s*(.+?))?\\s*(,|})",

	/*
	 * Detecting the absence of names.
	 */
	"^[\\s;,)]*$",

	/*
	 * Function parameters.
	 */
	"^(.*?)(?:,|\\))",

	/*
	 * Function return value, name, etc.
	 */
 	"^(.*?)((?:\\([(* ]*)*[A-Za-z_][A-Za-z0-9_]*(?:\\s*|\\))*)\\($",

	/*
	 * Check for an incomplete struct declaration.
	 */
	"^\\s*struct\\s*([A-Za-z_][A-Za-z0-9_]*)\\s*;\\s*$",

	/*
	 * Search for an enum.
	 */
	"enum\\s*[A-Za-z0-9_ ]({.*?})",

	/*
	 * An identifer.
	 */
	"^\\s*([A-Za-z_][A-Za-z0-9_]*)",
};

pcre *regexes_too[NO_PATTERNS_TOO];

/* We compile them once and once only */
void compile_patterns(void)
{
	int n;
	const char *error;
	int erroroffset;

	for (n = 0; n < NO_PATTERNS_TOO; n++)
	{
#ifdef SHOW_RE_COMPILES
		cl_info("Compiling '%s'\n", patterns_too[n]);
#endif
		regexes_too[n] = pcre_compile(patterns_too[n], 0, &error, &erroroffset, NULL);

		if (regexes_too[n] == NULL)
		{
			fprintf(stderr, "Error compiling expression %i:\n", n);
			fprintf(stderr, "%s\n", error);
			fprintf(stderr, "near character %i\n", erroroffset);
			fprintf(stderr, "%s\n", patterns_too[n]);
			fprintf(stderr, "%*c\n", erroroffset, '^');
			exit(EXIT_FAILURE);
		}
	}
}
