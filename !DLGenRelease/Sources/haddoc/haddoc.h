/* Copyright (c) 2003 Antony Sidwell  */

#define TRUE 1
#define FALSE 0
#define BOOL int


/*
 * These are all slightly over-large already, they should suffice
 * for any reasonable input file.
 */
#define LINEBUFF_SIZE 4096
#define OVECTOR_SIZE 30
#define MAX_NAMES_PER_OBJECT 16

typedef enum
{
	obj_FILE = 0,

	obj_FN,
	obj_FNTYPE,
	obj_VARIABLE,
	obj_COMMENT,
	obj_EMPTY,
	obj_TYPE,
	obj_MACROFN,
	obj_MACRO,

	NO_OBJECTTYPES
} object_type;

/*
 * We can use this one type for holding file information as well as
 * the various functions, typedefs and such.
 */
typedef struct object_info object_info;
struct object_info
{
	object_type type;
	int no_names;
	char *names[MAX_NAMES_PER_OBJECT];
	char *definition;
	char *description;
	int no_parts;
	object_info *contents;
	object_info *part_of;
	unsigned int id;
};

/* We may as well make these globals, it saves passing them all over the place */
extern char linebuff[LINEBUFF_SIZE];
extern int ovector[OVECTOR_SIZE];

/* We'll assign this to be an array of files */
extern object_info *files;

/* Adds a name to the list of those stored for this object. */
#define add_name(object, name) add_name_n(object, name, 0)
void add_name_n(object_info *object, const char *name, size_t len);

int add_object(object_info *object, object_type type);

#define add_text(text, to_add) add_text_n(text, to_add, 0)
BOOL add_text_n(char **text, const char *to_add, size_t len);

#define make_string(text) make_string_n(text, 0)
char *make_string_n(const char *string, size_t n);

char *make_result_string (int number);
void free_string(char *ptr);
void condense_whitespace(char *text);
void copy_result_string(const char *result, int number, char *string);

extern int no_files;

extern object_info *files;
extern const char **filenames;

/* deallocate a text block */
void free_text(char **ptr);

enum qualifiers
{
	qualifier_CONST = 0,
	qualifier_VOLATILE,
	qualifier_RESTRICT,
	NO_QUALIFIERS
};

int check_qualifier(const char **text, int qualifiers);
const char *find_params(const char *definition, size_t len);

