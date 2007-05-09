/* Copyright (c) 2003 Antony Sidwell  */

enum
{
	re_FILEFILTER = 0,

	re_DOCON,
	re_DOCOFF,
	re_DOCSKIP,
	re_DOCABSTRACT,

	re_SIGCOMMOPEN,
	re_COMMOPEN,
	re_COMMCLOSE,

	re_IGNORED,
	re_QUALIFIERS,

	re_names_STRUCTLIKE,
	re_names_TYPEDSTRUCTLIKE,
	re_names_MACROFN,
	re_names_MACRO,
	re_names_FN,
	re_names_VARIABLE,

	re_output_TYPE,

	re_output_TYPESIMPLE,

	re_output_ENUMVAL,

	re_output_NONAMES,

	re_output_FUNCPARAMS,
	re_output_FUNC,

	re_output_INCOMPLETESTRUCT,

	re_index_FINDENUM,

	re_output_IDENTIFIER,

	NO_PATTERNS_TOO
};

#if 0
enum
{
	/* General stuff, not to be stored */
	re_FILEFILTER = 0,
	re_CONDITION,        /* Other conditions */
	re_EMPTYLINE,

	/* Objects we wish to collect information on */
	re_TYPEDEFSTRUCT,
	re_TYPEDEFENUM,
	re_ENDTYPEDEF,
	re_ENUM,
	re_ENDENUM,
	re_TYPEDEFFN,
	re_TYPEDEFFNEND,
	re_FN,
	re_FNCLOSE,
	re_SIGCOMMOPEN,
	re_COMMOPEN,
	re_COMMCLOSE,
	re_EXTERN,
	re_FNMACRO,
	re_MACRO,
	re_MACROCONT,
	re_TYPEDEF,
	re_STRUCT,
	re_TYPEDEFUNION,

	/* End-of-enum counter */
	NO_PATTERNS
};

extern pcre *regex[NO_PATTERNS];

enum
{
	/* Output helpers */
	re_COMMFORMAT,
	re_COMMEMPTY,

	/* Lets us extract the type (r1), name (r3) and params(r4) */
	re_FNCHOP,
	re_FNARGCHOP,

	/* Extracting the contents of a struct or similar */
	re_TYPEDCHOP,
	re_ENUMCHOP,
	re_STRUCTCHOP,
	re_MEMBERCHOP,
	re_MEMBVARCHOP,

	re_ENUMMCHOP,

	re_TYPEDEFCHOP,

	re_MACROFCHOP,

	re_GLOBALCHOP,

	re_FNTYPECHOP,
	re_FNTYPECHOP2,

	NO_OUT_PATTERNS
};

extern pcre *out_regex[NO_OUT_PATTERNS];
#endif
void compile_patterns(void);

pcre *regexes_too[NO_PATTERNS_TOO];
