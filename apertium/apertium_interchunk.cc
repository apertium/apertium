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
#include <lttoolbox/cli.h>
#include <lttoolbox/file_utils.h>

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  CLI cli("process stream with interchunker");
  cli.add_file_arg("t2x", false);
  cli.add_file_arg("preproc", false);
  cli.add_file_arg("input");
  cli.add_file_arg("output");
  cli.add_bool_arg('t', "trace", "trace mode");
  cli.add_bool_arg('w', "dictionary-case", "ignore capitalization manipulation instructions");
  cli.add_bool_arg('z', "null-flush", "flush buffer on '\\0'");
  cli.add_bool_arg('h', "help", "show this message and exit");
  cli.set_epilog("FILES:\n" \
                 "  t2x        t2x rules file\n"                   \
                 "  preproc    result of preprocess trules file\n" \
                 "  input      input file, standard input by default\n" \
                 "  output     output file, standard output by default");
  cli.parse_args(argc, argv);

  Interchunk i;

  i.setNullFlush(cli.get_bools()["null-flush"]);
  i.setTrace(cli.get_bools()["trace"]);
  i.setDictionaryCase(cli.get_bools()["dictionary-case"]);

  InputFile input;
  if (!cli.get_files()[2].empty()) {
    input.open_or_exit(cli.get_files()[2].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[3]);
  i.read(cli.get_files()[0].c_str(), cli.get_files()[1].c_str());
  i.interchunk(input, output);
  return EXIT_SUCCESS;
}
