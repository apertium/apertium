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

#include <cstring>
#include <string>
#include <iostream>
#include <cstdio>
#include <list>
#include <lttoolbox/ustring.h>
#include <lttoolbox/lt_locale.h>
#include <lttoolbox/input_file.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/file_utils.h>

#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <string>
#include <getopt.h>

#ifdef __MINGW32__
#include <windows.h>
#endif

void endProgram(char* name)
{
  if (name != NULL) {
    std::cout << basename(name) << ": Transfer capitalization information to word-bound blanks" << std::endl;
    std::cout << "USAGE: " << basename(name) << " [-sh] [ input_file [ output_file ] ]" << std::endl;
#if HAVE_GETOPT_LONG
    std::cout << "  -s, --surface:              keep surface forms" << std::endl;
    std::cout << "  -h, --help:                 print this message and exit" << std::endl;
#else
    std::cout << "  -s:     keep surface forms" << std::endl;
    std::cout << "  -h:     print this message and exit" << std::endl;
#endif
  }
  exit(EXIT_FAILURE);
}

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
  LtLocale::tryToSetLocale();

  bool keep_surf = false;

#if HAVE_GETOPT_LONG
  int option_index=0;
  static struct option long_options[] =
    {
     {"surface",    no_argument, 0, 's'},
     {"null-flush", no_argument, 0, 'z'},
     {"help",       no_argument, 0, 'h'},
     {0, 0, 0, 0}
    };
#endif

  while (true) {
#if HAVE_GETOPT_LONG
    int c = getopt_long(argc, argv, "szh", long_options, &option_index);
#else
    int c = getopt(argc, argv, "szh");
#endif

    if (c == -1) break;

    switch (c) {
    case 's':
      keep_surf = true;
      break;

    case 'z':
      break;

    case 'h':
    default:
      endProgram(argv[0]);
      break;
    }
  }

  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);

  if (optind < argc) input.open_or_exit(argv[optind++]);
  if (optind < argc) output = openOutTextFile(argv[optind++]);
  if (optind < argc) endProgram(argv[0]);

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
