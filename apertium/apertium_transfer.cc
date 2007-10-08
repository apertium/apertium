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

using namespace Apertium;
using namespace std;

void message(char *progname)
{
  cerr << "USAGE: " << basename(progname) << " trules preproc biltrans [input [output]]" << endl;
  cerr << "  trules     transfer rules file" << endl;
  cerr << "  preproc    result of preprocess trules file" << endl;
  cerr << "  biltrans   bilingual letter transducer file" << endl;
  cerr << "  input      input file, standard input by default" << endl;
  cerr << "  output     output file, standard output by default" << endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  
  if(argc > 6 || argc <4)
  {
    message(argv[0]);
  }

  for(unsigned int i = 1; i < 4; i++)
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
  if(argc >= 5)
  {
    input = fopen(argv[4], "r");
    if(!input)
    {
      cerr << "Error: can't open input file '" << argv[4] << "'." << endl;
      exit(EXIT_FAILURE);
    }
    if(argc == 6)
    {
      output = fopen(argv[5], "w");
      if(!output)
      {
	cerr << "Error: can't open output file '";
	cerr << argv[5] << "'." << endl;
	exit(EXIT_FAILURE);
      }
    }
  }

  Transfer t;
  t.read(argv[1], argv[2], argv[3]);

  t.transfer(input, output);
  return EXIT_SUCCESS;
}
