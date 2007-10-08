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
#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <string>

#include <apertium/apertium_config.h>
#include <lttoolbox/lt_locale.h>
#include <apertium/tmx_builder.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>

using namespace Apertium;
void usage(char *progname)
{
  wcerr << L"USAGE: " << basename(progname) << L"doc1 doc2 [output_file]" << endl;
  wcerr << L"doc1, doc2    unformatted docs to build the TMX file" << endl;
  wcerr << L"output_file   if not specified, the result will be printed to stdout" << endl;
  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{ 
  LtLocale::tryToSetLocale();
  string output_file = "";
  string doc1 = "", doc2 = "";
  string lang1 = "", lang2 = "";
  
  switch(argc)
  {
    case 6:
      output_file = argv[5];
      // continued down  
    case 5:
      doc1 = argv[3];
      doc2 = argv[4];
      lang1 = argv[1];
      lang2 = argv[2];
      break;
      
    default:
      usage(argv[0]);
      return EXIT_FAILURE;
  } 
  
  TMXBuilder tmxb(UtfConverter::fromUtf8(lang1), UtfConverter::fromUtf8(lang2));
  if(!tmxb.check(doc1, doc2))
  {
    wcerr << L"Error: The two files are incompatible for building a TMX." << endl;
    exit(EXIT_FAILURE);
  }
  
  tmxb.generate(doc1, doc2, output_file);
  return EXIT_SUCCESS;
}
