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
#include <apertium/tsx_reader.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/lt_locale.h>
#include <apertium/hmm.h>
#include <apertium/tagger_data_hmm.h>
#include <apertium/tagger_word.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/input_file.h>

#include <cstdlib>
#include <iostream>
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <libgen.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif
#include <i18n.h>
#include <unicode/ustream.h>

using namespace Apertium;
using namespace std;

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();

  if(argc < 2 || argc > 4)
  {
    cerr << I18n(APER_I18N_DATA, "apertium").format("usage") << ": " << basename(argv[0]) << " tsx_file [input [output]" << endl;
    exit(EXIT_FAILURE);
  }

  char* input = NULL;
  UFILE* output = u_finit(stdout, NULL, NULL);
  switch(argc)
  {
    case 4:
      output = u_fopen(argv[3], "w", NULL, NULL);
      if (!output) {
        I18n(APER_I18N_DATA, "apertium").error("APER1000", {"file_name"}, {argv[3]}, true);
      }
      // no break
    case 3:
      input = argv[2];
      // no break
    case 2:
    default:
      break;
  }

  TSXReader reader;
  reader.read(argv[1]);

  TaggerWord::setArrayTags(reader.getTaggerData().getArrayTags());

  TaggerDataHMM tdhmm(reader.getTaggerData());
  HMM hmm(&tdhmm);
  hmm.filter_ambiguity_classes(input, output);

  return EXIT_SUCCESS;
}
