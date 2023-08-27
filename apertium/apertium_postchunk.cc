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
#include <apertium/postchunk.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/file_utils.h>
#include <i18n.h>

int main(int argc, char *argv[])
{
  I18n i18n {APER_I18N_DATA, "apertium"};
  LtLocale::tryToSetLocale();
  CLI cli(i18n.format("postchunck_desc"));
  cli.add_file_arg("t3x", false);
  cli.add_file_arg("preproc", false);
  cli.add_file_arg("input");
  cli.add_file_arg("output");
  cli.add_bool_arg('t', "trace", i18n.format("trace_desc"));
  cli.add_bool_arg('w', "dictionary-case", i18n.format("dictionary_case_desc"));
  cli.add_bool_arg('z', "null-flush", i18n.format("null_flush_desc"));
  cli.add_bool_arg('h', "help", i18n.format("help_desc"));
  cli.set_epilog(i18n.format("postchunk_epilog"));
  cli.parse_args(argc, argv);

  Postchunk p;

  p.setNullFlush(cli.get_bools()["null-flush"]);
  p.setTrace(cli.get_bools()["trace"]);
  p.setDictionaryCase(cli.get_bools()["dictionary-case"]);

  InputFile input;
  if (!cli.get_files()[2].empty()) {
    input.open_or_exit(cli.get_files()[2].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[3]);
  p.read(cli.get_files()[0].c_str(), cli.get_files()[1].c_str());
  p.postchunk(input, output);

  return EXIT_SUCCESS;
}
