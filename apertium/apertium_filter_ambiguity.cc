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
#include <apertium/tsx_reader.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/lt_locale.h>
#include <apertium/hmm.h>
#include <apertium/tagger_data_hmm.h>
#include <apertium/tagger_word.h>
#include <apertium/string_utils.h>

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

FILE * open_file(char const *filename, char const *mode)
{
  FILE *retval;

  struct stat var;  
  if(stat(filename, &var))
  {
    cerr << "Can't stat '" << filename << "'" << endl;
    exit(EXIT_FAILURE);
  }
 
  retval = fopen(filename, mode);
  
  if(!retval)
  {
    cerr << "Can't open '" << filename << "'" << endl;
    exit(EXIT_FAILURE);
  }
#ifdef _MSC_VER
  _setmode(_fileno(retval), _O_U8TEXT);
#endif   

  return retval;
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  
  if(argc < 2 || argc > 4)
  {
    cerr << "USAGE: " << basename(argv[0]) << " tsx_file [input [output]" << endl; 
    exit(EXIT_FAILURE);
  }

  FILE *input = stdin, *output = stdout;  
  switch(argc)
  {
    case 4:
      output = open_file(argv[3], "w");
      // no break
    case 3:      
      input = open_file(argv[2], "r");
      // no break
    case 2:
    default:
      break;
  }   
  
  TSXReader reader;
  reader.read(argv[1]);

  TaggerWord::setArrayTags(reader.getTaggerData().getArrayTags());

  TaggerDataHMM tdhmm(reader.getTaggerData());  
  HMM hmm(&tdhmm);
  hmm.filter_ambiguity_classes(input, output);
  
  return EXIT_SUCCESS;  
}
