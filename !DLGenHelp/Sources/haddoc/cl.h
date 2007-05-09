/* -------------------------------------------------- ajps, 27/02/03 ---
 * File: cl.h
 * Abstract: Handles the command line interface stuff.
 * Copyright (c) 2003 Antony Sidwell
 * --------------------------------------------------------------------- */

typedef enum
{
	cl_FATAL = 0,
	cl_NONFATAL
} cl_errortype;

int cl_parse(int argc, char *argv[]);
void cl_error(cl_errortype type, const char *format, ...);
void cl_info(const char *format, ...);
#ifdef RISCOS
char *unixify(const char *rfilename);
char *riscosify(const char *rfilename);
#endif

/* Various options */
struct hdoc_options
{
	BOOL verbose;
	BOOL native;
};
extern char *dirpath;
extern char *filename;
extern struct hdoc_options options;
extern const char *default_libname;
extern const char *libname;
extern const char *infofile;
extern const char *version;

extern int no_excl_patterns;
extern pcre **excl_patterns;

extern FILE *out_stream;
extern FILE *mess_stream;
