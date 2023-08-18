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
#include <apertium/transfer.h>
#include <lttoolbox/lt_locale.h>

#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <lttoolbox/string_utils.h>
#include "getopt_long.h"
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif
#include <i18n.h>
#include <unicode/ustream.h>

using namespace std;

void message(char *progname)
{
  I18n i18n {APER_I18N_DATA, "apertium"};
  cerr << i18n.format("usage") << ": "
       << basename(progname) << " trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -b trules preproc [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -n trules preproc [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -x extended trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -c trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -t trules preproc biltrans [input [output]]" << endl;
  cerr << "  trules     " <<  i18n.format("trules_desc") << endl;
  cerr << "  preproc    " <<  i18n.format("preproc_desc") << endl;
  cerr << "  biltrans   " <<  i18n.format("biltrans_desc") << endl;
  cerr << "  input      " <<  i18n.format("input_desc") << endl;
  cerr << "  output     " <<  i18n.format("output_desc") << endl;
  cerr << "  -b         " <<  i18n.format("from_bilingual_desc") << endl;
  cerr << "  -n         " <<  i18n.format("no_bilingual_desc") << endl;
  cerr << "  -x bindix  " <<  i18n.format("extended_desc") << endl;
  cerr << "  -c         " <<  i18n.format("case_sensitive_desc") << endl;
  cerr << "  -t         " <<  i18n.format("trace_desc") << endl;
  cerr << "  -T         " <<  i18n.format("trace_att_desc") << endl;
  cerr << "  -w         " <<  i18n.format("dictionary_case_desc") << endl;
  cerr << "  -z         " <<  i18n.format("null_flush_desc") << endl;
  cerr << "  -h         " <<  i18n.format("help_desc") << endl;


  exit(EXIT_FAILURE);
}

void testfile(string const &filename)
{
  struct stat mybuf;
  if(stat(filename.c_str(), &mybuf) == -1)
  {
		I18n(APER_I18N_DATA, "apertium").error("APER1000", {"file_name"}, {filename.c_str()}, true);
  }
}

void open_input(InputFile& input, const char* filename)
{
  if (!input.open(filename)) {
		I18n(APER_I18N_DATA, "apertium").error("APER1000", {"file_name"}, {filename}, true);
  }
}

UFILE* open_output(const char* filename)
{
  UFILE* output = u_fopen(filename, "w", NULL, NULL);
  if(!output) {
		I18n(APER_I18N_DATA, "apertium").error("APER1000", {"file_name"}, {filename}, true);
  }
  return output;
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

  Transfer t;

  int option_index=0;

  while (true) {
    static struct option long_options[] =
    {
      {"from-bilingual",      no_argument, 0, 'b'},
      {"no-bilingual",        no_argument, 0, 'n'},
      {"extended",      required_argument, 0, 'x'},
      {"case-sensitive",      no_argument, 0, 'c'},
      {"dictionary-case",     no_argument, 0, 'w'},
      {"null-flush",          no_argument, 0, 'z'},
      {"trace",               no_argument, 0, 't'},
      {"trace_att",           no_argument, 0, 'T'},
      {"help",                no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "nbx:cwztTh", long_options, &option_index);
    if (c==-1)
      break;

    switch (c)
    {
      case 'b':
        t.setPreBilingual(true);
        t.setUseBilingual(false);
        break;

      case 'n':
        t.setUseBilingual(false);
        break;

      case 'x':
        t.setExtendedDictionary(optarg);
        break;

      case 'c':
        t.setCaseSensitiveness(true);
        break;

      case 'w':
        t.setDictionaryCase(true);
        break;

      case 't':
        t.setTrace(true);
        break;

      case 'T':
        t.setTrace(true);
        t.setTraceATT(true);
        break;

      case 'z':
        t.setNullFlush(true);
        break;

      case 'h':
      default:
        message(argv[0]);
        break;
    }
  }

  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);

  switch(argc - optind + 1)
  {
    case 6:
      output = open_output(argv[argc-1]);
      open_input(input, argv[argc-2]);
      testfile(argv[argc-3]);
      testfile(argv[argc-4]);
      testfile(argv[argc-5]);
      t.read(argv[argc-5], argv[argc-4], argv[argc-3]);
      break;

    case 5:
      if(t.getUseBilingual() == false || t.getPreBilingual() == true)
      {
        output = open_output(argv[argc-1]);
        open_input(input, argv[argc-2]);
        testfile(argv[argc-3]);
        testfile(argv[argc-4]);
        t.read(argv[argc-4], argv[argc-3]);
      }
      else
      {
        open_input(input, argv[argc-1]);
        testfile(argv[argc-2]);
        testfile(argv[argc-3]);
        testfile(argv[argc-4]);
        t.read(argv[argc-4], argv[argc-3], argv[argc-2]);
      }
      break;

    case 4:
      if(t.getUseBilingual() == false || t.getPreBilingual() == true)
      {
        open_input(input, argv[argc-1]);
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
      if(t.getUseBilingual() == false || t.getPreBilingual() == true)
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
