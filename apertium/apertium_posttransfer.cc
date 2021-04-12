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
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <string>
#include "getopt_long.h"

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif
#include <apertium/string_utils.h>
#include <lttoolbox/lt_locale.h>


using namespace Apertium;
using namespace std;

void usage(char *progname)
{
  wcerr << L"USAGE: " << basename(progname) << L" [input_file [output_file]]" << endl;
  wcerr << L"  -z         null-flushing output on '\0'" << endl;
  wcerr << L"  -h         shows this message" << endl;
  exit(EXIT_FAILURE);
}

void processStream(FILE *in, FILE *out, bool null_flush)
{
  int prev = -1;
  int c = fgetc(in);
  while (c != EOF)
  {
    if (!((c == ' ') && (prev == ' ')))
    {
      putc(c, out);
    }
    if (c == 0 && null_flush)
    {
      fflush(out);
      putc(c, out);
    }
    prev = c;
    c = fgetc(in);
  }
}


int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  bool null_flush = false;

  int option_index=0;

  while (true) {
    static struct option long_options[] =
    {
      {"null-flush", no_argument, 0, 'z'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "zh", long_options, &option_index);
    if (c==-1)
      break;

    switch (c)
    {
      case 'z':
        null_flush = true;
        break;

      case 'h':
      default:
        usage(argv[0]);
        break;
    }
  }

  if((argc-optind+1) > 3)
  {
    usage(argv[0]);
  }

  FILE *input, *output;

  if((argc-optind+1) == 1)
  {
    input = stdin;
    output = stdout;
  }
  else if ((argc-optind+1) == 2)
  {
    input = fopen(argv[argc-1], "r");
    if(!input)
    {
      usage(argv[0]);
    }
    output = stdout;
  }
  else
  {
    input = fopen(argv[argc-2], "r");
    output = fopen(argv[argc-1], "w");

    if(!input || !output)
    {
      usage(argv[0]);
    }
  }

  if(feof(input))
  {
    wcerr << L"ERROR: Can't read file '" << argv[1] << L"'" << endl;
    exit(EXIT_FAILURE);
  }

  processStream(input, output, null_flush);

#ifdef _MSC_VER
    _setmode(_fileno(input), _O_U8TEXT);
    _setmode(_fileno(output), _O_U8TEXT);
#endif

}
