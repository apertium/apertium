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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

/*
#include <iostream>
#include <clocale>
#include <apertium/tagger_word.h>
#include <apertium/tagger_data.h>
#include <apertium/tsx_reader.h>
*/

#include <getopt.h>
#include <apertium/utf_converter.h>
#include <apertium/morpho_stream.h>
#include <apertium/tsx_reader.h>
#include <apertium/tagger_data_hmm.h>
#include <lttoolbox/lt_locale.h>
#include <iostream>

#include <cstdlib>
#include <apertium/string_utils.h>


using namespace std;

//Global vars
TaggerDataHMM tagger_data_hmm;
bool check_ambclasses;

void check_file(FILE *f, const string& path) {
  if (!f) {
    cerr<<"Error: cannot open file '"<<path<<"'\n";
    exit(EXIT_FAILURE);
  }
}

void readwords (FILE *is, int corpus_length) {
  MorphoStream lexmorfo(is, true, &tagger_data_hmm);
  TaggerWord *word=NULL;
  int nwords=0;

  word = lexmorfo.get_next_word();
  while(word) {
    nwords++;

    cout<<UtfConverter::toUtf8(word->get_superficial_form())<<" "<<UtfConverter::toUtf8(word->get_string_tags())<<"\n";

    if (check_ambclasses) {
      int k=tagger_data_hmm.getOutput()[word->get_tags()];
    
      if ((k>=tagger_data_hmm.getM())||(k<0)) {
	cerr<<"Error: Ambiguity class number out of range: "<<k<<"\n";
	cerr<<"Word: "<<UtfConverter::toUtf8(word->get_superficial_form())<<"\n";
	cerr<<"Ambiguity class: "<<UtfConverter::toUtf8(word->get_string_tags())<<"\n";
      }
    }

    delete word;

    if ((corpus_length>0) && (nwords>=corpus_length))
      break;

    word=lexmorfo.get_next_word();
  }
  cerr<<nwords<<" were readed\n";
}


void help(char *name) {
  cerr<<"USAGE:\n";
  cerr<<name<<" {--tsxfile file.tsx | --probfile file.prob} [--clength <corpus_length>] < file.crp \n\n";

  cerr<<"ARGUMENTS: \n"
      <<"   --tsxfile|-x: Specify a tagger specification file\n"
      <<"   --probfile|-p: Specify a tagger parameter file\n"
      <<"   --clength|-l: Specify the length of the corpus to process\n";
}


int main(int argc, char* argv[]) {
  string tsxfile="";
  string probfile="";
  int corpus_length=-1;

  int c;
  int option_index=0;

  cerr<<"LOCALE: "<<setlocale(LC_ALL,"")<<"\n";

  cerr<<"Command line: ";
  for(int i=0; i<argc; i++)
    cerr<<argv[i]<<" ";
  cerr<<"\n";

  while (true) {
    static struct option long_options[] =
      {
	{"tsxfile",  required_argument, 0, 'x'},
	{"probfile", required_argument, 0, 'p'},
	{"clength",  required_argument, 0, 'l'},
	{"help",       no_argument,     0, 'h'},
	{"version",    no_argument,     0, 'v'},
	{0, 0, 0, 0}
      };

    c=getopt_long(argc, argv, "x:p:l:hv",long_options, &option_index);
    if (c==-1)
      break;
      
    switch (c) {
    case 'l': 
      corpus_length=atoi(optarg);
      if(corpus_length<=0) {
	cerr<<"Error: corpus length provided with --clength must be a positive integer\n";
	help(argv[0]);
	exit(EXIT_FAILURE);
      }
      break;
    case 'x': 
      tsxfile=optarg;
      break;
    case 'p':
      probfile=optarg;
      break;
    case 'h': 
      help(argv[0]);
      exit(EXIT_SUCCESS);
      break;
    case 'v':
      cerr<<"apertium-tagger-readwords\n";
      cerr<<"LICENSE:\n\n"
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
	  <<"   along with this program; if not, write to the Free Software\n"
	  <<"   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA\n"
	  <<"   02111-1307, USA.\n";
      exit(EXIT_SUCCESS);
      break;    
    default:
      help(argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if((tsxfile=="") && (probfile=="")) {
    cerr<<"Error: You have provided neither a tagger specification file (.tsx) nor a tagger probability file (.prob). Use --tsxfile or --probfile to provide one of them\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if((tsxfile!="") && (probfile!="")) {
    cerr<<"Error: You provided a tagger specification file and a tagger probability file. Only one of them can be provided, not both\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (tsxfile!="") {
    cerr<<"Reading tagger specification from file '"<<tsxfile<<"' ..."<<flush;
    TSXReader treader;
    treader.read(tsxfile);
    tagger_data_hmm=treader.getTaggerData();
    cerr<<"done.\n";
    check_ambclasses=false;
  }

  if (probfile!="") {
    cerr<<"Reading tagger parameters from file '"<<probfile<<"' ..."<<flush;
    FILE* fin=NULL;
    fin=fopen(probfile.c_str(), "r");
    check_file(fin, probfile);
    tagger_data_hmm.read(fin);
    cerr<<"done.\n";
    fclose(fin);
    check_ambclasses=true;
  }

  TaggerWord::setArrayTags(tagger_data_hmm.getArrayTags());

  readwords(stdin, corpus_length);
}
