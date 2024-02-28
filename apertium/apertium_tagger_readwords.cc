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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include "getopt_long.h"
#include <apertium/file_morpho_stream.h>
#include <apertium/tsx_reader.h>
#include <apertium/tagger_data_hmm.h>
#include <lttoolbox/lt_locale.h>
#include <iostream>

#include <cstdlib>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/i18n.h>
#include <unicode/ustream.h>


using namespace std;

//Global vars
TaggerDataHMM tagger_data_hmm;
bool check_ambclasses;

void check_file(FILE *f, const string& path) {
  if (!f) {
		I18n(APR_I18N_DATA, "apertium").error("APR80000", {"file_name"}, {path.c_str()}, true);
  }
}

void readwords (int corpus_length) {
  FileMorphoStream lexmorfo(NULL, true, &tagger_data_hmm);
  TaggerWord *word=NULL;
  int nwords=0;

  word = lexmorfo.get_next_word();
  while(word) {
    nwords++;

    cout << word->get_superficial_form() << " " << word->get_string_tags() << "\n";

    if (check_ambclasses) {
      int k=tagger_data_hmm.getOutput()[word->get_tags()];

      if ((k>=tagger_data_hmm.getM())||(k<0)) {
		    I18n(APR_I18N_DATA, "apertium").error("APR80250", {"number", "word", "class"},
                                                           {k, icu::UnicodeString(word->get_superficial_form().data()),
                                                           icu::UnicodeString(word->get_string_tags().data())}, false);
      }
    }

    delete word;

    if ((corpus_length>0) && (nwords>=corpus_length))
      break;

    word=lexmorfo.get_next_word();
  }
  cerr << I18n(APR_I18N_DATA, "apertium").format("readed_words", {"number_of_words"}, {nwords}) << "\n";
}


void help(char *name) {
  I18n i18n {APR_I18N_DATA, "apertium"};
  cerr<< i18n.format("usage") << ":\n";
  cerr<<name<<" {--tsxfile file.tsx | --probfile file.prob} [--clength <corpus_length>] < file.crp \n\n";

  cerr<< i18n.format("arguments") << ": \n"
      <<"   --tsxfile|-x: " <<  i18n.format("readwords_tsxfile") << "Specify a tagger specification file\n"
      <<"   --probfile|-p: " <<  i18n.format("probfile_desc") << "\n"
      <<"   --clength|-l: " <<  i18n.format("clength_desc") << "\n";
}


int main(int argc, char* argv[]) {
  LtLocale::tryToSetLocale();
  string tsxfile="";
  string probfile="";
  int corpus_length=-1;

  int c;
  int option_index=0;

  I18n i18n {APR_I18N_DATA, "apertium"};
  cerr << i18n.format("locale") << ": " << setlocale(LC_ALL,"") << "\n";

  cerr << i18n.format("command_line") << ": ";
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
	      help(argv[0]);
        I18n(APR_I18N_DATA, "apertium").error("APR80260", true);
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
	  <<"   along with this program; if not, see <https://www.gnu.org/licenses/>.\n";
      exit(EXIT_SUCCESS);
      break;
    default:
      help(argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if((tsxfile=="") && (probfile=="")) {
    help(argv[0]);
    I18n(APR_I18N_DATA, "apertium").error("APR80270", true);
  }

  if((tsxfile!="") && (probfile!="")) {
    help(argv[0]);
    I18n(APR_I18N_DATA, "apertium").error("APR80280", true);
  }

  if (tsxfile!="") {
    cerr << i18n.format("reading_from_file", {"file_name"}, {tsxfile.c_str()}) << flush;
    TSXReader treader;
    treader.read(tsxfile);
    tagger_data_hmm=treader.getTaggerData();
    cerr <<i18n.format("done") << "\n";
    check_ambclasses=false;
  }

  if (probfile!="") {
    cerr << i18n.format("reading_from_file", {"file_name"}, {tsxfile.c_str()}) << flush;
    FILE* fin=NULL;
    fin=fopen(probfile.c_str(), "r");
    check_file(fin, probfile);
    tagger_data_hmm.read(fin);
    cerr <<i18n.format("done") << "\n";
    fclose(fin);
    check_ambclasses=true;
  }

  TaggerWord::setArrayTags(tagger_data_hmm.getArrayTags());

  readwords(corpus_length);
}
