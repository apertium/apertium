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
#include <i18n.h>
#include <unicode/ustream.h>

using namespace std;

void usage(char *progname)
{
  I18n i18n {APER_I18N_DATA, "apertium"};
  cerr << i18n.format("usage") << ": " << basename(progname)
       << " [options] code1 code2 doc1 doc2 [output_file]" << endl;
  cerr << i18n.format("options") << ":" << endl;
  cerr << "  -p percent    " <<  i18n.format("percent_desc") << endl;
  cerr << "  -e edit       " <<  i18n.format("edit_desc") << endl;
  cerr << "  -l low-limit  " <<  i18n.format("low_limit_desc") <<endl;
  cerr << "  -m max-edit   " <<  i18n.format("max_edit_desc") << endl;
  cerr << "  -d diagonal   " <<  i18n.format("diagonal_desc") << endl;
  cerr << "  -w window     " <<  i18n.format("window_desc") << endl;
  cerr << "  -s step       " <<  i18n.format("step_desc") << endl;
  cerr << "  -h help       " <<  i18n.format("help_desc") << endl;
  cerr << i18n.format("tmxbuild_note") << endl;

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
