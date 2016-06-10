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

#include <lttoolbox/fst_processor.h>

#include <apertium/lextor.h>
#include <apertium/lextor_word.h>
#include <apertium/lextor_data.h>
#include <apertium/lextor_eval.h>
#include <clocale>
#include <apertium/string_utils.h>

using namespace Apertium;
#define MODE_LEXTOR 1
#define MODE_LEXTORTL 2

using namespace std;


void help(char *name) {
  wcerr<<"USAGE:\n";
  wcerr<<name<<" --angle ang --reference reftext --parameters|-p model dic left right [--debug]\n\n";
  wcerr<<"ARGUMENTS: \n"
      <<"  --angle|-a: To specify the angle threshold to use............\n"
      <<"   --reference|-r: To specify the reference corpus used for evaluation (one word per\n"
      <<"               line with the correct translation sense for those words with more than one)\n"
      <<"   --parameters|-p: to specify the parameters used for the lexical selection task:\n"
      <<"   Required parameters:\n"
      <<"      model: file containing the model to be used for the lexical selection\n"
      <<"      dic: lexical-selection dictionary (binary format)\n"
      <<"      left: left-side context to take into account (number of words)\n"
      <<"      right: right-side context to take into account (number of words)\n"
      <<"   --weightexp|-w: Specify a weight value to change the influence of surrounding words while\n"
      <<"     performing the lexical selection. It must be positive.\n"
      <<"   --help|-h: Show this help\n"
      <<"   --version|-v: Show version information\n\n";

  wcerr<<"NOTE: It reads from the standard input the corpus to work with. That corpus must be\n"
      <<"      in the intermediate format used by Apertium.\n";
}

int main(int argc, char* argv[]) {
  int c;
  int option_index=0;

  int mode=MODE_LEXTOR;

  string model_file="";
  string dic_file="";
  int nwords_left=-1;
  int nwords_right=-1;

  //string in_file;
  string ref_file;

  double weight_exponent=0.0;

  LexTor::angleth=0.0;


  //For mode LEXTORTL
  string stopwords_file;
  string words_file;
  string bildic_file;

  //wcerr<<"LOCALE: "<<setlocale(LC_ALL,"")<<"\n";
  wcerr<<"Command line: ";
  for(int i=0; i<argc; i++)
    wcerr<<argv[i]<<" ";
  wcerr<<"\n";

  while (true) {
    static struct option long_options[] =
      {
	//{"input",       required_argument, 0, 'i'},
	{"reference",   required_argument, 0, 'r'},
	{"parameters",  required_argument, 0, 'p'},
        {"weightexp",   required_argument, 0, 'w'},
	{"angle",       required_argument, 0, 'a'},
	{"lextortl",    required_argument, 0, 'e'},
	{"debug",       no_argument,    0, 'd'},
	{"help",        no_argument,    0, 'h'},
	{"version",     no_argument,    0, 'v'},
	{0, 0, 0, 0}
      };

    c=getopt_long(argc, argv, "r:p:w:a:e:dhv",long_options, &option_index);
    if (c==-1)
      break;
      
    switch (c) {
      //case 'i':
      //in_file=optarg;
      //break;
    case 'r':
      ref_file=optarg;
      break;
    case 'p':
      mode=MODE_LEXTOR;
      model_file=optarg;
      dic_file=argv[optind++];
      nwords_left=atoi(argv[optind++]);
      nwords_right=atoi(argv[optind++]);
      break;
    case 'e':
      mode=MODE_LEXTORTL;
      stopwords_file=optarg;
      words_file=argv[optind++];
      model_file=argv[optind++];
      dic_file=argv[optind++];
      bildic_file=argv[optind++];
      nwords_left=atoi(argv[optind++]);
      nwords_right=atoi(argv[optind++]);
      break;
    case 'w':
      weight_exponent=atof(optarg);
      break;
    case 'a':
      LexTor::angleth=atof(optarg);
      break;
    case 'd':
      LexTor::debug=true;
      break;
    case 'h': 
      help(argv[0]);
      exit(EXIT_SUCCESS);
      break;
    case 'v':
      wcerr<<"APERTIUM"<<"\n"; //"APERTIUM" era PACKAGE_STRING
      wcerr<<"LICENSE:\n\n"
	  <<"   Copyright (C) 2006 Universitat d'Alacant / Universidad de Alicante\n\n"
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

  wcerr<<"TH ANGLE: "<<LexTor::angleth<<"\n";

  if (ref_file=="") {
    wcerr<<"Error: No reference corpus was given\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }
  ifstream fref;

  fref.open(ref_file.c_str(), ios::in);
  if (fref.fail()) {
    wcerr<<"Error: Cannot open file '"<<ref_file<<"'\n";
    exit(EXIT_FAILURE);
  }

  if (mode==MODE_LEXTOR) {
    if (nwords_left<0) {
      wcerr<<"Error: no left-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_right<0) {
      wcerr<<"Error: no right-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    //if (in_file=="") {
    //  wcerr<<"Error: No input corpus was given\n";
    //  help(argv[0]);
    //  exit(EXIT_FAILURE);
    //}
 
    if (model_file=="") {
      wcerr<<"Error: No model file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (dic_file=="") {
      wcerr<<"Error: No dictionary file (bin format) was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    if (weight_exponent<0) {
      wcerr<<"Error: the weight exponent provided is less than zero. It must be positive\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    ifstream fin, fmodel;

    //fin.open(in_file.c_str(), ios::in);
    //if (fin.fail()) {
    //  wcerr<<"Error: Cannot open file '"<<in_file<<"'\n";
    //  exit(EXIT_FAILURE);
    //}

    fmodel.open(model_file.c_str(), ios::in);
    if(fmodel.fail()) {
      wcerr<<"Error: Cannot open file '"<<model_file<<"'\n";
      exit(EXIT_FAILURE);
    }

    FILE *fdic=NULL;
    fdic=fopen(dic_file.c_str(), "rb");
    if (!fdic) {
      wcerr<<"Error: Cannot open file '"<<dic_file<<"'\n";
      exit(EXIT_FAILURE);
    }
    FSTProcessor fstp;
    fstp.load(fdic);
    fstp.initBiltrans();
    fclose(fdic);


    //When reading from the input stream '*all* characters must be
    //processed, including ' ','\n', .....
    cin.unsetf(ios::skipws);


    LexTorData lextor_model;
    lextor_model.read(fmodel);
    fmodel.close();

    LexTor lexical_selector;
    lexical_selector.set_lextor_data(&lextor_model);

    LexTorEval lteval(&fref);
    lexical_selector.lexical_selector(cin, fstp, nwords_left, nwords_right, weight_exponent, &lteval);
    lteval.print_evaluation();

    fref.close();
  }

  else if (mode==MODE_LEXTORTL) {
    if(stopwords_file=="") {
      wcerr<<"Error: no stopwords file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(words_file=="") {
      wcerr<<"Error: no words file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(model_file=="") {
      wcerr<<"Error: no target-language model file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (dic_file=="") {
      wcerr<<"Error: No lexical-selection dictionary was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (bildic_file=="") {
      wcerr<<"Error: No bilingual dictionary was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_left<0) {
      wcerr<<"Error: no left-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_right<0) {
      wcerr<<"Error: no right-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    ifstream fstopwords, fwords, fmodel;
    FILE *fdic=NULL, *fbildic=NULL;

    fstopwords.open(stopwords_file.c_str(), ios::in);
    if (fstopwords.fail()) {
      wcerr<<"Error: Cannot open file '"<<stopwords_file<<"'\n";
      exit(EXIT_FAILURE);
    }

    fwords.open(words_file.c_str(), ios::in);
    if (fwords.fail()) {
      wcerr<<"Error: Cannot open file '"<<words_file<<"'\n";
      exit(EXIT_FAILURE);
    }

    fmodel.open(model_file.c_str(), ios::in);
    if(fmodel.fail()) {
      wcerr<<"Error: Cannot open file '"<<model_file<<"'\n";
      exit(EXIT_FAILURE);
    }

    fdic=fopen(dic_file.c_str(), "rb");
    if(!fdic) {
      wcerr<<"Error: Cannot open file '"<<dic_file<<"'\n";
      exit(EXIT_FAILURE);
    }

    fbildic=fopen(bildic_file.c_str(), "rb");
    if(!fbildic) {
      wcerr<<"Error: Cannot open file '"<<bildic_file<<"'\n";
      exit(EXIT_FAILURE);
    }

    LexTorData lextor_data;

    lextor_data.read_stopwords(fstopwords);
    fstopwords.close();

    lextor_data.read_words(fwords);
    fwords.close();

    LexTor lexical_selector;
    lexical_selector.set_lextor_data(&lextor_data);

    LexTorData tlmodel;
    tlmodel.read(fmodel);
    fmodel.close();

    FSTProcessor fstpdic;
    fstpdic.load(fdic);
    fstpdic.initBiltrans();
    fclose(fdic);

    FSTProcessor fstpbildic;
    fstpbildic.load(fbildic);
    fstpbildic.initBiltrans();
    fclose(fbildic);


    lextor_data.read_lexical_choices(fstpdic);

    //Whe reading from the input stream '*all* characters must be
    //processed, including ' ','\n', .....
    cin.unsetf(ios::skipws);


    lexical_selector.set_tlmodel(&tlmodel);
    lexical_selector.set_bildic(&fstpbildic);


    LexTorEval lteval(&fref);
    lexical_selector.lexical_selector(cin, fstpdic, nwords_left, nwords_right, weight_exponent, &lteval);
    lteval.print_evaluation();

    fref.close();
  } 
}
