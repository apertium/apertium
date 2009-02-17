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
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <libgen.h>
#include <string>

#include <lttoolbox/lt_locale.h>
#include <apertium/unlocked_cstdio.h>
#ifdef WIN32
#if defined(__MINGW32__)
#define __MSVCRT_VERSION__  0x0800
#endif
#include <io.h>
#include <fcntl.h>
#endif
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;

void readAndWriteUntil(FILE *input, FILE *output, int const charcode)
{
  int mychar;

  while((mychar = fgetwc_unlocked(input)) != charcode)
  {
    if(feof(input))
    {
      wcerr << L"ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }
    fputwc_unlocked(mychar, output);
    if(mychar == L'\\')
    {
      mychar = fgetwc(input);
      fputwc(mychar, output);
    }
  }
}

void procWord(FILE *input, FILE *output)
{
  int mychar;
  wstring buffer = L"";

  bool buffer_mode = false;
  while((mychar = fgetwc_unlocked(input)) != L'$')
  {
    if(feof(input))
    {
      wcerr << L"ERROR: Unexpected EOF" << endl;
      exit(EXIT_FAILURE);
    }
  
    switch(mychar)
    {
    case L'<':
      if(!buffer_mode)
      {
	buffer_mode = true;
      }
      break;
    case L'#':
      if(buffer_mode)
      {
	buffer_mode = false;
      }
      break;
    }

    if(buffer_mode)
    {
      if(mychar != L'+')
      {
	buffer += static_cast<wchar_t>(mychar);
      }
      else
      {
        buffer.append(L"$ ^");
      }
    }
    else
    {
      fputwc_unlocked(mychar, output);
    }
  }
  fputws_unlocked(buffer.c_str(), output);
}

void processStream(FILE *input, FILE *output)
{
  while(true)
  {
    int mychar = fgetwc_unlocked(input);
    if(feof(input))
    {
      break;
    }
    switch(mychar)
    {
    case L'[':
      fputwc_unlocked(L'[', output);
      readAndWriteUntil(input, output, L']');
      fputwc_unlocked(L']', output);
      break;
    case L'\\':
      fputwc_unlocked(mychar, output);
      fputwc_unlocked(fgetwc_unlocked(input), output);
      break;
    case L'^':
      fputwc_unlocked(mychar, output);
      procWord(input, output);
      fputwc_unlocked(L'$', output);
      break;
    default:
      fputwc_unlocked(mychar, output);
      break;
    }
  }
}

void usage(char *progname)
{
  wcerr << L"USAGE: " << basename(progname) << L" [input_file [output_file]]" << endl;
  exit(EXIT_FAILURE);
}


int main(int argc, char *argv[])
{ 
  LtLocale::tryToSetLocale();

  if(argc > 3)
  {
    usage(argv[0]);
  }

  FILE *input, *output;
  
  if(argc == 1)
  {
    input = stdin;
    output = stdout;
  }
  else if (argc == 2)
  {
    input = fopen(argv[1], "r");
    if(!input)
    {
      usage(argv[0]);
    }
    output = stdout;
  }
  else
  {
    input = fopen(argv[1], "r");
    output = fopen(argv[2], "w");
#ifdef WIN32
    _setmode(_fileno(input), _O_U8TEXT);
    _setmode(_fileno(output), _O_U8TEXT);
#endif

  if(!input || !output)
    {
      usage(argv[0]);
    }
  }

  if(feof(input))
  {
    wcerr << L"ERROR: Can't read file '" << argv[1] << L"'" << endl;
    exit(EXIT_FAILURE);
  }

  processStream(input, output);
}
