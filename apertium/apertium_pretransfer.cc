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
#include <getopt.h>

#include <lttoolbox/lt_locale.h>
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;

bool compound_sep = false;

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

void procWord(FILE *input, FILE *output, bool surface_forms)
{
  int mychar;
  wstring buffer = L"";

  bool buffer_mode = false;
  bool in_tag = false;
  bool queuing = false;

  if(surface_forms)
  {
    while((mychar = fgetwc_unlocked(input)) != L'/') ;
  } 

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
      in_tag = true;
      if(!buffer_mode)
      {
        buffer_mode = true;
      }
      break;
      
    case L'>':
      in_tag = false;
      break;
      
    case L'#':
      if(buffer_mode)
      {
        buffer_mode = false;
        queuing = true;
      }
      break;
    }

    if(buffer_mode)
    { 
      if((mychar != L'+' || (mychar == L'+' && in_tag == true)) && 
         (mychar != L'~' || (mychar == L'~' && in_tag == true)))
      {
        buffer += static_cast<wchar_t>(mychar);
      }
      else if(in_tag == false && mychar == L'+')
      {
        buffer.append(L"$ ^");
      }
      else if(in_tag == false && mychar == L'~' and compound_sep == true)
      {
        buffer.append(L"$^");
      }
    }
    else
    {
      if(mychar == L'+' && queuing == true)  
      {
        buffer.append(L"$ ^");
        buffer_mode = true;
      }
      else 
      {
        fputwc_unlocked(mychar, output);
      }
    }

  }
  fputws_unlocked(buffer.c_str(), output);
}

void processStream(FILE *input, FILE *output, bool null_flush, bool surface_forms)
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
        procWord(input, output, surface_forms);
        fputwc_unlocked(L'$', output);
        break;
      
      case L'\0':
        fputwc_unlocked(mychar, output);
        
        if(null_flush)
        {
          fflush(output);
        }
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
  bool null_flush = false;
  bool surface_forms = false;
  
#if HAVE_GETOPT_LONG
  int option_index=0;
#endif

  while (true) {
#if HAVE_GETOPT_LONG
    static struct option long_options[] =
    {
      {"null-flush", no_argument, 0, 'z'},
      {"no-surface-forms", no_argument, 0, 'n'},
      {"compounds", no_argument, 0, 'e'},
      {"help", no_argument, 0, 'h'},
      {0, 0, 0, 0}
    };

    int c=getopt_long(argc, argv, "enzh", long_options, &option_index);
#else
    int c=getopt(argc, argv, "enzh");
#endif
    if (c==-1)
      break;
      
    switch (c)
    {
      case 'z':
        null_flush = true;
        break;

      case 'e':
        compound_sep = true;
        break;
     
      case 'n':
        surface_forms = true;
        break;
       
      case 'h':
      default:
        usage(argv[0]);
        break;
    }
  }

  if((argc-optind+1) > 3)
  {
    usage(argv[0]);
  }

  FILE *input, *output;
  
  if((argc-optind+1) == 1)
  {
    input = stdin;
    output = stdout;
  }
  else if ((argc-optind+1) == 2)
  {
    input = fopen(argv[argc-1], "r");
    if(!input)
    {
      usage(argv[0]);
    }
    output = stdout;
  }
  else
  {
    input = fopen(argv[argc-2], "r");
    output = fopen(argv[argc-1], "w");

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

#ifdef _MSC_VER
    _setmode(_fileno(input), _O_U8TEXT);
    _setmode(_fileno(output), _O_U8TEXT);
#endif

  processStream(input, output, null_flush, surface_forms);
}
