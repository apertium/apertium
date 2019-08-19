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

using namespace std;

#ifndef fputwc_unlocked
#define fputwc_unlocked fputwc
#endif

#ifndef fputws_unlocked
#define fputws_unlocked fputws
#endif

#ifndef fgetwc_unlocked
#define fgetwc_unlocked getwc
#endif


void
tryToSetLocale()
{
#if !defined(__CYGWIN__) && !defined (__MINGW32__)
  if(setlocale(LC_CTYPE, "") != NULL)
  {
    return;
  }

  cerr << "Warning: unsupported locale, fallback to \"C\"" << endl;

  setlocale(LC_ALL, "C");
#endif
#ifdef __CYGWIN__
  setlocale(LC_ALL, "C.UTF-8");
#endif
#ifdef __MINGW32__
  SetConsoleInputCP(65001);
  SetConsoleOutputCP(65001);
#endif
}

wstring
readFullBlock(FILE *input, wchar_t const delim1, wchar_t const delim2)
{
  wstring result = L"";
  result += delim1;
  wchar_t c = delim1;

  while(!feof(input) && c != delim2)
  {
    c = static_cast<wchar_t>(fgetwc_unlocked(input));
    result += c;
    if(c != L'\\')
    {
      continue;
    }
    else
    {
      result += L'\\';
      c = static_cast<wchar_t>(fgetwc(input));
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
  wstring buf = L"";
  wstring blanktmp = L"";
  bool keepblank = false;

  bool spaced = true;
  bool intoken = false;

  wchar_t ws = L' ';

  for(int i=1; i<argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      spaced = false;
      ws = L'\n';
    }
    else if (strcmp(argv[i], "-b") == 0) {
      keepblank = true;
    }
  }

  tryToSetLocale();

  wint_t c;
  while ((c = fgetwc(stdin)) != WEOF)
  {
    if (c == (wint_t) '^')
    {
      if (intoken)
      {
        wcerr << L"Error: unescaped '^': " << buf << "^" << endl;
        buf += L"\\^";
      }
      else
      {
        intoken = true;
        if (buf != L"" || ((buf == L"") && !spaced))
        {
          fputwc_unlocked(ws, stdout);
        }
        buf = L"^";
      }
    }
    else if(c == (wint_t) '$')
    {
      if (intoken)
      {
        intoken = false;
        buf += c;
        fputws_unlocked(buf.c_str(), stdout);
        buf = L"";
      }
      else
      {
        cerr << "Error: stray '$'" << endl;
      }
    }
    else if(c == (wint_t) '\\')
    {
      c = fgetwc_unlocked(stdin);
      buf += L'\\';
      buf += c;
    }
    else if(!intoken && c == (wint_t) '[')
    {
      fputwc_unlocked(ws, stdout);
      blanktmp = readFullBlock(stdin, L'[', L']');
      if(keepblank) {
        fputws_unlocked(blanktmp.c_str(), stdout);
      }
      blanktmp = L"";
    }
    else
    {
      buf += c;
    }
  }

  // If not in space mode, make sure there's a final newline
  if (!spaced)
  {
    fputwc_unlocked(L'\n', stdout);
  }

  return 0;
}
