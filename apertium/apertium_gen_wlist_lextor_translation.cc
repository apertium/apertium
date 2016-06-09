/*
 * Copyright (C) 2006 Universitat d'Alacant / Universidad de Alicante
 * 
 * author: Felipe Sánchez-Martínez
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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <fstream>
#include "getopt_long.h"
#include <string>

#include <lttoolbox/fst_processor.h>

#include <apertium/lextor_word.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>

using namespace Apertium;
using namespace std;


void help(char *name) {
  wcerr<<L"USAGE:\n";
  wcerr<<name<<L" --mono dic.bin --bil bildic.bin --wlist wlistfile\n\n";
  wcerr<<L"ARGUMENTS: \n"
      <<L"   --mono|-m: Specifies the monolingual lexical selection dictionary to use.\n"
      <<L"   --bil|-b: Specifies the bilingual lexical selection dictionary to use.\n"
      <<L"   --wlist|-w: Specifies the list of words to translate.\n"
      <<L"   --help|-h: Show this help\n"
      <<L"   --version|-v: Show version information\n\n"
      <<L"Write to standard output all possible translations of words found in wlistfile\n";
}

int main(int argc, char* argv[]) {
  int c;
  
  int option_index=0;
  string monodic_file="";
  string bildic_file="";
  string wlist_file="";

  while (true) {
    static struct option long_options[] =
      {
	{"mono",    required_argument, 0, 'm'},
	{"bil",     required_argument, 0, 'b'},
	{"wlist",   required_argument, 0, 'w'},
	{"help",        no_argument,   0, 'h'},
	{"version",     no_argument,   0, 'v'},
	{0, 0, 0, 0}
      };

    c=getopt_long(argc, argv, "m:b:w:hv",long_options, &option_index);
    if (c==-1)
      break;
      
    switch (c) {
    case 'm':
      monodic_file=optarg;
      break;
    case 'b':
      bildic_file=optarg;
      break;
    case 'w':
      wlist_file=optarg;
      break;
    case 'h': 
      help(argv[0]);
      exit(EXIT_SUCCESS);
      break;
    case 'v':
      wcerr<<L"APERTIUM"<<L"\n"; //"APERTIUM" era PACKAGE_STRING
      wcerr<<L"LICENSE:\n\n"
	  <<L"   Copyright (C) 2006 Universitat d'Alacant / Universidad de Alicante\n\n"
	  <<L"   This program is free software; you can redistribute it and/or\n"
	  <<L"   modify it under the terms of the GNU General Public License as\n"
	  <<L"   published by the Free Software Foundation; either version 2 of the\n"
	  <<L"   License, or (at your option) any later version.\n"
	  <<L"   This program is distributed in the hope that it will be useful, but\n"
	  <<L"   WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	  <<L"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
	  <<L"   General Public License for more details.\n"
	  <<L"\n"
	  <<L"   You should have received a copy of the GNU General Public License\n"
	  <<L"   along with this program; if not, see <http://www.gnu.org/licenses/>.\n";
      exit(EXIT_SUCCESS);
      break;    
    default:
      help(argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if(monodic_file=="") {
    wcerr<<L"Error: no monolingual dictionary file was given\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (bildic_file=="") {
    wcerr<<L"Error: no bilingual dictionary file was given\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (wlist_file=="") {
    wcerr<<L"Error: no word list file was given\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  wifstream fwlist;
  FILE *fmonodic, *fbildic;

  fmonodic=fopen(monodic_file.c_str(), "rb");
  if (!fmonodic) {
    wcerr<<L"Error: Cannot open file '"
         <<UtfConverter::fromUtf8(monodic_file)<<L"'\n";
    exit(EXIT_FAILURE);
  }

  fbildic=fopen(bildic_file.c_str(), "rb");
  if (!fbildic) {
    wcerr<<L"Error: Cannot open file '"
         <<UtfConverter::fromUtf8(bildic_file)<<L"'\n";
    exit(EXIT_FAILURE);
  }

  fwlist.open(wlist_file.c_str(), ios::in);
  if (fwlist.fail()) {
    wcerr<<L"Error: Cannot open file '"
         <<UtfConverter::fromUtf8(wlist_file)<<L"'\n";
    exit(EXIT_FAILURE);
  }

  FSTProcessor fstp_monodic, fstp_bildic;

  fstp_monodic.load(fmonodic);
  fstp_monodic.initBiltrans();
  fclose(fmonodic);

  fstp_bildic.load(fbildic);
  fstp_bildic.initBiltrans();
  fclose(fbildic);

  wstring strword=L"";
  while (!fwlist.eof()) {
    getline(fwlist, strword);
    if (!fwlist.eof()) {
      LexTorWord word(strword, &fstp_monodic);
      wcerr<<strword<<L" =>\n";
      for (int i=0; i<word.n_lexical_choices(); i++) {
	wcerr<<L"\t"<<word.translate(fstp_bildic,i)<<L" ("<<word.get_lexical_choice(i)<<L")\n";
	wcout<<word.translate(fstp_bildic,i)<<L"\n";
      }
      if (word.n_lexical_choices()<=1) {
        wcerr<<L"Warning: word '"<<strword<<L"' is supossed to be polysemous, but it has "<<word.n_lexical_choices()<<L" different translations\n";
      }
    }
  }

  fwlist.close();
}
