
#define OBJECT_ID_LEN 5

enum objtype
{
  objtype_FUNC = 0,
  objtype_MACROFN,
  objtype_MACRO,
  objtype_ENUMVAL,
  objtype_TYPE,
  objtype_GLOBAL,
  objtype_ANON,
  objtype_UNKNOWN,
  objtype_SIZE
};

struct object_record
{
  const xmlChar *name;
  const xmlChar *objectname;
  xmlNode *object;
  unsigned int type;
  unsigned int output;
};

int assemble_manual(xmlDoc *parsed, const char *ofilename);

