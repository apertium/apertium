/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#include <apertium/postchunk.h>
#include <lttoolbox/lt_locale.h>

#include <cstdlib>
#include "getopt_long.h"
#include <iostream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <apertium/string_utils.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

void message(char *progname)
{
  wcerr << "USAGE: " << basename(progname) << " [-z] t3x preproc [input [output]]" << endl;
  wcerr << "  t3x        t3x rules file" << endl;
  wcerr << "  preproc    result of preprocess trules file" << endl;
  wcerr << "  input      input file, standard input by default" << endl;
  wcerr << "  output     output file, standard output by default" << endl;
  wcerr << "OPTIONS" <<endl;
  wcerr << "  -t         trace (show rule numbers and patterns matched)" << endl;
  wcerr << "  -z         null-flushing output on '\0'" << endl;
  
  exit(EXIT_FAILURE);
}

void testfile(string const &filename)
{
  struct stat mybuf;
  if(stat(filename.c_str(), &mybuf) == -1)
  {
    wcerr << "Error: can't stat file '";
    wcerr << filename << "'." << endl;
    exit(EXIT_FAILURE);
  }
}

FILE * open_input(string const &filename)
{
  FILE *input = fopen(filename.c_str(), "r");
  if(!input)
  {
    wcerr << "Error: can't open input file '";
    wcerr << filename.c_str() << "'." << endl;
    exit(EXIT_FAILURE);
  }
  
  return input;
}  

FILE * open_output(string const &filename)
{
  FILE *output = fopen(filename.c_str(), "w");
  if(!output)
  {
    wcerr << "Error: can't open output file '";
    wcerr << filename.c_str() << "'." << endl;
    exit(EXIT_FAILURE);
  }
  return output;
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

  Postchunk p;
  
  int option_index=0;

  while (true) {
    static struct option long_options[] =
    {
      {"null-flush", no_argument, 0, 'z'},
      {"trace", no_argument, 0, 't'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "zht", long_options, &option_index);
    if (c == -1)
      break;
      
    switch (c)
    {
      case 't':
        p.setTrace(true);
        break;
 
      case 'z':
        p.setNullFlush(true);
        break;

      case 'h':
      default:
        message(argv[0]);
        break;
    }    
  }

  FILE *input = stdin, *output = stdout;
  string f1, f2;
  switch(argc - optind + 1)
  {
    case 5:
      output = open_output(argv[argc-1]);
      input = open_input(argv[argc-2]);
      testfile(argv[argc-3]);
      testfile(argv[argc-4]);
      f1 = argv[argc-4];
      f2 = argv[argc-3];
      break;
      
    case 4:
      input = open_input(argv[argc-1]);
      testfile(argv[argc-2]);
      testfile(argv[argc-3]);
      f1 = argv[argc-3];
      f2 = argv[argc-2];
      break;

    case 3:
      testfile(argv[argc-1]);
      testfile(argv[argc-2]);
      f1 = argv[argc-2];
      f2 = argv[argc-1];
      break;
    
    default:
      message(argv[0]);
      break;
  }  

#ifdef _MSC_VER
  _setmode(_fileno(input), _O_U8TEXT);
  _setmode(_fileno(output), _O_U8TEXT);
#endif

  p.read(f1, f2);
  p.postchunk(input, output);

  return EXIT_SUCCESS;
}
