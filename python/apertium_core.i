%module apertium_core

%{
#define SWIG_FILE_WITH_INIT
#include <apertium/interchunk.h>
#include <apertium/pretransfer.h>
#include <apertium/postchunk.h>
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
%include <apertium/transfer.h>

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
