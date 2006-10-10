This is an excerpt of C code that can be used to test the File_ library
code. It can be pasted into TestApp just after the hourglass test loop.
It also serves as a very basic example of usage of File_ calls.

  /* Test the file code by writing some stuff out, reading it back in */
  {
    file_handle infile, outfile;
    char *teststring = "This is a test string";
    char buffer[32];

    outfile = File_Open("<Tester$Dir>.testfile", file_WRITE);
    if (outfile == NULL)
    {
      if (file_lasterror != NULL)
        Error_Report(1, "Can't open testfile for writing: %s",
                        file_lasterror->errmess);
    }                    
    else
    {
      Error_Check(File_WriteBytes(outfile, teststring, 21));
      Error_Check(File_Write8(outfile, '*'));
      Error_Check(File_Write32(outfile, 'Zot!'));
      Error_Check(File_Write32R(outfile, 'Zot!'));
      Error_Check(File_Close(outfile));
    }

    if (File_Exists("<Tester$Dir>.testfile"))
    {
      infile = File_Open("<Tester$Dir>.testfile", file_READ);
      if (infile == NULL)
      {
        if (file_lasterror != NULL)
          Error_Report(1, "Can't open testfile for reading: %s",
                          file_lasterror->errmess);
      }
      else
      {
        if (File_Size("<Tester$Dir>.testfile") != 30)
          Error_Report(1, "Test file is not the length we expect! \n");

        File_ReadBytes(infile, buffer, 21);
        if (strncmp(buffer, teststring, 21))
          Error_Report(1, "File_Write/ReadBytes fails test! \n");

        if (File_Read8(infile) != '*')
          Error_Report(1, "File_Write/Read8 fails test! \n");

        if (File_Read32(infile) != 'Zot!')
          Error_Report(1, "File_Write/Read32 fails test! \n");

        if (File_Read32R(infile) != 'Zot!')
          Error_Report(1, "File_Write/Read32R fails test! \n");

        File_Close(infile);
      }
    }
  }
  Error_Check(File_Delete("<Tester$Dir>.testfile"));
