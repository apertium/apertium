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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#include <iostream>
#include <fstream>
#include <getopt.h>

#include <lttoolbox/fst_processor.h>

#include <apertium/lextor.h>
#include <apertium/lextor_word.h>
#include <apertium/lextor_data.h>
#include <apertium/utf_converter.h>
#include <clocale>
#include <cstdlib>
#include <apertium/string_utils.h>

using namespace Apertium;

#define MODE_TRAINWRD 0
#define MODE_TRAINLCH 1
#define MODE_LEXTOR 2
#define MODE_LEXTORTL 3

using namespace std;


void help(char *name) {
  cerr<<"USAGE:\n";
  cerr<<name<<" --trainwrd stopwords words n left right corpus model [--weightexp w]\nOR\n";
  cerr<<name<<" --trainlch stopwords lexchoices n left right corpus wordmodel dic bildic model [--weightexp w]\nOR\n";
  cerr<<name<<" --lextor model dic left right [--debug] [--weightexp w]\n\n";
  //cerr<<name<<" --lextortl stopwords words tlmodel dic bildic left right [--debug] [--weightexp w]\n\n"; 
  cerr<<"ARGUMENTS: \n"
      <<"   --trainwrd|-t: Train word co-occurrences model.\n"
      <<"   Required parameters:\n"
      <<"      stopwords: file containing a list of stop words. Stop words are ignored\n"
      <<"      words: file containing a list of words. For each word a co-occurrence model is built\n"
      <<"      n: number of words per co-occurrence model (for each model, the n most frequent words)\n"
      <<"      left: left-side context to take into account (number of words)\n"
      <<"      right: right-side context to take into account (number of words)\n"
      <<"      corpus: file containing the training corpus\n"
      <<"      model: output file on which the co-occurrence models are saved\n\n"

      <<"   --trainlch|-r: Train lexical choices co-occurrences model using a target-language co-occurrence model.\n"
      <<"   Required parameters:\n"
      <<"      stopwords: file containing a list of stop words. Stop words are ignored\n"
      <<"      lexchoices: file containing a list of lexical choices. For each lexical choice a co-occurrence model is built\n"
      <<"      n: number of words per co-occurrence model (for each model, the n most frequent words)\n"
      <<"      left: left-side context to take into account (number of words)\n"
      <<"      right: right-side context to take into account (number of words)\n"
      <<"      corpus: file containing the training corpus\n"
      <<"      wordmodel: target-language word co-occurrence model (previously trained by means of the --trainwrd option)\n"
      <<"      dic: lexical-selection dictionary (binary format)\n"
      <<"      bildic: bilingual dictionary (binary format)\n"
      <<"      model: output file on which the co-occurrence models are saved\n\n"

      <<"   --lextor|-l: Perform the lexical selection on the input stream.\n"
      <<"   Required parameters:\n"
      <<"      model: file containing the model to be used for the lexical selection\n"
      <<"      dic: lexical-selection dictionary (binary format)\n"
      <<"      left: left-side context to take into account (number of words)\n"
      <<"      right: right-side context to take into account (number of words)\n\n"

    //      <<"   --lextortl|-e: Perform the lexical selection on the input stream by using a tl model.\n"
    //      <<"   Required parameters:\n"
    //      <<"      stopwords: file containing a list of stop words in the source language. Stop words are ignored\n"
    //      <<"      words: file containing the list of polysemous words in the source language\n"
    //      <<"      tlmodel: file containing the target-language model to be used for the lexical selection\n"
    //      <<"      dic: lexical-selection dictionary (binary format)\n"
    //      <<"      bildic: bilingual dictionary (binary format)\n"
    //      <<"      left: left-side context to take into account (number of words)\n"
    //      <<"      right: right-side context to take into account (number of words)\n\n"

      <<"   --weightexp|-w: Specify a weight value to change the influence of surrounding words while training or\n"
      <<"     performing the lexica selection. It must be positive.\n\n"

      <<"   --debug|-d: Show debug information while operating\n"
      <<"   --help|-h: Show this help\n"
      <<"   --version|-v: Show version information\n\n";
  cerr<<"Reads from standard input and writes to standard output\n";
}

int main(int argc, char* argv[]) {
  int c;
#if HAVE_GETOPT_LONG
  int option_index=0;
#endif
  int mode=-1;

  //Parameters for the "trainwrd" or the "trainlch" mode option
  string stopwords_file="";
  string words_file="";
  string corpus_file="";
  int nwords_model=0;
  int nwords_left=-1;
  int nwords_right=-1;

  string model_file="";

  string lexchoices_file="";
  string wordmodel_file="";
  string bildic_file="";

  //Parameters for the "lextor" option
  string dic_file="";

  double weight_exponent=0.0;

  LexTor::debug=false;

  //cerr<<"LOCALE: "<<setlocale(LC_ALL,"")<<"\n";

  while (true) {
#if HAVE_GETOPT_LONG
    static struct option long_options[] =
      {
	{"trainwrd",  required_argument, 0, 't'},
	{"trainlch",  required_argument, 0, 'r'},
	{"lextor",    required_argument, 0, 'l'},
	//	{"lextortl",  required_argument, 0, 'e'},
        {"weightexp", required_argument, 0, 'w'},
	{"debug",        no_argument,    0, 'd'},
	{"help",         no_argument,    0, 'h'},
	{"version",      no_argument,    0, 'v'},
	{0, 0, 0, 0}
      };

    c=getopt_long(argc, argv, "t:r:l:e:w:dhv",long_options, &option_index);
#else
    c=getopt(argc, argv, "t:r:l:e:w:dhv");
#endif
    if (c==-1)
      break;
      
    switch (c) {
    case 't':
      mode=MODE_TRAINWRD;
      stopwords_file=optarg;
      words_file=argv[optind++];
      nwords_model=atoi(argv[optind++]);
      nwords_left=atoi(argv[optind++]);
      nwords_right=atoi(argv[optind++]);
      corpus_file=argv[optind++];
      model_file=argv[optind++];
      break;
    case 'r':
      //--trainlch stopwords lexchoices n left right corpus wordmodel dic bildic model
      mode=MODE_TRAINLCH;
      stopwords_file=optarg;
      lexchoices_file=argv[optind++];
      nwords_model=atoi(argv[optind++]);
      nwords_left=atoi(argv[optind++]);
      nwords_right=atoi(argv[optind++]);
      corpus_file=argv[optind++];
      wordmodel_file=argv[optind++];
      dic_file=argv[optind++];
      bildic_file=argv[optind++];
      model_file=argv[optind++];
      break;
    case 'l':
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
    case 'd':
      LexTor::debug=true;
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
	  <<L"   along with this program; if not, write to the Free Software\n"
	  <<L"   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA\n"
	  <<L"   02111-1307, USA.\n";
      exit(EXIT_SUCCESS);
      break;    
    default:
      help(argv[0]);
      exit(EXIT_FAILURE);
      break;
    }
  }

  if (weight_exponent<0) {
    wcerr<<L"Error: the weight exponent provided is less than zero. It must be positive\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  //When reading from the input stream '*all* characters must be
  //processed, including ' ','\n', .....
  wcin.unsetf(ios::skipws);

  if (mode==MODE_TRAINWRD) {
    if(stopwords_file=="") {
      wcerr<<L"Error: no stopwords file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (words_file=="") {
      wcerr<<L"Error: no words file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_model==0) {
      wcerr<<L"Error: the number of word per co-occurrence model must be greater than 0\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_left<0) {
      wcerr<<L"Error: no left-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_right<0) {
      wcerr<<L"Error: no right-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (corpus_file=="") {
      wcerr<<L"Error: No training corpus file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (model_file=="") {
      wcerr<<L"Error: No output file to save the co-occurrence models was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    wifstream fstopwords, fwords, fcorpus;

    fstopwords.open(stopwords_file.c_str(), ios::in);
    if (fstopwords.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(stopwords_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fwords.open(words_file.c_str(), ios::in);
    if (fwords.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(words_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fcorpus.open(corpus_file.c_str(), ios::in);
    if(fcorpus.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(corpus_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    FILE *fmodel = fopen(model_file.c_str(), "wb");
    if(!fmodel)
    {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(model_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }
    
    LexTorData lextor_data;

    lextor_data.read_stopwords(fstopwords);
    lextor_data.read_words(fwords);
    lextor_data.set_nwords_per_set(nwords_model);

    fstopwords.close();
    fwords.close();

    LexTor lexical_selector;
    lexical_selector.set_lextor_data(&lextor_data);

    //Whe reading from the input corpus '*all* characters must be
    //processed, including ' ','\n', .....
    fcorpus.unsetf(ios::skipws);

    //Train
    lexical_selector.trainwrd(fcorpus, nwords_left, nwords_right, weight_exponent);
    fcorpus.close();

    //Write parameters
    lextor_data.write(fmodel);
    fclose(fmodel);
  } 

  else if (mode==MODE_TRAINLCH) {
    if(stopwords_file=="") {
      wcerr<<L"Error: no stopwords file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (lexchoices_file=="") {
      wcerr<<L"Error: no lexical choices file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_model==0) {
      wcerr<<L"Error: the number of word per co-occurrence model must be greater than 0\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_left<0) {
      wcerr<<L"Error: no left-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_right<0) {
      wcerr<<L"Error: no rigth-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (corpus_file=="") {
      wcerr<<L"Error: No training corpus file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(wordmodel_file=="") {
      wcerr<<L"Error: No target-language word co-occurrence model was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (dic_file=="") {
      wcerr<<L"Error: No lexical-selection dictionary was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (bildic_file=="") {
      cerr<<"Error: No bilingual dictionary was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (model_file=="") {
      wcerr<<L"Error: No output file to save the co-occurrence models was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    wifstream fstopwords, flexchoices, fcorpus;
    FILE *fdic=NULL, *fbildic=NULL, *fwordmodel=NULL;

    fstopwords.open(stopwords_file.c_str(), ios::in);
    if (fstopwords.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(stopwords_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    flexchoices.open(lexchoices_file.c_str(), ios::in);
    if (flexchoices.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(lexchoices_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fcorpus.open(corpus_file.c_str(), ios::in);
    if(fcorpus.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(corpus_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fwordmodel = fopen(wordmodel_file.c_str(), "rb");
    if(!fwordmodel) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(wordmodel_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fdic=fopen(dic_file.c_str(), "rb");
    if(!fdic) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(dic_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fbildic=fopen(bildic_file.c_str(), "rb");
    if(!fbildic) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(bildic_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    FILE *fmodel = fopen(model_file.c_str(), "wb");
    if(!fmodel) {
      wcerr<<L"Error: Cannot open file '"
          <<UtfConverter::fromUtf8(model_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    LexTorData lextor_data;

    lextor_data.read_stopwords(fstopwords);
    lextor_data.read_words(flexchoices);
    lextor_data.set_nwords_per_set(nwords_model);

    fstopwords.close();
    flexchoices.close();

    LexTor lexical_selector;
    lexical_selector.set_lextor_data(&lextor_data);

    LexTorData wordmodel;
    wordmodel.read(fwordmodel);
    fclose(fwordmodel);

    FSTProcessor fstpdic;
    fstpdic.load(fdic);
    fstpdic.initBiltrans();
    fclose(fdic);

    lextor_data.read_lexical_choices(fstpdic);

    FSTProcessor fstpbildic;
    fstpbildic.load(fbildic);
    fstpbildic.initBiltrans();
    fclose(fbildic);


    //Whe reading from the input corpus '*all* characters must be
    //processed, including ' ','\n', .....
    fcorpus.unsetf(ios::skipws);

    //Train
    lexical_selector.trainlch(fcorpus, nwords_left, nwords_right, wordmodel, fstpdic, fstpbildic, weight_exponent);

    fcorpus.close();

    //Write parameters
    lextor_data.write(fmodel);
    fclose(fmodel);
  }

  else if (mode==MODE_LEXTOR) {
    if(model_file=="") {
      wcerr<<L"Error: no model file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (dic_file=="") {
      wcerr<<L"Error: no dic file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_left<0) {
      wcerr<<L"Error: no left-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_right<0) {
      wcerr<<L"Error: no rigth-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    FILE *fdic=NULL;
    fdic=fopen(dic_file.c_str(), "rb");
    if (!fdic) {
      wcerr<<L"Error: Cannot open dictionary file '"
           <<UtfConverter::fromUtf8(dic_file)<<L"' for lexical selection\n";
      exit(EXIT_FAILURE);
    }
    FSTProcessor fstp;
    fstp.load(fdic);
    fstp.initBiltrans();
    fclose(fdic);

    FILE *fmodel = fopen(model_file.c_str(), "rb");
    if(!fmodel) {
      wcerr<<L"Error: Cannot open file '"
	   <<UtfConverter::fromUtf8(model_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    //Whe reading from the input stream '*all* characters must be
    //processed, including ' ','\n', .....
    wcin.unsetf(ios::skipws);

    LexTorData lextor_model;
    lextor_model.read(fmodel);
    fclose(fmodel);

    LexTor lexical_selector;
    lexical_selector.set_lextor_data(&lextor_model);

    lexical_selector.lexical_selector(wcin, fstp, nwords_left, nwords_right, weight_exponent);
  } 

  else if (mode==MODE_LEXTORTL) {
    if(stopwords_file=="") {
      wcerr<<L"Error: no stopwords file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(words_file=="") {
      wcerr<<L"Error: no words file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if(model_file=="") {
      wcerr<<L"Error: no target-language model file was given\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (dic_file=="") {
      wcerr<<L"Error: No lexical-selection dictionary was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (bildic_file=="") {
      wcerr<<L"Error: No bilingual dictionary was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_left<0) {
      wcerr<<L"Error: no left-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }
    if (nwords_right<0) {
      wcerr<<L"Error: no rigth-side context number of words was provided\n";
      help(argv[0]);
      exit(EXIT_FAILURE);
    }

    wifstream fstopwords, fwords;
    FILE *fdic=NULL, *fbildic=NULL, *fmodel = NULL;

    fstopwords.open(stopwords_file.c_str(), ios::in);
    if (fstopwords.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(stopwords_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fwords.open(words_file.c_str(), ios::in);
    if (fwords.fail()) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(words_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fmodel = fopen(model_file.c_str(), "rb");
    if(!fmodel) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(model_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fdic=fopen(dic_file.c_str(), "rb");
    if(!fdic) {
      wcerr<<L"Error: Cannot open file '"
           <<UtfConverter::fromUtf8(dic_file)<<L"'\n";
      exit(EXIT_FAILURE);
    }

    fbildic=fopen(bildic_file.c_str(), "rb");
    if(!fbildic) {
      wcerr<<L"Error: Cannot open file '"
	   <<UtfConverter::fromUtf8(bildic_file)<<L"'\n";
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
    fclose(fmodel);

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
    wcin.unsetf(ios::skipws);


    lexical_selector.set_tlmodel(&tlmodel);
    lexical_selector.set_bildic(&fstpbildic);

    lexical_selector.lexical_selector(wcin, fstpdic, nwords_left, nwords_right, weight_exponent);
  } 

  else {
    wcerr<<L"Error: No operation mode was provided\n";
    help(argv[0]);
    exit(EXIT_FAILURE);
  }
}
