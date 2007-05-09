
#define MAX_FILENAME 256

#ifdef SIMPLE_OUTPUT
  #define IFILE FILE
#else
  struct ifile
  {
    char *filename;
  };

  #define IFILE struct ifile
#endif

int wputs(const char *str, IFILE *stream);
int wputc(char c, IFILE *stream);
IFILE *open_output(const char *filename);
void close_output(IFILE *fptr);
void write_manual(const char *filename);

