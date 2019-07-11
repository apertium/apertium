%module apertium_core

%{
#define SWIG_FILE_WITH_INIT
#include <apertium/interchunk.h>
#include <apertium/pretransfer.h>
#include <apertium/postchunk.h>
#include <apertium/tagger.h>
#include <apertium/transfer.h>

class apertium: public Transfer, public Interchunk, public Postchunk
{
public:
  /**
   * Imitates functionality of apertium-core binaries using file path
   */
  void interchunk_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path);
  void pretransfer(char arg, char *input_path, char *output_path);
  void postchunk_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path);
  void transfer_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path);
};

class apertium_tag: public Apertium::apertium_tagger
{
public:
  /**
   * Imitates functionality of apertium-tagger
   * Pass int to int&, char** to char**&
   */
  apertium_tag(int argc, char **argv): apertium_tagger(argc, argv) {;}
};

void
apertium::transfer_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path)
{
  FILE *input = fopen(input_path, "r"), *output = fopen(output_path, "w");

  switch(arg)
  {
      case 'b':
        setPreBilingual(true);
        setUseBilingual(false);
        break;

      case 'n':
        setUseBilingual(false);
        break;
  }
  Transfer::read(transferfile, datafile);
  transfer(input, output);
  fclose(input);
  fclose(output);
}

void
apertium::interchunk_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path)
{
  FILE *input = fopen(input_path, "r"), *output = fopen(output_path, "w");
  Interchunk::read(transferfile, datafile);
  interchunk(input, output);
  fclose(input);
  fclose(output);
}

void
apertium::pretransfer(char arg, char *input_path, char *output_path)
{
  bool useMaxEnt = false;
  FILE *input = fopen(input_path, "r"), *output = fopen(output_path, "w");
  processStream(input, output, false, false, false);
  fclose(input);
  fclose(output);
}

void
apertium::postchunk_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path)
{
  FILE *input = fopen(input_path, "r"), *output = fopen(output_path, "w");
  Postchunk::read(transferfile, datafile);
  postchunk(input, output);
  fclose(input);
  fclose(output);
}

%}

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
      PyObject *py_obj = PyList_GetItem($input,i);
      if (PyUnicode_Check(py_obj)){
        $1[i] = strdup(PyUnicode_AsUTF8(py_obj));
      }
      else {
        PyErr_SetString(PyExc_TypeError,"list must contain strings");
        free($1);
        return NULL;
      }
    }
    $1[i] = 0;
  } else {
    PyErr_SetString(PyExc_TypeError,"not a list");
    return NULL;
  }
}

%typemap(freearg) char ** {
  free((char *) $1);
}

class apertium: public Transfer, public Interchunk, public Postchunk
{
public:
  /**
   * Imitates functionality of apertium-core binaries using file path
   */
  void interchunk_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path);
  void pretransfer(char arg, char *input_path, char *output_path);
  void postchunk_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path);
  void transfer_text(char arg, char *transferfile, char *datafile, char *input_path, char *output_path);
};

class apertium_tag: public Apertium::apertium_tagger
{
public:
  /**
   * Imitates functionality of apertium-tagger
   * Pass int to int&, char** to char**&
   */
  apertium_tag(int argc, char **argv);
};
