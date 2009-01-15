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
#include <getopt.h>
#include <iostream>
#include <libgen.h>
#include <string>
#include <cstdio>
#include <lttoolbox/lt_locale.h>

#include <apertium/apertium_config.h>
#include <apertium/tmx_builder.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>
#include <apertium/unlocked_cstdio.h>

using namespace Apertium;
using namespace std;

void usage(char *progname)
{
  wcerr << L"USAGE: " << basename(progname) << L" [-p percent] [-l lowlimit]  code1 code2 doc1 doc2 [output_file]" << endl;
  wcerr << L"Options:" << endl;
  wcerr << L"  -p percent    number 0 < n <= 1 to set margin of confidene of TU's " << endl;
  wcerr << L"                (0.95 by default)" << endl;
  wcerr << L"  -l lowlimit   skip percent if the segment is less than lowlimit" <<endl;
  wcerr << L"                (10 by default)" << endl;
  wcerr << L"Other parameters:" << endl;
  wcerr << L"  code1, code2 codes of the languages (i.e. ISO-631 ones)" << endl;
  wcerr << L"  doc1, doc2    unformatted docs to build the TMX file" << endl;
  wcerr << L"  output_file   if not specified, the result will be printed to stdout" << endl;
  
  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{ 
  LtLocale::tryToSetLocale();
  string output_file = "";
  string doc1 = "", doc2 = "";
  string lang1 = "", lang2 = "";
  double percent = 0.95;
  unsigned int lowlimit = 10;
  
#if HAVE_GETOPT_LONG
  int option_index=0;
#endif

  while (true) {
#if HAVE_GETOPT_LONG
    static struct option long_options[] =
    {
      {"percent",      required_argument, 0, 'p'},
      {"low-limit", required_argument, 0, 'l'},
      {"help",       no_argument,       0, 'h'}, 
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "p:l:h", long_options, &option_index);
#else
    int c=getopt(argc, argv, "p:l:h");
#endif
    if (c==-1)
      break;
      
    switch (c)
    {
      case 'p':
        percent = strtod(optarg, NULL);
        if(percent <= 0 || percent > 1)
        {
          usage(argv[0]);
        }
        break;
        
      case 'l':
        lowlimit = atoi(optarg);
        if(lowlimit < 0)
        {
          usage(argv[0]);
        }
        break;


      default:
        //wcerr<<L"Error: getopt() returned the char code '"<<c<<L"'\n";
        usage(argv[0]);
        break;
    }    
  }

  switch(argc - optind + 1)
  {
    case 6:
      output_file = argv[optind - 1 + 5];
      // continued down  
    case 5:
      doc1 = argv[optind - 1 + 3];
      doc2 = argv[optind - 1 + 4];
      lang1 = argv[optind - 1 + 1];
      lang2 = argv[optind - 1 + 2];
      break;
      
    default:
      usage(argv[0]);
      return EXIT_FAILURE;
  } 
  
  TMXBuilder tmxb(UtfConverter::fromUtf8(lang1), UtfConverter::fromUtf8(lang2));
//  if(!tmxb.check(doc1, doc2))
//  {
//    wcerr << L"Error: The two files are incompatible for building a TMX." << endl;
//    exit(EXIT_FAILURE);
//  }
  
  tmxb.setPercent(percent);
  tmxb.setLowLimit(lowlimit);
  tmxb.generate(doc1, doc2, output_file);
  return EXIT_SUCCESS;
}
