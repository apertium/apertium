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
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

using namespace Apertium;
using namespace std;

void usage(char *progname)
{
  wcerr << L"USAGE: " << basename(progname) << L" [options] code1 code2 doc1 doc2 [output_file]" << endl;
  wcerr << L"Options:" << endl;
  wcerr << L"  -p percent    number 0 < n <= 1 to set margin of confidence of TU's " << endl;
  wcerr << L"                (0.85 by default) in length terms" << endl;
  wcerr << L"  -e edit       number 0 < n <= 1 to set margin of confidence of TU's " << endl;
  wcerr << L"                (0.30 by default) in edit distance terms" << endl;
  wcerr << L"  -l low-limit  ignore percent if the segment is less than lowlimit" <<endl;
  wcerr << L"                (15 by default)" << endl;
  wcerr << L"  -m max-edit   characters to be taken into account when aligning" << endl;
  wcerr << L"                sentences (50 by default)" << endl;
  wcerr << L"  -d diagonal   diagonal width for using edit distance, 10 by default" << endl;
  wcerr << L"  -w window     window size of the edit distance with sentences" << endl;
  wcerr << L"                (100 sentences by default)" << endl;
  wcerr << L"  -s step       step for moving the window during the alingment" <<endl;
  wcerr << L"                (75 sentences by default)" << endl;
  wcerr << L"  -h help       display this help" << endl;
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

  double percent = 0.85;
  int low_limit = 15;
  int max_edit = 50;
  int diagonal_width = 10;
  int window_size = 100;
  int step = 75;
  double edit_distance_percent = 0.30;
  string translation = "";


#if HAVE_GETOPT_LONG
  int option_index=0;
#endif

  while (true) {
#if HAVE_GETOPT_LONG
    static struct option long_options[] =
    {
      {"percent",      required_argument, 0, 'p'},
      {"edit-distance-percent",      required_argument, 0, 'e'},
      {"low-limit", required_argument, 0, 'l'},
      {"max-edit", required_argument, 0, 'm'},
      {"diagonal", required_argument, 0, 'd'},
      {"window", required_argument, 0, 'w'},
      {"step", required_argument, 0, 's'},
      {"translation", required_argument, 0, 't'},
      {"help",       no_argument,       0, 'h'}, 
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "p:e:l:m:d:w:s:t:h", long_options, &option_index);
#else
    int c=getopt(argc, argv, "p:e:l:m:d:w:s:t:h");
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
      case 'e':
        edit_distance_percent = strtod(optarg, NULL);
        if(edit_distance_percent <= 0 || edit_distance_percent > 1)
        {
          usage(argv[0]);
        }
        break;
        
      case 'l':
        low_limit = atoi(optarg);
        if(low_limit < 0)
        {
          usage(argv[0]);
        }
        break;

      case 'm':
        max_edit = atoi(optarg);
        if(max_edit < 0)
        {
          usage(argv[0]);
        }
        break;

      case 'd':
        diagonal_width = atoi(optarg);
        if(diagonal_width < 0)
        {
          usage(argv[0]);
        }
        break;

      case 'w':
        window_size = atoi(optarg);
        if(window_size < 0)
        {
          usage(argv[0]);
        }
        break;

      case 's':
        step = atoi(optarg);
        if(step < 0)
        {
          usage(argv[0]);
        }
        break;
     
      case 't':
	translation = optarg;
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
   
  // Set parameters

  tmxb.setPercent(percent);
  tmxb.setEditDistancePercent(edit_distance_percent);
  tmxb.setMaxEdit(max_edit);
  tmxb.setDiagonalWidth(diagonal_width);
  tmxb.setWindowSize(window_size);
  tmxb.setStep(step);
  tmxb.setLowLimit(low_limit);
  if(translation != "")
  {
    tmxb.setTranslation(translation);  
  }
  
  tmxb.generate(doc1, doc2, output_file);
  return EXIT_SUCCESS;
}
