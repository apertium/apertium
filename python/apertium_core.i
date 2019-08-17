%module apertium_core

%include <apertium/interchunk.h>
%include <apertium/pretransfer.h>
%include <apertium/postchunk.h>
%include <apertium/tagger.h>
%include <apertium/transfer.h>

// Wrapper on char ** for char **argv
// Modified for python 3 from http://www.swig.org/Doc1.3/Python.html#Python_nn59

%typemap(in) char ** {
  if (PyList_Check($input)) {
    int size = PyList_Size($input);
    int i = 0;
    $1 = (char **) malloc((size+1)*sizeof(char *));
    for (i = 0; i < size; i++) {
      PyObject *py_obj = PyList_GetItem($input, i);
      if (PyUnicode_Check(py_obj)) {
        $1[i] = strdup(PyUnicode_AsUTF8(py_obj));
      }
      else {
        PyErr_SetString(PyExc_TypeError, "list must contain strings");
        free($1);
        return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not a list");
    return NULL;
  }
}

%typemap(freearg) char ** {
  free((char *) $1);
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
  FILE* input = fopen(input_path, "r");
  FILE* output = fopen(output_path, "w");
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
  fclose(input);
  fclose(output);
}

class ApertiumTransfer: public Transfer
{
  public:

  ApertiumTransfer(char *transferfile, char *datafile)
  {
    read(transferfile, datafile);
  }

  void transfer_text(char argc, char **argv, char *input_path, char *output_path)
  {
    FILE* input = fopen(input_path, "r");
    FILE* output = fopen(output_path, "w");
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
    fclose(input);
    fclose(output);
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
    FILE* input = fopen(input_path, "r");
    FILE* output = fopen(output_path, "w");
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
    fclose(input);
    fclose(output);
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
    FILE* input = fopen(input_path, "r");
    FILE* output = fopen(output_path, "w");
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
    fclose(input);
    fclose(output);
  }
};

%}
