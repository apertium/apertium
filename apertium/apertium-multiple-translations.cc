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
#include <apertium/transfer_mult.h>
#include <lttoolbox/lt_locale.h>
#include <apertium/apertium_config.h>

#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef WIN32
#if defined(__MINGW32__)
#define __MSVCRT_VERSION__  0x0800
#endif
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

void message(char *progname)
{
  cerr << "USAGE: " << basename(progname) << " preproc biltrans [input [output]]" << endl;
  cerr << "  preproc    result of preprocess trules file" << endl;
  cerr << "  biltrans   bilingual letter transducer file" << endl;
  cerr << "  input      input file, standard input by default" << endl;
  cerr << "  output     output file, standard output by default" << endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  
  if(argc > 5 || argc <3)
  {
    message(argv[0]);
  }

  for(unsigned int i = 1; i < 3; i++)
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
  if(argc >= 4)
  {
    input = fopen(argv[3], "r");
    if(!input)
    {
      cerr << "Error: can't open input file '" << argv[3] << "'." << endl;
      exit(EXIT_FAILURE);
    }
    if(argc == 5)
    {
      output = fopen(argv[4], "w");
      if(!output)
      {
	cerr << "Error: can't open output file '";
	cerr << argv[4] << "'." << endl;
	exit(EXIT_FAILURE);
      }
    }
  }
#ifdef WIN32
  _setmode(_fileno(input), _O_U8TEXT);
  _setmode(_fileno(output), _O_U8TEXT);
#endif 

  TransferMult t;
  t.read(argv[1], argv[2]);

  t.transfer(input, output);
  return EXIT_SUCCESS;
}
