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
#include <lttoolbox/file_utils.h>
#include <lttoolbox/string_utils.h>
#include "getopt_long.h"
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

void message(char *progname)
{
  cerr << "USAGE: " << basename(progname) << " trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -b trules preproc [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -n trules preproc [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -x extended trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -c trules preproc biltrans [input [output]]" << endl;
  cerr << "       " << basename(progname) << " -t trules preproc biltrans [input [output]]" << endl;
  cerr << "  trules     transfer rules file" << endl;
  cerr << "  preproc    result of preprocess trules file" << endl;
  cerr << "  biltrans   bilingual letter transducer file" << endl;
  cerr << "  input      input file, standard input by default" << endl;
  cerr << "  output     output file, standard output by default" << endl;
  cerr << "  -b         input from lexical transfer" << endl;
  cerr << "  -n         don't use bilingual dictionary" << endl;
  cerr << "  -x bindix  extended mode with user dictionary" << endl;
  cerr << "  -c         case-sensitiveness while accessing bilingual dictionary" << endl;
  cerr << "  -t         trace (show rule numbers and patterns matched)" << endl;
  cerr << "  -T         trace, for apertium-transfer-tools (also sets -t)" << endl;
  cerr << "  -w         ignore capitalization manipulation instructions" << endl;
  cerr << "  -z         null-flushing output on '\0'" << endl;
  cerr << "  -h         shows this message" << endl;


  exit(EXIT_FAILURE);
}

void testfile(string const &filename)
{
  struct stat mybuf;
  if(stat(filename.c_str(), &mybuf) == -1)
  {
    cerr << "Error: can't stat file '";
    cerr << filename << "'." << endl;
    exit(EXIT_FAILURE);
  }
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
      output = openOutTextFile(argv[argc-1]);
      input.open_or_exit(argv[argc-2]);
      testfile(argv[argc-3]);
      testfile(argv[argc-4]);
      testfile(argv[argc-5]);
      t.read(argv[argc-5], argv[argc-4], argv[argc-3]);
      break;

    case 5:
      if(t.getUseBilingual() == false || t.getPreBilingual() == true)
      {
        output = openOutTextFile(argv[argc-1]);
        input.open_or_exit(argv[argc-2]);
        testfile(argv[argc-3]);
        testfile(argv[argc-4]);
        t.read(argv[argc-4], argv[argc-3]);
      }
      else
      {
        input.open_or_exit(argv[argc-1]);
        testfile(argv[argc-2]);
        testfile(argv[argc-3]);
        testfile(argv[argc-4]);
        t.read(argv[argc-4], argv[argc-3], argv[argc-2]);
      }
      break;

    case 4:
      if(t.getUseBilingual() == false || t.getPreBilingual() == true)
      {
        input.open_or_exit(argv[argc-1]);
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
