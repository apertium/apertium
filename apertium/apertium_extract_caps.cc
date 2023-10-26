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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <lttoolbox/ustring.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/file_utils.h>
#include <lttoolbox/cli.h>
#include <lttoolbox/i18n.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

UString read_until_slash(InputFile& input)
{
  UString ret;
  while (!input.eof()) {
    UChar32 c = input.get();
    if (c == '\\') {
      ret += c;
      ret += input.get();
    } else if (c == '/' || c == '$' || c == '\0') {
      input.unget(c);
      break;
    } else {
      ret += c;
    }
  }
  return ret;
}

std::pair<UString, UString> read_lu(InputFile& input)
{
  UString surf = read_until_slash(input);
  UString reading1;
  if (input.peek() == '/') {
    input.get();
    reading1 = read_until_slash(input);
    while (!input.eof() && input.peek() != '\0' && input.peek() != '$') {
      input.get();
      read_until_slash(input);
    }
  }
  if (input.peek() == '$') input.get();
  return std::make_pair(surf, reading1);
}

int main(int argc, char* argv[])
{
  I18n i18n {APR_I18N_DATA, "apertium"};
  LtLocale::tryToSetLocale();
  CLI cli(i18n.format("extract_caps_desc"));
  cli.add_file_arg("input_file");
  cli.add_file_arg("output_file");
  cli.add_bool_arg('s', "surface", i18n.format("surface_desc"));
  cli.add_bool_arg('z', "null-flush", i18n.format("null_flush_desc"));
  cli.add_bool_arg('h', "help", i18n.format("help_desc"));
  cli.parse_args(argc, argv);

  bool keep_surf = cli.get_bools()["surface"];

  InputFile input;
  if (!cli.get_files()[0].empty()) {
    input.open_or_exit(cli.get_files()[0].c_str());
  }
  UFILE* output = openOutTextFile(cli.get_files()[1]);

  while (!input.eof()) {
    write(input.readBlank(false), output);
    UChar32 c = input.get();
    if (c == '\0') {
      u_fputc(c, output);
      u_fflush(output);
    }
    if (c == '\0' || c == U_EOF) continue;

    UString wblank;
    if (c == '[') {
      input.get();
      wblank = input.finishWBlank();
      c = input.get();
    }
    if (c != '^') {
      input.unget(c);
      write(wblank, output);
      continue;
    }
    auto lu = read_lu(input);
    UString surf_case = StringUtils::getcase(lu.first);
    UString::size_type loc = lu.second.find('<');
    UString lemma_case = StringUtils::getcase(lu.second.substr(0, loc));
    UString new_wblank = "[[c:"_u;
    new_wblank += surf_case;
    new_wblank += '/';
    new_wblank += lemma_case;
    new_wblank += "]]"_u;
    write(StringUtils::merge_wblanks(wblank, new_wblank), output);
    u_fputc('^', output);
    if (keep_surf) {
      write(lu.first, output);
      u_fputc('/', output);
    }
    write(lu.second, output);
    u_fputc('$', output);
  }

  u_fclose(output);
  return EXIT_SUCCESS;
}
