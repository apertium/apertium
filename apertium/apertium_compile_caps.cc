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

#include <apertium/caps_compiler.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/lt_locale.h>
#include <i18n.h>

int main(int argc, char** argv)
{
  I18n i18n {APER_I18N_DATA, "apertium"};
  LtLocale::tryToSetLocale();
  CLI cli(i18n.format("compile_caps_desc"));
  cli.add_bool_arg('h', "help", i18n.format("help_desc"));
  cli.add_file_arg("rule_file", false);
  cli.add_file_arg("output_file", false);
  cli.parse_args(argc, argv);

  CapsCompiler cc;
  cc.parse(cli.get_files()[0]);
  FILE* output = openOutBinFile(cli.get_files()[1]);
  cc.write(output);
  fclose(output);
  return EXIT_SUCCESS;
}
