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
/** PoS tagger main program.
 *
 *  @author Felipe Sánchez-Martínez - fsanchez@dlsi.ua.es
 */

#include <apertium/tagger.h>

#include <apertium/hmm.h>
#include <apertium/lswpost.h>
#include <apertium/tagger_utils.h>
#include <apertium/tsx_reader.h>
#include <apertium/tagger_word.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <libgen.h>
#include <locale.h>

#include <sys/types.h>
#include <unistd.h>
#include <getopt.h>
#include <apertium/string_utils.h>
#ifdef _MSC_VER
#include <io.h>
#include <fcntl.h>
#endif

using namespace Apertium;
using namespace std;

void
Tagger::setShowSF(bool val) {
  showSF = val;
}

bool
Tagger::getShowSF() {
  return showSF;
}

int
Tagger::getMode(int argc, char *argv[]) {
  int mode=UNKNOWN_MODE;
   
  int c;

  is_sw = false;

#if HAVE_GETOPT_LONG
  int option_index=0;
#endif

  while (true) {
#if HAVE_GETOPT_LONG
    static struct option long_options[] =  {
      {"sliding-window",   no_argument, 0, 'w'},
      {"train",      required_argument, 0, 't'},
      {"supervised", required_argument, 0, 's'},
      {"retrain",    required_argument, 0, 'r'},
      {"tagger",     no_argument,       0, 'g'},
      {"show-superficial", no_argument, 0, 'p'},
      {"eval",       no_argument,       0, 'e'},
      {"first",      no_argument,       0, 'f'},
      {"help",       no_argument,       0, 'h'}, 
      {"debug",      no_argument,       0, 'd'}, 
      {"mark",       no_argument,       0, 'm'},
      {"null-flush", no_argument,       0, 'z'},
      {"help",       no_argument,       0, 'h'},
      {0, 0, 0, 0}
    };

    c=getopt_long(argc, argv, "wmdt:s:r:gpefhz",long_options, &option_index);
#else
    c=getopt(argc, argv, "wmdt:s:r:gpefhz");
#endif
    if (c==-1)
      break;
      
    switch (c) {
      case 'w':
        is_sw = true;
        if (mode == TRAIN_HMM_UNSUPERVISED_MODE) {
          mode = TRAIN_LSW_UNSUPERVISED_MODE;
        } else if (mode == TRAIN_HMM_SUPERVISED_MODE) {
          mode = TRAIN_LSW_SUPERVISED_MODE;
        } else if (mode == RETRAIN_HMM_UNSUPERVISED_MODE) {
          mode = RETRAIN_LSW_UNSUPERVISED_MODE;
        } else if (mode == TAGGER_HMM_MODE) {
          mode = TAGGER_LSW_MODE;
        } else if (mode == TAGGER_HMM_EVAL_MODE) {
          mode = TAGGER_LSW_EVAL_MODE;
        } else if (mode == TAGGER_HMM_FIRST_MODE) {
          mode = TAGGER_LSW_FIRST_MODE;
        }
        break;
      case 'm':
	TaggerWord::generate_marks = true;
        break;
        
      case 'd':
        debug=true;
        break;

      case 't':  //Training
        if(!isNumber(optarg)) {
	  wcerr <<L"Error: mandatory --train argument <n> must be zero or a positive integer\n";
	  help();
        }
        else {
          nit = atoi(optarg); //Number of iterations
        }
        if(mode==UNKNOWN_MODE) {
          if (is_sw) {
            mode = TRAIN_LSW_UNSUPERVISED_MODE;
          }
          else {
            mode = TRAIN_HMM_UNSUPERVISED_MODE;
          }
        }
        else {
	  wcerr<<L"Error: --train <n> argument cannot be mixed with --retrain or --tagger arguments\n";
	  help();
	}
	break;
      
      case 's':
        if(!isNumber(optarg)) {
	  wcerr<<L"Error: mandatory --supervised argument <n> must be zero or a positive integer\n";
	  help();
        }
        else {
          nit = atoi(optarg); //Number of iterations
        }

        if(mode==UNKNOWN_MODE) {
          if (is_sw) {
            mode = TRAIN_LSW_SUPERVISED_MODE;
          }
          else {
            mode = TRAIN_HMM_SUPERVISED_MODE;
          }
        }
        else {
	  wcerr<<L"Error: --supervised optional argument should only appear after --train <n> argument\n";
	  help();
	}
	break;

      case 'p':
        setShowSF(true);
	break;
	
      case 'r':
        if(!isNumber(optarg)) {
	  wcerr<<L"Error: mandatory --train argument <n> must be zero or a positive integer\n";
          help();
        } 
        else { 
	  nit = atoi(optarg); //Number of iterations
        }
        if(mode==UNKNOWN_MODE) {
          if (is_sw) {
            mode = RETRAIN_LSW_UNSUPERVISED_MODE;
          }
          else {
            mode = RETRAIN_HMM_UNSUPERVISED_MODE;
          }
        }
        else {
	  wcerr<<L"Error: --retrain argument cannot be mixed with --train or --tagger arguments\n";
	  help();
	}
        break;
        
      case 'g': 
        if(mode==UNKNOWN_MODE) {
          if (is_sw) {
            mode = TAGGER_LSW_MODE;
          }
          else {
	        mode = TAGGER_HMM_MODE;
          }
        }
        else {
          wcerr<<L"Error: --tagger argument cannot be mixed with --train or --retrain arguments\n";
          help();
        }
        break;
         
      case 'e':
        if(mode==TAGGER_HMM_MODE) {
          mode = TAGGER_HMM_EVAL_MODE;
        }
        else if (mode == TAGGER_LSW_MODE) {
          mode = TAGGER_LSW_EVAL_MODE;
        }
        else {
          wcerr<<L"Error: --eval optional argument should only appear after --tagger argument\n";
	  help();
	}
	break;
        
      case 'f': 
        if(mode==TAGGER_HMM_MODE) {
          mode = TAGGER_HMM_FIRST_MODE;
        }
        else if (mode == TAGGER_LSW_MODE) {
          mode = TAGGER_LSW_FIRST_MODE;
        }
        else {
          wcerr<<L"Error: --first optional argument should only appear after --tagger argument\n";
	  help();
	} 
	break;
      
      case 'z':
        null_flush = true;
        break;
        
      case 'h':
        help(); 
        break;
     
      default:
        //wcerr<<L"Error: getopt() returned the char code '"<<c<<L"'\n";
        help();
        break;
    }    
  }

  if(mode==UNKNOWN_MODE) {
    wcerr<<L"Error: Arguments missing\n";
    help();
  }   
  
  switch(argc-optind) {
    case 6:
      if(mode != TRAIN_HMM_SUPERVISED_MODE
          && mode != TRAIN_LSW_SUPERVISED_MODE) {
        help();
      }
      break;
    
    case 4:
      if(mode != TRAIN_HMM_UNSUPERVISED_MODE
          && mode != TRAIN_LSW_UNSUPERVISED_MODE) {
        help();
      }
      break;
    case 3:
      if (mode != TAGGER_HMM_MODE
          && mode != TAGGER_HMM_FIRST_MODE
          && mode != TAGGER_LSW_MODE
          && mode != TAGGER_LSW_FIRST_MODE) {
        help();
      }
      break;
      
    case 2:
      if(mode != RETRAIN_HMM_UNSUPERVISED_MODE
          && mode != TAGGER_HMM_MODE
          && mode != RETRAIN_LSW_UNSUPERVISED_MODE
          && mode != TAGGER_LSW_MODE) {
        help();
      }
      break;
    
    case 1:
      if (mode != TAGGER_HMM_MODE
          && mode != TAGGER_HMM_FIRST_MODE
          && mode != TAGGER_LSW_MODE
          && mode != TAGGER_LSW_FIRST_MODE)  {
        help();
      }
      break;
    
    default:
      help();
      break;
  }

  for(int i = optind; i != argc; i++) {
    filenames.push_back(argv[i]);
  }
  
  return mode;
}

Tagger::Tagger() {
  debug = false;
  showSF = false;
  null_flush = false;
}

void
Tagger::main(int argc, char *argv[]) {
  name = argv[0];
  int mode = getMode(argc, argv);

  switch(mode)  {
    case TRAIN_HMM_UNSUPERVISED_MODE:
      trainHMM();
      break;

    case TRAIN_LSW_UNSUPERVISED_MODE:
      trainLSW();
      break;
    
    case TRAIN_HMM_SUPERVISED_MODE:
      trainHMMSupervised();
      break;

    case TRAIN_LSW_SUPERVISED_MODE:
      trainLSWSupervised();
      break;

    case RETRAIN_HMM_UNSUPERVISED_MODE:
      retrainHMM();
      break;
      
    case RETRAIN_LSW_UNSUPERVISED_MODE:
      retrainLSW();
      break;
      
    case TAGGER_HMM_MODE:
      taggerHMM();
      break;

    case TAGGER_LSW_MODE:
      taggerLSW();
      break;

    case TAGGER_HMM_FIRST_MODE:
      taggerHMM(true);
      break;

    case TAGGER_LSW_FIRST_MODE:
      taggerLSW(true);
      break;

    default:
      wcerr<<L"Error: Unknown working mode mode\n";
      help();
      break;
  }
}

void
Tagger::taggerHMM(bool mode_first) {
  FILE *ftdata = fopen(filenames[0].c_str(), "rb");
  if (!ftdata) {
    filerror(filenames[0]);
  }

  TaggerDataHMM tdhmm;
  tdhmm.read(ftdata);
  fclose(ftdata);
  
  HMM hmm(&tdhmm);

  hmm.set_show_sf(showSF);
  hmm.setNullFlush(null_flush);

  if(filenames.size() == 1) {
    hmm.tagger(stdin, stdout, mode_first);
  }
  else {
    FILE *finput = fopen(filenames[1].c_str(), "r");
    if (!finput) {
      filerror(filenames[1]);
    }
#ifdef _MSC_VER
	_setmode(_fileno(finput), _O_U8TEXT);
#endif
    if(filenames.size() == 2) {
      hmm.tagger(finput, stdout, mode_first);
    }
    else  {
      FILE *foutput = fopen(filenames[2].c_str(), "w");
      if (!foutput) {
        filerror(filenames[2]);
      }
#ifdef _MSC_VER
	  _setmode(_fileno(foutput), _O_U8TEXT);
#endif

      hmm.tagger(finput, foutput, mode_first);
      fclose(foutput);
    }
    fclose(finput);
  }
}

void
Tagger::taggerLSW(bool mode_first) {
  FILE *ftdata = fopen(filenames[0].c_str(), "rb");
  if (!ftdata) {
    filerror(filenames[0]);
  }

  TaggerDataLSW tdlsw;
  tdlsw.read(ftdata);
  fclose(ftdata);
  
  LSWPoST lswpost(&tdlsw);
  lswpost.set_show_sf(showSF);
  lswpost.setNullFlush(null_flush);

  if(filenames.size() == 1) {
    lswpost.tagger(stdin, stdout, mode_first);
  }
  else {
    FILE *finput = fopen(filenames[1].c_str(), "r");
    if (!finput) {
      filerror(filenames[1]);
    }
#ifdef _MSC_VER
	_setmode(_fileno(finput), _O_U8TEXT);
#endif
    if(filenames.size() == 2) {
      lswpost.tagger(finput, stdout, mode_first);
    }
    else {
      FILE *foutput = fopen(filenames[2].c_str(), "w");
      if (!foutput) {
        filerror(filenames[2]);
      }
#ifdef _MSC_VER
	  _setmode(_fileno(foutput), _O_U8TEXT);
#endif

      lswpost.tagger(finput, foutput, mode_first);
      fclose(foutput);
    }
    fclose(finput);
  }
}

void
Tagger::filerror(string const &filename) {
  cerr << "Error: cannot open file '" << filenames[0] << "'\n\n";
  help();
}

void
Tagger::trainHMM() {
  TSXReader treader;
  treader.read(filenames[2]);
  TaggerDataHMM tdhmm(treader.getTaggerData());
  HMM hmm(&tdhmm);
  hmm.set_debug(debug);
  hmm.set_eos((tdhmm.getTagIndex())[L"TAG_SENT"]);
  TaggerWord::setArrayTags(tdhmm.getArrayTags());
  
  wcerr << L"Calculating ambiguity classes...\n";
  FILE *fdic = fopen(filenames[0].c_str(), "r");
  if(fdic) {
    hmm.read_dictionary(fdic);
  }
  else {
    filerror(filenames[0]);
  }
  wcerr << L"Kupiec's initialization of transition and emission probabilities...\n";
  FILE *fcrp = fopen(filenames[1].c_str(), "r");
  if(fcrp) {
#ifdef _MSC_VER
    _setmode(_fileno(fcrp), _O_U8TEXT);
#endif 
    hmm.init_probabilities_kupiec(fcrp);               
  }
  else {
    filerror(filenames[1]);
  }
  
  wcerr << L"Applying forbid and enforce rules...\n";
  hmm.apply_rules();
  
  wcerr << L"Training (Baum-Welch)...\n";
  for(int i=0; i != nit; i++) {
    fseek(fcrp, 0, SEEK_SET);
    hmm.train(fcrp);
  }
  wcerr << L"Applying forbid and enforce rules...\n";
  hmm.apply_rules();

  fclose(fdic);
  fclose(fcrp);

  FILE *ftdata = fopen(filenames[3].c_str(), "wb");
  if(!ftdata)  {
    filerror(filenames[3]);
  }
  tdhmm.write(ftdata);
  fclose(ftdata);
}

void
Tagger::trainLSW() {
  TSXReader treader;
  treader.read(filenames[2]);
  TaggerDataLSW tdlsw(treader.getTaggerData());
  LSWPoST lswpost(&tdlsw);
  lswpost.set_debug(debug);
  lswpost.set_eos(tdlsw.getTagIndex()[L"TAG_SENT"]);
  TaggerWord::setArrayTags(tdlsw.getArrayTags());

  wcerr << L"Calculating ambiguity classes...\n";
  FILE *fdic = fopen(filenames[0].c_str(), "r");
  if(fdic) {
    lswpost.read_dictionary(fdic);
  }
  else {
    filerror(filenames[0]);
  }
  wcerr << L"Average initialization of Light Sliding-Window probabilities, with forbid and enforce rules...\n";
  FILE *fcrp = fopen(filenames[1].c_str(), "r");
  if(fcrp) {
#ifdef _MSC_VER
    _setmode(_fileno(fcrp), _O_U8TEXT);
#endif
    lswpost.init_probabilities(fcrp);
  }
  else {
    filerror(filenames[1]);
  }

  wcerr << L"Training (Light Sliding-Window, Unsupervised)...\n";
  for(int i=0; i != nit; i++) {
    fseek(fcrp, 0, SEEK_SET);
    lswpost.train(fcrp);
    wcout << L"iteration " << (i + 1) << " done." << endl;
  }

  fclose(fdic);
  fclose(fcrp);

  FILE *ftdata = fopen(filenames[3].c_str(), "wb");
  if(!ftdata)  {
    filerror(filenames[3]);
  }
  tdlsw.write(ftdata);
  fclose(ftdata);
}

void
Tagger::trainHMMSupervised() {
  TSXReader treader;
  treader.read(filenames[2]);
  TaggerDataHMM tdhmm(treader.getTaggerData());
  HMM hmm(&tdhmm);
  hmm.set_debug(debug);
  hmm.set_eos(tdhmm.getTagIndex()[L"TAG_SENT"]);
  TaggerWord::setArrayTags(tdhmm.getArrayTags());
  
  wcerr << L"Calculating ambiguity classes...\n";
  FILE *fdic = fopen(filenames[0].c_str(), "r");
  if(fdic) {
    hmm.read_dictionary(fdic);
  }
  else {
    filerror(filenames[0]);
  }
  wcerr << L"Kupiec's initialization of transition and emission probabilities...\n";
  FILE *ftagged = fopen(filenames[4].c_str(), "r");
  FILE *funtagged = fopen(filenames[5].c_str(), "r");
  if(ftagged && funtagged)  {
#ifdef _MSC_VER
    _setmode(_fileno(ftagged), _O_U8TEXT);
    _setmode(_fileno(funtagged), _O_U8TEXT);
#endif 
    wcerr << L"Initializing transition and emission probabilities from a hand-tagged corpus...\n";
    hmm.init_probabilities_from_tagged_text(ftagged, funtagged);
  }
  else {
    filerror(filenames[4]+ "' or '" + filenames[5]);
  }
  fclose(ftagged);
  fclose(funtagged);
  
  wcerr << L"Applying forbid and enforce rules...\n";
  hmm.apply_rules();
  
  wcerr << L"Training (Baum-Welch)...\n";
  FILE *fcrp = fopen(filenames[1].c_str(), "r");
  if(fcrp)  {
#ifdef _MSC_VER
    _setmode(_fileno(fcrp), _O_U8TEXT);
#endif 
    for(int i=0; i != nit; i++)  {
      fseek(fcrp, 0, SEEK_SET);
      hmm.train(fcrp);
    }
    wcerr << L"Applying forbid and enforce rules...\n";
    hmm.apply_rules();
  }
  else {
    filerror(filenames[1]);
  }

  fclose(fdic);
  fclose(fcrp);

  FILE *ftdata = fopen(filenames[3].c_str(), "wb");
  if(!ftdata)  {
    filerror(filenames[3]);
  }
  tdhmm.write(ftdata);
  fclose(ftdata);
}

void
Tagger::trainLSWSupervised() {
}

void
Tagger::retrainHMM() {
  TaggerDataHMM tdhmm;
  FILE *ftdata = fopen(filenames[1].c_str(), "rb");
  if(!ftdata) {
    filerror(filenames[1]);
  }
  tdhmm.read(ftdata);
  fclose(ftdata);

  HMM hmm(&tdhmm);
  hmm.set_debug(debug);
  hmm.set_eos((tdhmm.getTagIndex())[L"TAG_SENT"]);
  TaggerWord::setArrayTags(tdhmm.getArrayTags());

  FILE *fcrp = fopen(filenames[0].c_str(), "r");
  if(!fcrp)  {
    filerror(filenames[0]);
  }
#ifdef _MSC_VER
  _setmode(_fileno(fcrp), _O_U8TEXT);
#endif 
  wcerr << L"Training (Baum-Welch)...\n";
  for(int i=0; i != nit; i++)  {
    fseek(fcrp, 0, SEEK_SET);
    hmm.train(fcrp);
  }
  wcerr << L"Applying forbid and enforce rules...\n";
  hmm.apply_rules();
  fclose(fcrp);

  ftdata = fopen(filenames[1].c_str(), "wb");
  if(!ftdata)  {
    filerror(filenames[1]);
  }
  tdhmm.write(ftdata);
  fclose(ftdata);
}

void
Tagger::retrainLSW() {
  TaggerDataLSW tdlsw;
  FILE *ftdata = fopen(filenames[1].c_str(), "rb");
  if(!ftdata) {
    filerror(filenames[1]);
  }
  tdlsw.read(ftdata);
  fclose(ftdata);

  LSWPoST lswpost(&tdlsw);
  lswpost.set_debug(debug);
  lswpost.set_eos((tdlsw.getTagIndex())[L"TAG_SENT"]);
  TaggerWord::setArrayTags(tdlsw.getArrayTags());

  FILE *fcrp = fopen(filenames[0].c_str(), "r");
  if(!fcrp)  {
    filerror(filenames[0]);
  }
#ifdef _MSC_VER
  _setmode(_fileno(fcrp), _O_U8TEXT);
#endif 
  wcerr << L"Training (Light Sliding-Window, Unsupervised)...\n";
  for(int i=0; i != nit; i++)  {
    fseek(fcrp, 0, SEEK_SET);
    lswpost.train(fcrp);
    wcout << L"iteration " << (i + 1) << " done." << endl;
  }
  fclose(fcrp);

  ftdata = fopen(filenames[1].c_str(), "wb");
  if(!ftdata)  {
    filerror(filenames[1]);
  }
  tdlsw.write(ftdata);
  fclose(ftdata);
}

void
Tagger::help() {
  ostream &out = cerr;
  char* localname=new char[name.size()+1];
  strcpy(localname, name.c_str());
  out << basename(localname) << ": HMM/LSW part-of-speech tagging and training program" << endl;
  out << "GENERIC USAGE: " << basename(localname) << "[-d] <OPTION>=[PARAM] [FILES]" << endl;
  out << "USAGE: " << basename(localname) << "[-d] [-w] -t=n DIC CRP TSX TAGGER_DATA" << endl;
  out << "       " << basename(localname) << "[-d] [-w] -s=n DIC CRP TSX TAGGER_DATA HTAG UNTAG" << endl;
  out << "       " << basename(localname) << "[-d] [-w] -r=n CRP TAGGER_DATA" << endl;
  out << "       " << basename(localname) << "[-d] [-w] -g [-f] TAGGER_DATA [INPUT [OUTPUT]]" << endl;
  out << endl;
  out << "Where OPTIONS are:" << endl;
  out << "  -w, --sliding-window:   use the Sliding-Window training and tagging algorithm," << endl;
  out << "                          or if not specified, use the HMM algorithm by default" << endl;
  out << "  -t, --train=n:          performs n iterations of training (unsupervised)" << endl;
  out << "  -s, --supervised=n:     initializes parameters against a hand-tagged text (supervised)," << endl;
  out << "                          and trains it with n iterations" << endl;
  out << "  -r, --retrain=n:        retrains the model with n aditional iterations (unsupervised)" << endl;
  out << "  -g, --tagger:           tags input text" << endl;
  out << "  -p, --show-superficial: show superficial forms in the output stream" << endl;
  out << "  -f, --first:            used in conjuntion with -g (--tagger) makes the tagger"<< endl;
  out << "                          give all lexical forms of each word, with the chosen" << endl;
  out << "                          one in the first place (after the lemma)"<<endl;
  out << "  -d, --debug:            print error mesages when tagging input text" << endl;
  out << "  -m, --mark:             generate marks of solved ambiguities" << endl;
  out << "  -z, --null-flush:       flush output stream when reading '\\0' characters" <<endl;
  out << endl;
  out << "And FILES are:" << endl;          
  out << "  DIC:         full expanded dictionary file" << endl;
  out << "  CRP:         training text corpus file" << endl;
  out << "  TSX:         tagger specification file, in XML format" << endl;
  out << "  TAGGER_DATA: tagger data file, built in the training and used while" << endl;
  out << "               tagging" << endl;
  out << "  HTAG:        hand-tagged text corpus" << endl;
  out << "  UNTAG:       untagged text corpus, morphological analysis of HTAG" << endl;
  out << "               corpus to use both jointly with -s option" << endl; 
  out << "  INPUT:       input file, stdin by default" << endl;
  out << "  OUTPUT:      output file, stdout by default" << endl;
  delete[] localname;
  exit(EXIT_FAILURE);
}

bool
Tagger::isNumber(const char *str) {
  for(unsigned int i = 0, limit = strlen(str); i != limit; i++) {
    if(!isdigit(str[i]))  {
      return false;
    }
  }
  
  return true;
}
