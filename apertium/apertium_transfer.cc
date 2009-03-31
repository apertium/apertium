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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#include <apertium/transfer.h>
#include <lttoolbox/lt_locale.h>

#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <apertium/string_utils.h>
#include <getopt.h>
#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

void message(char *progname)
{
  cerr << "USAGE: " << basename(progname) << " trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -n trules preproc [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -x extended trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -c trules preproc biltrans [input [output]]" << endl;
  cerr << "  trules     transfer rules file" << endl;
  cerr << "  preproc    result of preprocess trules file" << endl;
  cerr << "  biltrans   bilingual letter transducer file" << endl;
  cerr << "  input      input file, standard input by default" << endl;
  cerr << "  output     output file, standard output by default" << endl;
  cerr << "  -n         don't use bilingual dictionary" << endl;
  cerr << "  -x bindix  extended mode with user dictionary" << endl;
  cerr << "  -c         case-sensitiveness while accessing bilingual dictionary" << endl;
  

  exit(EXIT_FAILURE);
}

void testfile(string const &filename)
{
  struct stat mybuf;
  if(stat(filename.c_str(), &mybuf) == -1)
  {
    cerr << "Error: can't stat file '";
    cerr << filename << "'." << endl;
    exit(EXIT_FAILURE);
  }
}

FILE * open_input(string const &filename)
{
  FILE *input = fopen(filename.c_str(), "r");
  if(!input)
  {
    cerr << "Error: can't open input file '";
    cerr << filename.c_str() << "'." << endl;
    exit(EXIT_FAILURE);
  }
  
  return input;
}  

FILE * open_output(string const &filename)
{
  FILE *output = fopen(filename.c_str(), "w");
  if(!output)
  {
    cerr << "Error: can't open output file '";
    cerr << filename.c_str() << "'." << endl;
    exit(EXIT_FAILURE);
  }
  return output;
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

#ifdef WIN32
  _setmode(_fileno(input), _O_U8TEXT);
  _setmode(_fileno(output), _O_U8TEXT);
#endif 
  
  Transfer t;
  
#if HAVE_GETOPT_LONG
  int option_index=0;
#endif

  while (true) {
#if HAVE_GETOPT_LONG
    static struct option long_options[] =
    {
      {"no-bilingual",      no_argument, 0, 'n'},
      {"extended",      required_argument, 0, 'x'},
      {"case-sensitive", required_argument, 0, 'c'},
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "nx:c", long_options, &option_index);
#else
    int c=getopt(argc, argv, "nx:c");
#endif
    if (c==-1)
      break;
      
    switch (c)
    {
      case 'n':
        t.setUseBilingual(false);
        break;
        
      case 'x':
        t.setExtendedDictionary(optarg);
        break;
        
      case 'c':
        t.setCaseSensitiveness(true);
        break;

      default:
        message(argv[0]);
        break;
    }    
  }

  FILE *input = stdin, *output = stdout;

  switch(argc - optind + 1)
  {
    case 6:
      output = open_output(argv[argc-1]);
      input = open_input(argv[argc-2]);
      testfile(argv[argc-3]);
      testfile(argv[argc-4]);
      testfile(argv[argc-5]);
      t.read(argv[argc-5], argv[argc-4], argv[argc-3]);
      break;
      
    case 5:
      if(t.getUseBilingual() == false)
      {
        output = open_output(argv[argc-1]);
        input = open_input(argv[argc-2]);
        testfile(argv[argc-3]);
        testfile(argv[argc-4]);
        t.read(argv[argc-4], argv[argc-3]);
      }
      else
      {
        input = open_input(argv[argc-1]);
        testfile(argv[argc-2]);
        testfile(argv[argc-3]);
        testfile(argv[argc-4]);
        t.read(argv[argc-4], argv[argc-3], argv[argc-2]);
      }
      break;
      
    case 4:
      if(t.getUseBilingual() == false)
      {
        input = open_input(argv[argc-1]);
        testfile(argv[argc-2]);
        testfile(argv[argc-3]);
        t.read(argv[argc-3], argv[argc-2]);
      }
      else
      {
        testfile(argv[argc-1]);
        testfile(argv[argc-2]);
        testfile(argv[argc-3]);
        t.read(argv[argc-3], argv[argc-2], argv[argc-1]);
      }
      break;
    case 3:
      if(t.getUseBilingual() == false)
      {
        testfile(argv[argc-1]);
        testfile(argv[argc-2]);
        t.read(argv[argc-2], argv[argc-1]);
      }
      else
      {
        message(argv[0]);
      }
      break;
    
    default:
      message(argv[0]);
      break;
  }  
  
  t.transfer(input, output);
  return EXIT_SUCCESS; 
}
