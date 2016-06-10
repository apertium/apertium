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

#include <iostream>
#include <cmath>
#include <apertium/lextor_eval.h>
#include <apertium/lextor.h>
#include <apertium/string_utils.h>

using namespace Apertium;
LexTorEval::LexTorEval(wistream* iref) {
  nwords=0;
  //nunknown=0;
  nignored=0;
  npol=0;
  //nerrors_nopol=0;
  nerrors_pol=0;
  //nerrors_unk=0;

  ndefault=0;

  refer=iref;

  //words2ignore.insert();
  words2ignore.insert(L"as<cnjadv>");
  words2ignore.insert(L"at<pr>");
  words2ignore.insert(L"before<pr>");
  words2ignore.insert(L"but<cnjcoo>");
  words2ignore.insert(L"by<pr>");
  words2ignore.insert(L"for<pr>");
  words2ignore.insert(L"how<adv>");
  words2ignore.insert(L"in<pr>");
  words2ignore.insert(L"just<adv>");
  words2ignore.insert(L"off<pr>");
  words2ignore.insert(L"on<pr>");
  words2ignore.insert(L"over<pr>");
  words2ignore.insert(L"right<adv>");
  words2ignore.insert(L"since<cnjadv>");
  words2ignore.insert(L"whether<cnjadv>");
}

LexTorEval::~LexTorEval() {
}

void 
LexTorEval::print_evaluation() {
  wcerr<<L"# of words.......................................... "<<nwords<<L"\n"
      <<L"# of ignored words.................................. "<<nignored<<L"\n"
      <<L"# of polisemous words............................... "<<npol<<L"\n"
      <<L"# of errors due to polisemous words................. "<<nerrors_pol<<L"\n"
      <<L"# of times context does not discriminate (NODIS).... "<<ndefault<<L"\n"
      <<L"% of polysemous words............................... "<<(npol/nwords)*100.0<<L" %\n"
      <<L"% of error over polisemous words ................... "<<(nerrors_pol/npol)*100.0<<L" %\n"
      <<L"% of error over all words .......................... "<<(nerrors_pol/nwords)*100.0<<L" %\n"
      <<L"% of NODIS.......................................... "<<(ndefault/npol)*100.0<<L" %\n";
  wcerr<<L"NOTE: # ignored words ARE NOT included in the rest of counts\n";

  return;

  wcerr<<L"\nReport by words:\n---------------------------------------\n";

  map<wstring, double>::iterator it;
  wcerr<<L"WORD\t\tOCCURR\tERROR\tDEFAULT\t%ERROR\t%DEFAULT\n";
  wcerr<<L"-----------------------------------------------------------------\n";
  for(it=nwords_per_word.begin(); it!=nwords_per_word.end(); it++) {
    wcerr<<it->first<<L"\t"<<it->second<<L"\t"<<nerrors_per_word[it->first]<<L"\t"
	<<ndefault_per_word[it->first]<<L"\t"<<(nerrors_per_word[it->first]/it->second)*100<<L"\t"
	<<(ndefault_per_word[it->first]/it->second)*100<<L"\n";
  }
}

void 
LexTorEval::evalword(LexTorWord& ltword, int winner, LexTorData* lextor_data) {
  wstring reduced_w=lextor_data->reduce(ltword.get_lexical_choice(winner,false));
  wstring word=lextor_data->reduce(ltword.get_word_string());
  wstring wref;
  wstring reduced_wref;
  bool ignore=false;

  getline(*refer,wref);

  //  if (words2ignore.find(word)!=words2ignore.end()) {
  //  return;
  //}

  if (wref.find(L">__IGNORE") != wstring::npos) 
    ignore=true;
  
  if (!ignore) {
    nwords+=1.0;
    reduced_wref=lextor_data->reduce(wref);
    if (ltword. n_lexical_choices()>1) {
      npol+=1.0;
      nwords_per_word[word]+=1.0;
      if (winner<0) {
	ndefault+=1.0;
	ndefault_per_word[word]+=1.0;
      }
      if (reduced_w!=reduced_wref) {
	nerrors_pol+=1.0;
	nerrors_per_word[word]+=1.0;
	if (LexTor::debug) {
	  wcerr<<L"========= ERROR\n";
	}
      } else {
	if (LexTor::debug) {
	  wcerr<<L"========= OK\n";
	}
      }
    } else {
      //if (LexTor::debug)
      //  wcerr<<"EVAL: reduced_w="<<reduced_w<<" reduced_wref="<<reduced_wref<<"\n";

      if(reduced_wref!=reduced_w) {
	wcerr<<L"Error: Input and reference corpora are not aligned\n";
	wcerr<<L"word="<<reduced_w<<L" ref. word="<<reduced_wref<<L"\n";
	wcerr<<L"Number of words: "<<nwords+nignored<<L"\n";
	exit(EXIT_FAILURE);
      }
    }
  } else {
    //reduced_wref=wref;
    nignored+=1.0;
    if (LexTor::debug) {
      wcerr<<L"========= IGNORED\n";
    }

  }
}
