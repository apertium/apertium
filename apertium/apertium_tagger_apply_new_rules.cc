/*
 * Copyright (C) 2004-2006 Felipe Sánchez-Martínez
 * Copyright (C) 2006 Universitat d'Alacant
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

#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "getopt_long.h"
#include <apertium/string_utils.h>

#include <apertium/hmm.h>
#include <apertium/tagger_data_hmm.h>
#include <apertium/tsx_reader.h>
#include <apertium/string_utils.h>

using namespace Apertium;

using namespace std;

//Global vars
TaggerDataHMM tagger_data_hmm;
TTag eos; //End-of-sentence tag

void check_file(FILE *f, const string& path) {
  if (!f) {
    wcerr<<"Error: cannot open file '"<<path.c_str()<<"'\n";
    exit(EXIT_FAILURE);
  }
}

void help(char *name) {
  wcerr<<"Forbid and enforce rules are applied to the given HMM parameters\n\n";
  wcerr<<"USAGE:\n";
  wcerr<<name<<" --filein filein.prob --fileout fileout.prob --tsxfile file.tsx\n\n";

  wcerr<<"ARGUMENTS: \n"
      <<"   --filein|-i: To specify the file with the HMM parameter to process\n\n"
      <<"   --fileout|-o: To specify the file to which the HMM will be written\n\n"
      <<"   --tsxfile|-x: File containing the rules to apply\n\n"
      <<"NOTE: Parameters are read from and written to the files provided\n";
}

int main(int argc, char* argv[]) {
  string filein="";
  string fileout="";
  string filetsx="";

  int c;
  int option_index=0;

  wcerr<<"Command line: ";
  for(int i=0; i<argc; i++)
    wcerr<<argv[i]<<" ";
  wcerr<<"\n";

  while (true) {
    static struct option long_options[] =
      {
	{"filein",    required_argument, 0, 'i'},
	{"fileout",   required_argument, 0, 'o'},
	{"tsxfile",   required_argument, 0, 'x'},
	{0, 0, 0, 0}
      };

    c=getopt_long(argc, argv, "i:o:x:hv",long_options, &option_index);
    if (c==-1)
      break;
      
    switch (c) {
    case 'i':
      filein=optarg; 
      break;
    case 'o':
      fileout=optarg; 
      break;
    case 'h': 
      help(argv[0]);
      exit(EXIT_SUCCESS);
    case 'x':
      filetsx=optarg;
      break;
    case 'v':
      wcerr<<"LICENSE:\n\n"
	  <<"   Copyright (C) 2006 Felipe Sánchez Martínez\n\n"
	  <<"   This program is free software; you can redistribute it and/or\n"
	  <<"   modify it under the terms of the GNU General Public License as\n"
	  <<"   published by the Free Software Foundation; either version 2 of the\n"
	  <<"   License, or (at your option) any later version.\n"
	  <<"   This program is distributed in the hope that it will be useful, but\n"
	  <<"   WITHOUT ANY WARRANTY; without even the implied warranty of\n"
	  <<"   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU\n"
	  <<"   General Public License for more details.\n"
	  <<"\n"
	  <<"   You should have received a copy of the GNU General Public License\n"
	  <<"   along with this program; if not, see <http://www.gnu.org/licenses/>.\n";
      exit(EXIT_SUCCESS);
      break;    
    default:
      help(argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
  }

  //Now we check the command line arguments
  if (filein=="") {
    wcerr<<"Error: You did not provide an input file (.prob). Use --filein to do that\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (fileout=="") {
    wcerr<<"Error: You did not provide an output file (.prob). Use --fileout to do that\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (filetsx=="") {
    wcerr<<"Error: You did not provide a tagger definition file (.tsx). Use --filetsx to do that\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  FILE *fin, *fout;

  fin=fopen(filein.c_str(), "rb");
  check_file(fin, filein);

  wcerr<<"Reading apertium-tagger data from file '"<<filein<<"' ... "<<flush;
  tagger_data_hmm.read(fin);
  fclose(fin);
  wcerr<<"done.\n";

  wcerr<<"Reading apertium-tagger definition from file '"<<filetsx<<"' ... "<<flush;
  TSXReader treader;
  treader.read(filetsx);
  wcerr<<"done.\n";
  
  tagger_data_hmm.setForbidRules(treader.getTaggerData().getForbidRules());
  tagger_data_hmm.setEnforceRules(treader.getTaggerData().getEnforceRules());
  tagger_data_hmm.setPreferRules(treader.getTaggerData().getPreferRules());

  HMM hmm(&tagger_data_hmm);
  hmm.apply_rules();

  fout=fopen(fileout.c_str(), "wb");
  check_file(fout, fileout);
  wcerr<<"Writing apertium-tagger data to file '"<<fileout<<"' ... "<<flush;
  hmm.serialise(fout);
  fclose(fout);
  wcerr<<"done.\n";
}
