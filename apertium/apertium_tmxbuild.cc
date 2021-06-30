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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#include <cstdlib>
#include "getopt_long.h"
#include <iostream>
#include <libgen.h>
#include <string>
#include <cstdio>
#include <lttoolbox/lt_locale.h>

#include <apertium/apertium_config.h>
#include <apertium/tmx_builder.h>
#include <lttoolbox/string_utils.h>
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

using namespace std;

void usage(char *progname)
{
  cerr << "USAGE: " << basename(progname) << " [options] code1 code2 doc1 doc2 [output_file]" << endl;
  cerr << "Options:" << endl;
  cerr << "  -p percent    number 0 < n <= 1 to set margin of confidence of TU's " << endl;
  cerr << "                (0.85 by default) in length terms" << endl;
  cerr << "  -e edit       number 0 < n <= 1 to set margin of confidence of TU's " << endl;
  cerr << "                (0.30 by default) in edit distance terms" << endl;
  cerr << "  -l low-limit  ignore percent if the segment is less than lowlimit" <<endl;
  cerr << "                (15 by default)" << endl;
  cerr << "  -m max-edit   characters to be taken into account when aligning" << endl;
  cerr << "                sentences (50 by default)" << endl;
  cerr << "  -d diagonal   diagonal width for using edit distance, 10 by default" << endl;
  cerr << "  -w window     window size of the edit distance with sentences" << endl;
  cerr << "                (100 sentences by default)" << endl;
  cerr << "  -s step       step for moving the window during the alingment" <<endl;
  cerr << "                (75 sentences by default)" << endl;
  cerr << "  -h help       display this help" << endl;
  cerr << "Other parameters:" << endl;
  cerr << "  code1, code2 codes of the languages (i.e. ISO-631 ones)" << endl;
  cerr << "  doc1, doc2    unformatted docs to build the TMX file" << endl;
  cerr << "  output_file   if not specified, the result will be printed to stdout" << endl;

  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  string output_file = "";
  string doc1 = "", doc2 = "";
  UString lang1, lang2;

  double percent = 0.85;
  int low_limit = 15;
  int max_edit = 50;
  int diagonal_width = 10;
  int window_size = 100;
  int step = 75;
  double edit_distance_percent = 0.30;
  string translation = "";


  int option_index=0;

  while (true) {
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
        //cerr<<"Error: getopt() returned the char code '"<<c<<"'\n";
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
      lang1 = to_ustring(argv[optind - 1 + 1]);
      lang2 = to_ustring(argv[optind - 1 + 2]);
      break;

    default:
      usage(argv[0]);
      return EXIT_FAILURE;
  }

  TMXBuilder tmxb(lang1, lang2);
//  if(!tmxb.check(doc1, doc2))
//  {
//    cerr << "Error: The two files are incompatible for building a TMX." << endl;
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
