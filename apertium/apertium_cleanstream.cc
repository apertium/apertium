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

#ifdef __MINGW32__
#include <windows.h>
#endif

using namespace std;

UString
readFullBlock(InputFile& input, UChar32 const delim1, UChar32 const delim2)
{
  UString result;
  result += delim1;
  UChar32 c = delim1;

  while(!input.eof() && c != delim2) {
    c = input.get();
    result += c;
    if(c != '\\') {
      continue;
    } else {
      result += '\\';
      c = input.get();
      result += c;
    }
  }

  if(c != delim2)
  {
    cerr << "Error: expected: " << delim2 << ", saw: " << c << endl;
  }

  return result;
}

int
main (int argc, char** argv)
{
  UString buf;
  UString blanktmp;
  bool keepblank = false;

  bool spaced = true;
  bool intoken = false;

  UChar32 ws = ' ';

  for(int i=1; i<argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      spaced = false;
      ws = '\n';
    }
    else if (strcmp(argv[i], "-b") == 0) {
      keepblank = true;
    }
  }

  LtLocale::tryToSetLocale();

  InputFile input;
  UFILE* output = u_finit(stdout, NULL, NULL);
  UChar32 c;
  while (input.eof()) {
    c = input.get();
    if (c == '^') {
      if (intoken) {
        cerr << "Error: unescaped '^': " << buf << "^" << endl;
        buf += "\\^"_u;
      } else {
        intoken = true;
        if (!buf.empty() || (buf.empty() && !spaced)) {
          u_fputc(ws, output);
        }
        buf = "^"_u;
      }
    } else if(c == '$') {
      if (intoken) {
        intoken = false;
        buf += c;
        write(buf, output);
        buf.clear();
      } else {
        cerr << "Error: stray '$'" << endl;
      }
    } else if(c == '\\') {
      c = input.get();
      buf += '\\';
      buf += c;
    } else if(!intoken && c == '[') {
      u_fputc(ws, output);
      blanktmp = readFullBlock(input, L'[', L']');
      if(keepblank) {
        write(blanktmp, output);
      }
      blanktmp.clear();
    } else {
      buf += c;
    }
  }

  // If not in space mode, make sure there's a final newline
  if (!spaced) {
    u_fputc('\n', output);
  }
}
