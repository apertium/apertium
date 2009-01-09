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
  cerr << "       " << basename(progname) << " -x trules preproc biltrans extended [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -c trules preproc biltrans extended [input [output]]" << endl;
  cerr << "  trules     transfer rules file" << endl;
  cerr << "  preproc    result of preprocess trules file" << endl;
  cerr << "  biltrans   bilingual letter transducer file" << endl;
  cerr << "  input      input file, standard input by default" << endl;
  cerr << "  output     output file, standard output by default" << endl;
  cerr << "  -n         don't use bilingual dictionary" << endl;
  cerr << "  -x         extended mode with user dictionary" << endl;
  cerr << "  -c         case-sensitiveness while accessing bilingual dictionary" << endl;
  

  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  
  if(argc > 7 || argc <4)
  {
    message(argv[0]);
  }

  int firstfile = 1;
  if(!strcmp(argv[1],"-n") || !strcmp(argv[1], "-x") || !strcmp(argv[1], "-c"))
  {
    firstfile = 2;
  }
  
  for(int i = firstfile; i < argc; i++)
  {
    struct stat mybuf;
    if(stat(argv[i], &mybuf) == -1)
    {
      cerr << "Error: can't stat file '";
      cerr << argv[i] << "'." << endl;
      exit(EXIT_FAILURE);
    }
  }
  
  FILE *input = stdin, *output = stdout;
  int argclimit1 = 5;
  int argclimit2 = 6;
  
  if(!strcmp(argv[1],"-x"))
  {
    argclimit1 = 7;
    argclimit2 = 8;
  }
  
  if(argc >= argclimit1)
  {
    input = fopen(argv[argclimit1 - 1], "r");
    if(!input)
    {
      cerr << "Error: can't open input file '" << argv[argclimit1 - 1] << "'." << endl;
      exit(EXIT_FAILURE);
    }
    if(argc == argclimit2)
    {
      output = fopen(argv[argclimit2 - 1], "w");
      if(!output)
      {
	cerr << "Error: can't open output file '";
	cerr << argv[argclimit2 - 1] << "'." << endl;
	exit(EXIT_FAILURE);
      }
    }
  }

  #ifdef WIN32
  _setmode(_fileno(input), _O_U8TEXT);
  _setmode(_fileno(output), _O_U8TEXT);
#endif 

  Transfer t;
  if(firstfile == 1)
  {
    t.read(argv[1], argv[2], argv[3]);
  }
  else if(!strcmp(argv[1], "-n"))
  {
    t.read(argv[2], argv[3]);
    t.setUseBilingual(false);
  }
  else if(!strcmp(argv[1], "-c"))
  {
    t.read(argv[2], argv[3], argv[4]);
    t.setCaseSensitiveness(true);
  }
  else if(!strcmp(argv[1], "-x"))
  {
    t.read(argv[2], argv[3], argv[4]);
    t.setExtendedDictionary(argv[5]);
  }
  
  t.transfer(input, output);
  return EXIT_SUCCESS;
}
