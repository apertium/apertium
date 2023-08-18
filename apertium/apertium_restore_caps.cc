/*
 * Copyright (C) 2022 Apertium
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

#include <apertium/caps_restorer.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/lt_locale.h>
#include <i18n.h>

int main(int argc, char** argv)
{
  LtLocale::tryToSetLocale();
  I18n i18n {APER_I18N_DATA, "apertium"};
  CLI cli(i18n.format("restore_caps_desc"));
  cli.add_bool_arg('k', "keep", i18n.format("keep_desc"));
  cli.add_bool_arg('z', "null-flush", i18n.format("null_flush_desc"));
  cli.add_bool_arg('h', "help", i18n.format("help_desc"));
  cli.add_file_arg("rule_file", false);
  cli.add_file_arg("input_file", true);
  cli.add_file_arg("output_file", true);
  cli.parse_args(argc, argv);

  CapsRestorer cr;

  cr.setDeleteWblanks(!cli.get_bools()["keep"]);

  FILE* in = openInBinFile(cli.get_files()[0]);
  cr.load(in);
  fclose(in);

  InputFile input;
  if (!cli.get_files()[1].empty()) {
    input.open_or_exit(cli.get_files()[1].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[2]);

  cr.process(input, output);

  u_fclose(output);
  return EXIT_SUCCESS;
}
