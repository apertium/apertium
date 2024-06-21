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
#include <apertium/transfer_mult.h>
#include <lttoolbox/lt_locale.h>
#include <apertium/apertium_config.h>

#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
#include <lttoolbox/i18n.h>
#include <unicode/ustream.h>

using namespace std;

void message(char *progname)
{
  I18n i18n {APR_I18N_DATA, "apertium"};
  cerr << i18n.format("usage") << ": " << basename(progname) << " preproc biltrans [input [output]]" << endl;
  cerr << "  preproc    " <<  i18n.format("preproc_desc") << endl;
  cerr << "  biltrans   " <<  i18n.format("biltrans_desc") << endl;
  cerr << "  input      " <<  i18n.format("input_desc") << endl;
  cerr << "  output     " <<  i18n.format("output_desc") << endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

  if(argc > 5 || argc <3)
  {
    message(argv[0]);
  }

  for(unsigned int i = 1; i < 3; i++)
  {
    struct stat mybuf;
    if(stat(argv[i], &mybuf) == -1)
    {
		  I18n(APR_I18N_DATA, "apertium").error("APR80000", {"file_name"}, {argv[i]}, true);
    }
  }

  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);
  if(argc >= 4)
  {
    if (!input.open(argv[3])) {
		  I18n(APR_I18N_DATA, "apertium").error("APR80000", {"file_name"}, {argv[3]}, true);
    }
    if(argc == 5)
    {
      output = u_fopen(argv[4], "w", NULL, NULL);
      if(!output)
      {
		    I18n(APR_I18N_DATA, "apertium").error("APR80000", {"file_name"}, {argv[4]}, true);
      }
    }
  }

  TransferMult t;
  t.read(argv[1], argv[2]);

  t.transfer(input, output);
  return EXIT_SUCCESS;
}
