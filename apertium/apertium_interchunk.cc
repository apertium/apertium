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
#include <apertium/interchunk.h>
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

using namespace std;

void message(char *progname)
{
  cerr << "USAGE: " << basename(progname) << " [-tz] t2x preproc [input [output]]" << endl;
  cerr << "  t2x        t2x rules file" << endl;
  cerr << "  preproc    result of preprocess trules file" << endl;
  cerr << "  input      input file, standard input by default" << endl;
  cerr << "  output     output file, standard output by default" << endl;
  cerr << "OPTIONS" <<endl;
  cerr << "  -t         trace mode" << endl;
  cerr << "  -z         flush buffer on '\0'" << endl;

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

void open_input(InputFile& input, const char* filename)
{
  if (!input.open(filename)) {
    cerr << "Error: can't open input file '";
    cerr << filename << "'." << endl;
    exit(EXIT_FAILURE);
  }
}

UFILE * open_output(const char* filename)
{
  UFILE* output = u_fopen(filename, "w", NULL, NULL);
  if(!output)
  {
    cerr << "Error: can't open output file '";
    cerr << filename << "'." << endl;
    exit(EXIT_FAILURE);
  }
  return output;
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

  Interchunk i;

  int option_index=0;

  while (true) {
    static struct option long_options[] =
    {
      {"null-flush", no_argument, 0, 'z'},
      {"trace", no_argument, 0, 't'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "zth", long_options, &option_index);
    if (c == -1)
      break;

    switch (c)
    {
      case 'z':
        i.setNullFlush(true);
        break;

      case 't':
        i.setTrace(true);
        break;

      case 'h':
      default:
        message(argv[0]);
        break;
    }
  }

  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);
  const char* f1;
  const char* f2;
  switch(argc - optind + 1)
  {
    case 5:
      output = open_output(argv[argc-1]);
      open_input(input, argv[argc-2]);
      testfile(argv[argc-3]);
      testfile(argv[argc-4]);
      f1 = argv[argc-4];
      f2 = argv[argc-3];
      break;

    case 4:
      open_input(input, argv[argc-1]);
      testfile(argv[argc-2]);
      testfile(argv[argc-3]);
      f1 = argv[argc-3];
      f2 = argv[argc-2];
      break;

    case 3:
      testfile(argv[argc-1]);
      testfile(argv[argc-2]);
      f1 = argv[argc-2];
      f2 = argv[argc-1];
      break;

    default:
      message(argv[0]);
      break;
  }

  i.read(f1, f2);
  i.interchunk(input, output);
  return EXIT_SUCCESS;
}
