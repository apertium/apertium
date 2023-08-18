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

#include <fstream>
#include <iostream>
#include <string>
#include <sys/types.h>
#include <unistd.h>
#include "getopt_long.h"

#include <apertium/hmm.h>
#include <apertium/tagger_data_hmm.h>
#include <apertium/tsx_reader.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/lt_locale.h>
#include <i18n.h>
#include <unicode/ustream.h>

using namespace Apertium;

using namespace std;

//Global vars
TaggerDataHMM tagger_data_hmm;
TTag eos; //End-of-sentence tag

void check_file(FILE *f, const string& path) {
  if (!f) {
		I18n(APER_I18N_DATA, "apertium").error("APER1000", {"file_name"}, {path.c_str()}, true);
  }
}

void help(char *name) {
	I18n i18n {APER_I18N_DATA, "apertium"};
  cerr<< i18n.format("tagger_apply_new_rules_desc") << "\n\n";
  cerr<< i18n.format("usage") << ":\n";
  cerr<<name<<" --filein filein.prob --fileout fileout.prob --tsxfile file.tsx\n\n";

  cerr<< i18n.format("arguments") << ": \n"
      <<"   --filein|-i: " <<  i18n.format("hmm_filein_desc") << "\n\n"
      <<"   --fileout|-o: " <<  i18n.format("hmm_fileout_desc") << "\n\n"
      <<"   --tsxfile|-x: " <<  i18n.format("hmm_tsxfile_desc") << "\n\n"
      << i18n.format("tagger_apply_new_rules_note") << "\n";
}

int main(int argc, char* argv[]) {
  LtLocale::tryToSetLocale();
  string filein="";
  string fileout="";
  string filetsx="";

  int c;
  int option_index=0;

  cerr<<"Command line: ";
  for(int i=0; i<argc; i++)
    cerr<<argv[i]<<" ";
  cerr<<"\n";

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

  //Now we check the command line arguments
  if (filein=="") {
    help(argv[0]);
		I18n(APER_I18N_DATA, "apertium").error("APER1022", {}, {}, true);
  }

  if (fileout=="") {
    help(argv[0]);
		I18n(APER_I18N_DATA, "apertium").error("APER1023", {}, {}, true);
  }

  if (filetsx=="") {
    help(argv[0]);
		I18n(APER_I18N_DATA, "apertium").error("APER1024", {}, {}, true);
  }

  FILE *fin, *fout;

  fin=fopen(filein.c_str(), "rb");
  check_file(fin, filein);

	I18n i18n {APER_I18N_DATA, "apertium"};
  cerr << i18n.format("reading_from_file", {"file_name"}, {filein.c_str()}) << flush;
  tagger_data_hmm.read(fin);
  fclose(fin);
  cerr <<i18n.format("done") << "\n";

  cerr << i18n.format("reading_from_file", {"file_name"}, {filetsx.c_str()}) << flush;
  TSXReader treader;
  treader.read(filetsx);
  cerr <<i18n.format("done") << "\n";

  tagger_data_hmm.setForbidRules(treader.getTaggerData().getForbidRules());
  tagger_data_hmm.setEnforceRules(treader.getTaggerData().getEnforceRules());
  tagger_data_hmm.setPreferRules(treader.getTaggerData().getPreferRules());

  HMM hmm(&tagger_data_hmm);
  hmm.apply_rules();

  fout=fopen(fileout.c_str(), "wb");
  check_file(fout, fileout);
  cerr << i18n.format("writing_to_file", {"file_name"}, {fileout.c_str()}) << flush;
  hmm.serialise(fout);
  fclose(fout);
  cerr <<i18n.format("done") << "\n";
}
