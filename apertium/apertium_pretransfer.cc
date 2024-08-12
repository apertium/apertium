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

#include <apertium/pretransfer.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/lt_locale.h>

int main(int argc, char *argv[])
{
  LtLocale::tryToSetLocale();
  CLI cli("split compounds in preparation for transfer");
  cli.add_bool_arg('e', "compounds", "treat ~ as compound separator");
  cli.add_bool_arg('n', "no-surface-forms", "assume no surface forms");
  cli.add_bool_arg('z', "null-flush", "null-flushing output on '\\0'");
  cli.add_bool_arg('h', "help", "shows this message");
  cli.add_file_arg("input_file", true);
  cli.add_file_arg("output_file", true);
  cli.parse_args(argc, argv);

  std::string infile = cli.get_files()[0];
  InputFile input;
  if (!infile.empty()) input.open_or_exit(infile.c_str());
  UFILE* output = openOutTextFile(cli.get_files()[1]);

  processStream(input, output,
                cli.get_bools()["null-flush"],
                cli.get_bools()["no-surface-forms"],
                cli.get_bools()["compounds"]);
}
