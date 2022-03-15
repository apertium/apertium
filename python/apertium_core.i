%module apertium_core

%include <apertium/transfer_base.h>
%include <apertium/interchunk.h>
%include <apertium/pretransfer.h>
%include <apertium/postchunk.h>
%include <apertium/tagger.h>
%include <apertium/transfer.h>

// Wrapper on char ** for char **argv
// Modified for python 3 from https://www.swig.org/Doc1.3/Python.html#Python_nn59

%typemap(in) (int argc, char **argv) {
  if (PyTuple_Check($input)) {
    int i = 0;
    $1 = PyTuple_Size($input);
    $2 = (char **) malloc(($1 + 1)*sizeof(char *));
    for (i = 0; i < $1; i++) {
      PyObject *py_obj = PyTuple_GetItem($input, i);
      if (PyUnicode_Check(py_obj)) {
        $2[i] = strdup(PyUnicode_AsUTF8(py_obj));
      }
      else {
        PyErr_SetString(PyExc_TypeError, "tuple must contain strings");
        free($2);
        return NULL;
      }
    }
    $2[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not a tuple");
    return NULL;
  }
}

%typemap(freearg) (int argc, char **argv) {
  free((char *) $2);
}

%inline%{
#define SWIG_FILE_WITH_INIT
#include <apertium/interchunk.h>
#include <apertium/pretransfer.h>
#include <apertium/postchunk.h>
#include <apertium/tagger.h>
#include <apertium/transfer.h>

#include <getopt.h>

/**
 * Imitates functionality of apertium-core binaries using file path
 */


void pretransfer(int argc, char **argv, char *input_path, char *output_path)
{
  InputFile input;
  input.open(input_path);
  UFILE* output = u_fopen(output_path, "w", NULL, NULL);
  bool compound_sep = false;
  bool null_flush = false;
  bool surface_forms = false;

  optind = 1;
  while (true)
  {
    int c = getopt(argc, argv, "enz");
    if(c == -1)
    {
      break;
    }
    switch (c)
    {
      case 'z':
        null_flush = true;
        break;

      case 'e':
        compound_sep = true;
        break;

      case 'n':
        surface_forms = true;
        break;
      default:
        break;
    }
  }
  processStream(input, output, null_flush, surface_forms, compound_sep);
  u_fclose(output);
}

class ApertiumTransferBase: public TransferBase
{
};

class ApertiumTransfer: public Transfer
{
  public:

  ApertiumTransfer(char *transferfile, char *datafile)
  {
    read(transferfile, datafile);
  }

  void transfer_text(int argc, char **argv, char *input_path, char *output_path)
  {
	InputFile input;
	input.open(input_path);
    UFILE* output = u_fopen(output_path, "w", NULL, NULL);
    optind = 1;
    while (true)
    {
      int c = getopt(argc, argv, "nbx:cztT");
      if(c == -1)
      {
        break;
      }

      switch(c)
      {
        case 'b':
          setPreBilingual(true);
          setUseBilingual(false);
          break;

        case 'n':
          setUseBilingual(false);
          break;

        case 'x':
          setExtendedDictionary(optarg);
          break;

        case 'c':
          setCaseSensitiveness(true);
          break;

        case 't':
          setTrace(true);
          break;

        case 'T':
          setTrace(true);
          setTraceATT(true);
          break;

        case 'z':
          setNullFlush(true);
          break;
        default:
          break;
      }
    }
    transfer(input, output);
    u_fclose(output);
  }
};

class ApertiumTagger: public Apertium::apertium_tagger
{
  public:
  /**
   * Imitates functionality of apertium-tagger
   * tagger::tagger() passes int and char** to apertium_tagger::apertium_tagger() int&, char**& respectively
   */
  ApertiumTagger(int argc, char **argv): apertium_tagger(argc, argv){}
};

class ApertiumInterchunk: public Interchunk
{
  public:

  ApertiumInterchunk(char *transferfile, char *datafile)
  {
    read(transferfile, datafile);
  }

  void interchunk_text(int argc, char **argv, char *input_path, char *output_path)
  {
	InputFile input;
	input.open(input_path);
    UFILE* output = u_fopen(output_path, "w", NULL, NULL);
    optind = 1;
    while (true)
    {
      int c = getopt(argc, argv, "zt");
      if(c == -1)
      {
        break;
      }
      switch (c)
      {
        case 'z':
          setNullFlush(true);
          break;

        case 't':
          setTrace(true);
          break;
        default:
          break;
      }
    }
    interchunk(input, output);
    u_fclose(output);
  }
};

class ApertiumPostchunk: public Postchunk
{
  public:

  ApertiumPostchunk(char *transferfile, char *datafile)
  {
    read(transferfile, datafile);
  }

 void postchunk_text(int argc, char **argv, char *input_path, char *output_path)
  {
	InputFile input;
	input.open(input_path);
    UFILE* output = u_fopen(output_path, "w", NULL, NULL);
    optind = 1;
    while (true)
    {
      int c = getopt(argc, argv, "zt");
      if(c == -1)
      {
        break;
      }
      switch (c)
      {
        case 'z':
          setNullFlush(true);
          break;

        case 't':
          setTrace(true);
          break;
        default:
          break;
      }
    }
    postchunk(input, output);
    u_fclose(output);
  }
};

%}
