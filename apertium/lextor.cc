/*
 * Copyright (C) 2006 Universitat d'Alacant / Universidad de Alicante
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

#include <apertium/lextor.h>
#include <apertium/string_utils.h>

#include <algorithm>
#include <string>
#include <cmath>
#include <apertium/string_utils.h>

using namespace Apertium;


#define PI 3.14159265358979323846264338327950288

bool LexTor::debug;
double LexTor::angleth;

LexTor::LexTor() :
fstpbil(0)
{
  lextor_data=NULL;
  tlmodel=NULL;
}
  
LexTor::LexTor(const LexTor& lt) :
fstpbil(0)
{
  lextor_data=lt.lextor_data;
  tlmodel=lt.tlmodel;
}
  
LexTor::~LexTor() {
}

void 
LexTor::set_lextor_data(LexTorData* ltd) {
  lextor_data=ltd;
}

void 
LexTor::set_tlmodel(LexTorData* tlm) {
  tlmodel=tlm;
}

void 
LexTor::set_bildic(FSTProcessor *fstp) {
  fstpbil=fstp;
}

void 
LexTor::trainwrd(wistream& is, int left, int right, double weigth_exponent) {
  if (lextor_data==NULL) {
    wcerr<<L"Error in LexTor::trainwrd, you must call set_lextor_data before training\n";
    exit(EXIT_FAILURE);
  }

  lextor_data->ensure_stopwords_ok();

  wcerr<<L"Number of words to take into account on the left side: "<<left<<L"\n";
  wcerr<<L"Number of words to take into account on the right side: "<<right<<L"\n";

  set<wstring> words2workwith=lextor_data->get_words();
  set<wstring>::iterator itword;

  map<wstring, COUNT_DATA_TYPE> wordsum;

  wcerr<<L"Words to work with:\n";
  for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
    wcerr<<*itword<<L"\n";
  }
  wcerr<<L"\n";

  is.clear();
  is.seekg(ios::beg);

  int nw=0;

  map<wstring, map<wstring, COUNT_DATA_TYPE> > context;
  deque<wstring> buffer;
  unsigned word_index=(unsigned)left;

  unsigned buffer_max_size=(unsigned)(left+1+right);
  
  LexTorWord *ltword;
  ltword=LexTorWord::next_word(is);
  while(ltword!=NULL) {
    if ((++nw%250000)==0)
      wcerr<<nw<<L" words processed\n";
      
    if(debug) {
      wcerr<<L"Word read from corpus: "<<ltword->get_word_string()
	  <<L", reduced: "<<lextor_data->reduce(ltword->get_word_string())<<flush;
      getchar();
    }

    wstring reduced_word=lextor_data->reduce(ltword->get_word_string());

    if (!lextor_data->is_stopword(reduced_word)) {
      if (buffer.size()>=buffer_max_size) {
	buffer.pop_front();
      }
      buffer.push_back(reduced_word);

      wordsum[reduced_word]+=1.0;

      //The buffer is already full
      if (buffer.size()==buffer_max_size) {
	for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
	  if (buffer[word_index]==(*itword)) {
	    if(debug) {
	      wcerr<<L"WINDOW: ";
	      for (unsigned i=0; i<buffer.size(); i++) {
		if(i==word_index)
		  wcerr<<L"[>>>>"<<buffer[i]<<L"<<<<] ";
		else
		  wcerr<<L"["<<buffer[i]<<L"] ";
	      }
	      wcerr<<L"\n";
	    }

	    int distance=(-1)*left;
	    for(unsigned i=0; i<buffer.size(); i++) {
	      if ((i!=word_index) && (buffer[i]!=(*itword))) {
		if (debug) {
		  wcerr<<L"   WORD: ["<<buffer[i]<<L"] ";
		  wcerr<<L"   DISTANCE: "<<distance<<L" ";
		  wcerr<<L"   ADDED COUNT: "<<1.0/pow(fabs((double)distance),weigth_exponent)<<L" ";
		  wcerr<<L"   TO ["<<*itword<<L"]\n";
		}
		context[*itword][buffer[i]]+=1.0/pow(fabs((double)distance),weigth_exponent);
	      }
	      distance++;
	    }
	    if (debug)
	      getchar();
	    break;
	  }
	}
      }
    }

    delete ltword;
    ltword=LexTorWord::next_word(is);
  }

  wcerr<<L"Corpus has "<<nw<<L" words\n";

  //Set the count of each word
  map<wstring, COUNT_DATA_TYPE>::iterator itws;
  for(itws=wordsum.begin(); itws!=wordsum.end(); itws++) {
    lextor_data->set_wordcount(itws->first,itws->second);
    //if(debug) {
    wcerr<<L"wordcount("<<itws->first<<L") = "<<itws->second<<L"\n";
    //}
  }

  //All co-occurrences have been collected. We need to filter them
  //so as to take into account only the n most frequents
  for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
    PairStringCountComparer comparer;
    vector<pair<wstring, COUNT_DATA_TYPE> > context_v;
    map<wstring, COUNT_DATA_TYPE>::iterator itm;

    while(context[*itword].size()>0) {
      itm=context[*itword].begin();
      context_v.push_back(*itm);
      context[*itword].erase(itm);
    }

    sort(context_v.begin(), context_v.end(), comparer);
    wstring w=*itword;
    lextor_data->set_cooccurrence_context(w, context_v);
    lextor_data->set_lexchoice_sum(w, wordsum[w]);

    //if (debug) {
    wcerr<<L"lexchoice_sum("<<w<<L") = "<<wordsum[w]<<L"\n";
    //}
  }
}

void 
LexTor::trainlch(wistream& is, int left, int right, LexTorData& tlwordmodel, 
                 FSTProcessor& dic, FSTProcessor& bildic, double weigth_exponent) {
  if (lextor_data==NULL) {
    wcerr<<L"Error in LexTor::trainlch, you must call set_lextor_data before training\n";
    exit(EXIT_FAILURE);
  }

  lextor_data->ensure_stopwords_ok();

  wcerr<<L"Number of words to take into account on the left side: "<<left<<L"\n";
  wcerr<<L"Number of words to take into account on the right side: "<<right<<L"\n";

  set<wstring> words2workwith=lextor_data->get_words();
  set<wstring>::iterator itword;

  map<wstring, COUNT_DATA_TYPE> wordsum;
  map<wstring, COUNT_DATA_TYPE> lechsum;

  wcerr<<L"Words to work with:\n";
  for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
    wcerr<<*itword<<L"\n";
  }
  wcerr<<L"\n";

  //For a given lexical choice it stores its translation
  map<wstring, wstring> lexchoice_translation;
  map<wstring, set<wstring> > lexical_choices_of_word;

  wcerr<<L"Lexical choices:\n";
  for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
    set<wstring> lexical_choices=lextor_data->get_lexical_choices(*itword);
    lexical_choices_of_word[*itword]=lexical_choices;
    set<wstring>::iterator itlch;
    for(itlch=lexical_choices.begin(); itlch!=lexical_choices.end(); itlch++) {
      lexchoice_translation[*itlch]=tlwordmodel.reduce(bildic.biltrans(*itlch,false));
      wcerr<<*itlch<<L", tr:"<<lexchoice_translation[*itlch]<<L"\n";
    }
  }
  wcerr<<L"\n";


  is.clear();
  is.seekg(ios::beg);

  int nw=0;

  map<wstring, map<wstring, COUNT_DATA_TYPE> > context;
  deque<LexTorWord> buffer;

  int word_index=left;
  unsigned buffer_max_size=left+right+1;

  LexTorWord *ltword;
  ltword=LexTorWord::next_word(is,&dic);
  while(ltword!=NULL) {
    if (debug) {
      wcerr<<L"Word read from corpus: "<<ltword->get_word_string()<<L", reduce: "<<lextor_data->reduce(ltword->get_word_string());
      getchar();
    }
    if ((++nw%250000)==0)
      wcerr<<nw<<L" words processed\n";

    wstring reduced_word=lextor_data->reduce(ltword->get_word_string());

    if (!lextor_data->is_stopword(reduced_word)) {	
      if (buffer.size()>=buffer_max_size) {
	buffer.pop_front();
      }
      buffer.push_back(*ltword);

      wordsum[reduced_word]+=1.0;

      //The buffer is already full
      if (buffer.size()==buffer_max_size) {

	wstring reduced_buffer_word=lextor_data->reduce(buffer[word_index].get_word_string());

        for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
	  if (reduced_buffer_word==(*itword)) {
	    //We translate each word in the context
	    //Note: Words in the context can also be ambiguous (with more than one lexical choice)
	    //In that case the count will come from all the possible
	    //translations 
        vector <vector<wstring> > translation_buffer(buffer_max_size);
		vector <wstring> reduced_buffer(buffer_max_size);

	    for (int i=0; i<(int)buffer_max_size; i++) {
	      reduced_buffer[i]=lextor_data->reduce(buffer[i].get_word_string());	      
	    }

	    if(debug) {
	      wcerr<<L"WINDOW: ";
	      for (unsigned i=0; i<buffer.size(); i++) {
		if(i==(unsigned)word_index)
		  wcerr<<L"[>>>>"<<reduced_buffer[i]<<L"<<<<] ";
		else
		  wcerr<<L"["<<reduced_buffer[i]<<L"] ";
	      }
	      wcerr<<L"\n";
	      wcerr<<L"TRANSLATED: ";
	    }

	    for (int i=0; i<(int)buffer_max_size; i++) {
	      wstring str_translations=L"";
	      for(int j=0; j<buffer[i].n_lexical_choices(); j++) {
		wstring aux_tr=buffer[i].translate(bildic,j);
		if (aux_tr.length()>0) {
		  wstring tr=tlwordmodel.reduce(aux_tr);
		  translation_buffer[i].push_back(tr);
		  str_translations+=tr+L"/";
		} else {
		  wcerr<<L"Warning in LexTor::trainlch: translation of ["<<buffer[i].get_word_string()
		      <<L"] is empty\n";
		}
	      }
	      if (debug) {
		if (i==word_index)
		  wcerr<<L"[>>>>"<<str_translations<<L"<<<<] ";
		else
		  wcerr<<L"["<<str_translations<<L"] ";
	      }
	    }

	    if(debug)
	      wcerr<<L"\n";

	    set<wstring> lexical_choices=lexical_choices_of_word[*itword];
	    set<wstring>::iterator itlch;

	    map<wstring, map<wstring, COUNT_DATA_TYPE> > local_context;
	    map<wstring, COUNT_DATA_TYPE> sumvotes_context;

	    //For each lexical choice the counts from the TL are collected
	    for(itlch=lexical_choices.begin(); itlch!=lexical_choices.end(); itlch++) {
	      for (int i=0; i<(int)buffer_max_size; i++) {
		if ((i!=word_index)&&(reduced_buffer[i]!=(*itword))) {
		  COUNT_DATA_TYPE target_vote=0;

		  //The counts of the TL co-occurrence model are transferred to the SL. If the SL word is ambiguous
		  //it will have more than one translation into TL, so we need to normalize using the frequency of words
		  //in the TL
		  vector <double> translation_weighs(translation_buffer[i].size());
		  double sum=0.0;
		  if (translation_buffer[i].size()>1) {
		    for(int j=0; j<(int)translation_buffer[i].size(); j++) {
		      translation_weighs[j]=tlwordmodel.get_lexchoice_sum(translation_buffer[i][j]);
		      sum+=translation_weighs[j];

		      //!!!!! Para hacer que no tenga en cuenta las polisemicas del contexto
		      ///////translation_weighs[j]=0;
		      //!!!!!

		      if (debug) {
			wcerr<<L"Frequency of translation ["<<translation_buffer[i][j]<<L"] ="
			    <<translation_weighs[j]<<L"\n";
		      }
		    }
		  } else {
		    translation_weighs[0]=1;
		    sum=1;
		  }

		  for(int j=0; j<(int)translation_buffer[i].size(); j++) {
		    translation_weighs[j]=translation_weighs[j]/sum;
		    if (debug) {
		      wcerr<<L"Weight of translation ["<<translation_buffer[i][j]<<L"] ="
			  <<translation_weighs[j]<<L"\n";
		    }
		  }

		  for(int j=0; j<(int)translation_buffer[i].size(); j++) {
		    if (lexchoice_translation[*itlch].length()==0) {
		      wcerr<<L"Error: Translation of lexical choice '"<<*itlch<<L"' is empty\n";
		    }

		    double aux_vote=0;
		    //aux_vote=tlwordmodel.vote_from_word(lexchoice_translation[*itlch], 
		    //				    translation_buffer[i][j])*translation_weighs[j];
		    if (tlwordmodel.get_wordcount(lexchoice_translation[*itlch])>0) {
		      aux_vote=(tlwordmodel.vote_from_word(lexchoice_translation[*itlch],translation_buffer[i][j])/
				tlwordmodel.get_wordcount(lexchoice_translation[*itlch]))*translation_weighs[j];
		      if (debug) {
			wcerr<<L"C("<<lexchoice_translation[*itlch]<<L", "<<translation_buffer[i][j]<<L") = "
			    <<tlwordmodel.vote_from_word(lexchoice_translation[*itlch],translation_buffer[i][j])<<L"\n";
			wcerr<<L"C("<<lexchoice_translation[*itlch]<<L") = "<<tlwordmodel.get_wordcount(lexchoice_translation[*itlch])<<L"\n";
		      }
		    } else {
		      if (tlwordmodel.vote_from_word(lexchoice_translation[*itlch],translation_buffer[i][j])>0) {
			wcerr<<L"Error in LexTor::trainlch: TL vote is not null, but its word count is null.\n";
			wcerr<<L"lexchoice_translation: "<<lexchoice_translation[*itlch]<<L"\n";
			wcerr<<L"translation_buffer: "<<translation_buffer[i][j]<<L"\n";
		      }
		    }
		    target_vote+=aux_vote;

		    if(debug) {
		      wcerr<<L"Target vote for ["<<lexchoice_translation[*itlch]
			  <<L"] from ["<<translation_buffer[i][j]<<L"] = "<<aux_vote<<L"\n";
		    }
		  }

		  if (target_vote>0) {
		    local_context[*itlch][reduced_buffer[i]]+=target_vote;
		    sumvotes_context[reduced_buffer[i]]+=target_vote;
		  }
		}
	      }
	    }

	    if (debug) {
	      wcerr<<L"COUNTS NORMALIZATION\n";
	    }

	    //Now we normalize the counts and estimate the number of
	    //times each lexical choice has been seen.
	    map<wstring, COUNT_DATA_TYPE> local_lexsum;
	    double local_lexsumsum=0.0;
	    for(itlch=lexical_choices.begin(); itlch!=lexical_choices.end(); itlch++) {
	      int distance=(-1)*left;
	      for (int i=0; i<(int)buffer_max_size; i++) { 
		if ((i!=word_index)&&(reduced_buffer[i]!=(*itword))) {
		  if (local_context[*itlch][reduced_buffer[i]]>0) {
		    double cc=local_context[*itlch][reduced_buffer[i]]/sumvotes_context[reduced_buffer[i]];
		    double count_to_apply=cc/pow(fabs((double)distance),weigth_exponent);
		    context[*itlch][reduced_buffer[i]]+=count_to_apply;
		    if (debug) {
		      wcerr<<L"Lexical choice: ["<<*itlch
                          <<L"], context word: ["<<reduced_buffer[i]<<L"], "
			  <<L"normalize count: "<<cc<<L"\n";
		      wcerr<<L"Distance: "<<distance<<L", count to apply: "
			  <<count_to_apply<<L"\n";

		    }

		    local_lexsum[*itlch]+=count_to_apply;
		    local_lexsumsum+=count_to_apply;

		    if (debug) {
		      wcerr<<L"local_lexsum["<<*itlch<<L"] = "<<local_lexsum[*itlch]<<L"\n";
		      wcerr<<L"local_lexsumsum = "<<local_lexsumsum<<L"\n";
		    }

		  }
		}
		distance++;
	      }
	    }

	    for(itlch=lexical_choices.begin(); itlch!=lexical_choices.end(); itlch++) {
	      if ((local_lexsum[*itlch]>0) && (local_lexsumsum>0))
		lechsum[*itlch]+=local_lexsum[*itlch]/local_lexsumsum;
	      if (debug) {
		wcerr<<L"lechsum["<<*itlch<<L"] = "<<lechsum[*itlch]<<L"\n";
	      }
	    }
	    

	    if(debug) {
	      wcerr<<L"\n";
	      getchar();
	    }

	    break;
	  }
	}
      }
    } 

    delete ltword;
    ltword=LexTorWord::next_word(is,&dic);
  }
  
  wcerr<<L"Corpus has "<<nw<<L" words\n";

  //Set the count of each word
  map<wstring, COUNT_DATA_TYPE>::iterator itws;
  for(itws=wordsum.begin(); itws!=wordsum.end(); itws++) {
    lextor_data->set_wordcount(itws->first,itws->second);
    //if(debug) {
    wcerr<<L"wordcount("<<itws->first<<L") = "<<itws->second<<L"\n";
    //}
  }

  //All co-occurrences have been collected. We need to filter them
  //so as to take into account only the n most frequent
  for(itword=words2workwith.begin(); itword!=words2workwith.end(); itword++) {
    set<wstring> lexical_choices=lexical_choices_of_word[*itword];
    set<wstring>::iterator itlch;
    for(itlch=lexical_choices.begin(); itlch!=lexical_choices.end(); itlch++) {
      PairStringCountComparer comparer;
      vector<pair<wstring, COUNT_DATA_TYPE> > context_v;
      map<wstring, COUNT_DATA_TYPE>::iterator itm;

      while(context[*itlch].size()>0) {
	itm=context[*itlch].begin();
	//wcerr<<itm->first<<L" "<<itm->second<<L"\n";
	context_v.push_back(*itm);
	context[*itlch].erase(itm);
      }
    
      sort(context_v.begin(), context_v.end(), comparer);
      wstring lch=*itlch;
      lextor_data->set_cooccurrence_context(lch, context_v);
      //lextor_data->set_lexchoice_sum(lch, tlwordmodel.get_lexchoice_sum(lexchoice_translation[lch]));

      //wcerr<<L"lexchoice_sum("<<lch<<L") = lexchoice_sum_target("<<lexchoice_translation[lch]<<L") ="
      //    <<tlwordmodel.get_lexchoice_sum(lexchoice_translation[lch])<<L"\n";
    }
  } 

  //Set the count of each lexical choice
  map<wstring, COUNT_DATA_TYPE>::iterator itlcs;
  for(itlcs=lechsum.begin(); itlcs!=lechsum.end(); itlcs++) {
    lextor_data->set_lexchoice_sum(itlcs->first,itlcs->second);
    //if(debug) {
    wcerr<<L"lexchoice_sum("<<itlcs->first<<L") = "<<itlcs->second<<L"\n";
    //}
  }


  wcerr<<L"Training done\n"; 
}

void 
LexTor::lexical_selector(wistream& is, FSTProcessor &fstp, int left, int right, double weigth_exponent, LexTorEval* lteval) {
  if (lextor_data==NULL) {
    wcerr<<L"Error in LexTor::lexical_selector, you must call set_lextor_data before\n";
    exit(EXIT_FAILURE);
  }

  deque<LexTorWord> buffer;
  deque<LexTorWord> window;

  LexTorWord  nullword(L"NULLWORD", &fstp);

  for(int i=0; i<(left+right+1); i++)
    window.push_back(nullword);

  int retain=0;

  LexTorWord* ltword=NULL;
  ltword=LexTorWord::next_word(is, &fstp);

  while(ltword) {
    //wcerr<<L"Word read: "<<ltword->get_word_string()
    //<<L", reduced: "<<lextor_data->reduce(ltword->get_word_string())<<L" ";
    //wcerr<<L"# lexical choices: "<<ltword->n_lexical_choices()<<L"\n";

    if (!lextor_data->is_stopword(lextor_data->reduce(ltword->get_word_string()))) { 
      if (window.size()>=(unsigned)(left+1+right)) 
	window.pop_front();
      
      window.push_back(*ltword);

      if (ltword->n_lexical_choices()>1) {
	retain++;
	if (retain>1)
	  buffer.push_back(*ltword);
      } else {
	if (retain>0) 
	  buffer.push_back(*ltword);
	else {
	  wcout<<ltword->get_lexical_choice(-1,true);
	  if (lteval) 
	    lteval->evalword(*ltword, -1, lextor_data);
	}
      }

      if (window[left].n_lexical_choices()>1) {

	if (debug) {
	  wcerr<<L"WINDOW: ";
	  for(int i=0; i<(int)window.size(); i++) {
	    if(i==left)
	      wcerr<<L"[>>>>"<<window[i].get_word_string()<<L"<<<<] ";
	    else
	      wcerr<<L"["<<window[i].get_word_string()<<L"] ";
	  }
	  wcerr<<L"\n";
	  wcerr<<L"BUFFER: ";
	  for(int i=0; i<(int)buffer.size(); i++)
	    wcerr<<L"["<<buffer[i].get_word_string()<<L"] ";
	  wcerr<<L"\n\n";
  
	}

	int winner=estimate_winner_lch(window, left, weigth_exponent);

	wcout<<window[left].get_lexical_choice(winner,true);
	if (lteval) 
	  lteval->evalword(window[left], winner, lextor_data);
	
	//For debug
	/*
	  cout<<L" | ";
	  for(int j=0; j<window[left].n_lexical_choices(); j++) {
	  if (j>0)
	  cout<<L"|";
	  cout<<window[left].get_lexical_choice(j,false);
	  }
	  cout<<L"\n";
	*/

	//Now those words that were retained must be released
	if(retain>0) {
	  while ((buffer.size()>0)&&(buffer[0].n_lexical_choices()==1)) {
	    wcout<<buffer[0].get_lexical_choice(-1,true);
	    if (lteval) 
	      lteval->evalword(buffer[0], -1, lextor_data);
	    buffer.pop_front();
	  }
	  if ((buffer.size()>0)&&(buffer[0].n_lexical_choices()>1))
	    buffer.pop_front(); 

	  retain--;
	}
      } 
    } else { //It's a stopword
      if (retain>0) 
	buffer.push_back(*ltword);
      else {
	wcout<<ltword->get_lexical_choice(-1,true);
	if (lteval) 
	  lteval->evalword(*ltword, -1, lextor_data);
      }
    }

    delete ltword;
    ltword=LexTorWord::next_word(is, &fstp);
  }

  if (retain>0) {
    for(unsigned i=left+1; i<window.size(); i++) {
      if (window[i].n_lexical_choices()>1) {
	int winner=estimate_winner_lch(window, i, weigth_exponent);

	wcout<<window[i].get_lexical_choice(winner,true);
	if (lteval)
	  lteval->evalword(window[i], winner, lextor_data);

	//For debug
	/*
	  cout<<L" | ";
	  for(int j=0; j<window[i].n_lexical_choices(); j++) {
	  if (j>0)
	  cout<<L"|";
	  cout<<window[i].get_lexical_choice(j,false);
	  }
	  cout<<L"\n";
	*/

	//Now those words that were retained must be released
	if(retain>0) {
	  while ((buffer.size()>0)&&(buffer[0].n_lexical_choices()==1)) {
	    wcout<<buffer[0].get_lexical_choice(-1,true);
	    if (lteval) 
	      lteval->evalword(buffer[0], -1, lextor_data);
	    buffer.pop_front();
	  }
	  if ((buffer.size()>0)&&(buffer[0].n_lexical_choices()>1))
	    buffer.pop_front(); 

	  retain--;
	}

      }
    }
  }

  //wcerr<<L"retain: "<<retain<<L"\n";
}

int 
LexTor::estimate_winner_lch(deque<LexTorWord>& window, int word_index, double weigth_exponent) {
  //return estimate_winner_lch_cosine(window, word_index, weigth_exponent);
  return estimate_winner_lch_voting(window, word_index, weigth_exponent);
  //return estimate_winner_lch_mostprob(window, word_index, weigth_exponent);
  //return estimate_winner_lch_votingtl(window, word_index, weigth_exponent);
  //return -1;
}

int 
LexTor::estimate_winner_lch_voting(deque<LexTorWord>& window, int word_index, double weigth_exponent) {
  vector <double> lexchoices_count(window[word_index].n_lexical_choices());

  if (debug) {
    wcerr<<L"WINDOW: ";
    for(unsigned i=0; i<window.size(); i++) {
      if (i==(unsigned)word_index)
	wcerr<<L"[>>>>"<<lextor_data->reduce(window[i].get_word_string())<<L"<<<<] ";
      else
	wcerr<<L"["<<lextor_data->reduce(window[i].get_word_string())<<L"] ";
    }
    wcerr<<L"\n";
  }

  //
  double sum_lexchoices=0.0;
  for(int i=0; i<window[word_index].n_lexical_choices(); i++) {
    double aux_lexchoice_sum=lextor_data->get_lexchoice_sum(lextor_data->reduce_lexical_choice(window[word_index].get_lexical_choice(i,false)));
    sum_lexchoices+=aux_lexchoice_sum;
    if (debug) {
      wcerr<<L"lexchoice_sum("<<lextor_data->reduce_lexical_choice(window[word_index].get_lexical_choice(i,false))<<L") = "<<aux_lexchoice_sum<<L"\n";
    }
  }
  double wordcount=lextor_data->get_wordcount(lextor_data->reduce(window[word_index].get_word_string()));
  if (debug) {
    wcerr<<L"wordcount("<<lextor_data->reduce(window[word_index].get_word_string())<<L") = "<<wordcount<<L"\n";
  }
  //

  for(int i=0; i<window[word_index].n_lexical_choices(); i++) {
    lexchoices_count[i]=0;
    wstring reduced_lexchoice=lextor_data->reduce_lexical_choice(window[word_index].get_lexical_choice(i,false));
    if (debug) {
      wcerr<<L"lexical choice: "<<window[word_index].get_lexical_choice(i)<<L" reduced: "<<reduced_lexchoice<<L"\n";
    }

    int distance=(-1)*(word_index);
    for(int j=0; j<(int)window.size(); j++) { 
      //For all words in the context window
      if(j!=word_index) {
	COUNT_DATA_TYPE vote=0;

	wstring reduced_word=lextor_data->reduce(window[j].get_word_string());

	if (lextor_data->get_wordcount(reduced_word)>0) {
	  vote=lextor_data->vote_from_word(reduced_lexchoice, reduced_word)/
	    (((lextor_data->get_lexchoice_sum(reduced_lexchoice))/sum_lexchoices)*wordcount);

	  lexchoices_count[i]+=vote/pow(fabs((double)distance),weigth_exponent);
	}

	if (debug) {
	  wcerr<<L"Count for "<<reduced_lexchoice<<L" from "<<reduced_word<<L" is "<<vote<<L"\n";
	  wcerr<<L"Vote: "<<lextor_data->vote_from_word(reduced_lexchoice, reduced_word)<<L" word count: "
	      <<lextor_data->get_wordcount(reduced_word)<<L"\n";
	  wcerr<<L"["<<reduced_word<<L"] DISTANCE: "<<distance<<L", ";
	  wcerr<<L" Count to apply: "<<vote/pow(fabs((double)distance),weigth_exponent)<<L"\n";
	}
      }
      distance++;
    }
  }

  //Now the winner is calculated
  int winner=-1; //This will make the default one to be used if unchanged
  COUNT_DATA_TYPE winner_vote=-100000000;
  for(int i=0; i<window[word_index].n_lexical_choices(); i++) {
    if ((lexchoices_count[i]>0) && (lexchoices_count[i]>winner_vote)) {
      winner_vote=lexchoices_count[i];
      winner=i;
    } 
    /*
      else if ((lexchoices_count[i]>0) && (lexchoices_count[i]==winner_vote)) {
      //Take the most probable one, the one with the highest sum
      COUNT_DATA_TYPE sum_i=lextor_data->get_lexchoice_sum(lextor_data->reduce(window[word_index].get_lexical_choice(i)));
      COUNT_DATA_TYPE sum_win=lextor_data->get_lexchoice_sum(lextor_data->reduce(window[word_index].get_lexical_choice(winner)));
      if (sum_i>sum_win)
      winner=i;
      }
    */
  }
  
  if (debug) {
    wcerr<<L"WINNER: "<<winner<<L" "<<window[word_index].get_lexical_choice(winner)<<L"\n";
  }
  return winner;
}

int 
LexTor::estimate_winner_lch_mostprob(deque<LexTorWord>& window, int word_index,  double weigth_exponent) {
  int winner=-1;
  double greatest_sum=-1;
  for(int i=0; i<window[word_index].n_lexical_choices(); i++) {
    wstring reduced_lexchoice=lextor_data->reduce_lexical_choice(window[word_index].get_lexical_choice(i,false));
    double sumlch=lextor_data->get_lexchoice_sum(reduced_lexchoice);


    if (debug) {
      wcerr<<L"sum("<<reduced_lexchoice<<L") = "<<sumlch<<L"\n";
    }

    if (sumlch>greatest_sum) {
      greatest_sum=sumlch;
      winner=i;
    }
  }

  if (greatest_sum==0)
    winner=-1;

  if (debug) 
    wcerr<<L"WINNER: "<<winner<<L" "<<window[word_index].get_lexical_choice(winner)<<L"\n";

  return winner;
}

int 
LexTor::estimate_winner_lch_cosine(deque<LexTorWord>& window, int word_index, double weigth_exponent) {
  map<wstring, double> vcontext;

  int distance=(-1)*(word_index);
  for(int i=0; i<(int)window.size(); i++) {
    if (i!=word_index) {
      wstring reduced_word=lextor_data->reduce(window[i].get_word_string());
      vcontext[reduced_word]+=1.0/pow(fabs((double)distance),weigth_exponent);
    }
    distance++;
  }

  if (debug) {
    wcerr<<L"CONTEXT VECTOR\n-------------------\n";
    map<wstring, double>::iterator it;
    for(it=vcontext.begin(); it!=vcontext.end(); it++)
      wcerr<<it->first<<L", "<<it->second<<L"\n";
  }

  ////double max_cosine=-2;
  double min_angle=360;
  int winner=-1;
  double diff_angle=-1;
  for(int i=0; i<window[word_index].n_lexical_choices(); i++) {
    wstring reduced_lexchoice=lextor_data->reduce_lexical_choice(window[word_index].get_lexical_choice(i,false));

    double aux_cosine=cosine(vcontext, reduced_lexchoice);
    double aux_angle=(acos(aux_cosine)*180)/PI;
    if (debug) {
      wcerr<<L"cos("<<lextor_data->reduce(window[word_index].get_word_string())<<L", "
	  <<reduced_lexchoice<<L") = "<<aux_cosine<<L"; ang = "<<aux_angle<<L" grades\n";
    }

    if (aux_angle<min_angle) {
      if (min_angle!=360) {
	diff_angle=min_angle-aux_angle;
      }
      min_angle=aux_angle;
      winner=i;
    } else if ((min_angle!=360)&&(diff_angle==-1)) {
      diff_angle=fabs(min_angle-aux_angle);
    }


    /*
      if (aux_cosine>max_cosine) {
      diff_angle=abs(min_angle-aux_angle);
      winner=i;
      max_cosine=aux_cosine;
      min_angle=aux_angle;
      }
    */
  }

  if (debug) {
    wcerr<<L"DIFF ANGLE: "<<diff_angle<<L"\n";
  }
  if (diff_angle<=angleth)
    winner=-1;

  if (debug) 
    wcerr<<L"WINNER: "<<winner<<L" "<<window[word_index].get_lexical_choice(winner)<<L"\n";
  
  return winner;
}

int 
LexTor::estimate_winner_lch_votingtl(deque<LexTorWord>& window, int word_index, double weigth_exponent) {
  if (tlmodel==NULL) {
    wcerr<<L"Error in LexTor::estimate_winner_lch_votingtl: you must call LexTor::set_tlmodel first.\n";
    exit(EXIT_FAILURE);
  }  

  vector <double> lexchoices_count(window[word_index].n_lexical_choices());
  vector <vector <wstring> > translation_window (window.size());
  vector <wstring> reduced_window(window.size());

  for (unsigned i=0; i<window.size(); i++) 
    reduced_window[i]=lextor_data->reduce(window[i].get_word_string());	      
  
  if(debug) {
    wcerr<<L"WINDOW: ";
    for (unsigned i=0; i<window.size(); i++) {
      if(i==(unsigned)word_index)
	wcerr<<L"[>>>>"<<reduced_window[i]<<L"<<<<] ";
      else
	wcerr<<L"["<<reduced_window[i]<<L"] ";
    }
    wcerr<<L"\n";
    wcerr<<L"TRANSLATED: ";
  }

  //We translate each word in the context
  //Note: Words in the context can also be ambiguous (with more than one lexical choice)
  for (unsigned i=0; i<window.size(); i++) {
    wstring str_translations=L"";
    for(int j=0; j<window[i].n_lexical_choices(); j++) {
      wstring tr=tlmodel->reduce(window[i].translate(*fstpbil,j));
      translation_window[i].push_back(tr);
      str_translations+=tr+L"/";
    }
    if (debug) {
      if (i==(unsigned)word_index)
	wcerr<<L"[>>>>"<<str_translations<<L"<<<<] ";
      else
	wcerr<<L"["<<str_translations<<L"] ";
    }
  }

  if(debug)
    wcerr<<L"\n";

  //For each lexical choice the counts from the TL are collected
  for(unsigned i=0; i<(unsigned)window[word_index].n_lexical_choices(); i++) {
    lexchoices_count[i]=0;

    for (unsigned k=0; k<window.size(); k++) {
      if ((k!=(unsigned)word_index)&&(reduced_window[k]!=reduced_window[word_index])) {
	COUNT_DATA_TYPE target_vote=0;

	//If the SL word is ambiguous it will have more than one
	//translation into TL, so we need to normalize using the
	//frequency of words in the TL
    vector <double> translation_weighs(translation_window[k].size());
	double sum=0.0;
	if (translation_window[k].size()>1) {
	  for(unsigned j=0; j<translation_window[k].size(); j++) {
	    translation_weighs[j]=tlmodel->get_lexchoice_sum(translation_window[k][j]);
	    sum+=translation_weighs[j];

	    //!!!!! Para hacer que no tenga en cuenta las
	    //!!!!! polisemicas del contexto
	    ///////translation_weighs[j]=0;
	    //!!!!!
	    //!!!!!

	    if (debug) {
	      wcerr<<L"Frequency of translation ["<<translation_window[k][j]<<L"] ="
		  <<translation_weighs[j]<<L"\n";
	    }
	  }
	} else {
	  translation_weighs[0]=1;
	  sum=1;
	}

	for(unsigned j=0; j<translation_window[k].size(); j++) {
	  translation_weighs[j]=translation_weighs[j]/sum;
	  if (debug) {
	    wcerr<<L"Weight of translation ["<<translation_window[k][j]<<L"] ="
		<<translation_weighs[j]<<L"\n";
	  }
	}

	for(unsigned j=0; j<translation_window[k].size(); j++) {
	  double aux_vote=0;
	  //aux_vote=tlwordmodel.vote_from_word(lexchoice_translation[*itlch], 
	  //				    translation_buffer[i][j])*translation_weighs[j];
	  if(debug) 
	    wcerr<<translation_window[word_index][i]<<L" "<<translation_window[k][j]<<L" "
		<<tlmodel->vote_from_word(translation_window[word_index][i],translation_window[k][j])<<L" "
		<<tlmodel->get_wordcount(translation_window[k][j])<<L" "<<translation_weighs[j]<<L"\n";
	
	  if (tlmodel->get_wordcount(translation_window[k][j])>0) {
	    aux_vote=(tlmodel->vote_from_word(translation_window[word_index][i],translation_window[k][j])/
		      tlmodel->get_wordcount(translation_window[k][j]))*translation_weighs[j];
	  } 
	  target_vote+=aux_vote;

	  if(debug) {
	    wcerr<<L"Target vote for ["<<translation_window[word_index][i]
		<<L"] from ["<<translation_window[k][j]<<L"] = "<<aux_vote<<L"\n";
	  }
	}

	lexchoices_count[i]+=target_vote;
      }
    }
  }


  if(debug) {
    for(int i=0; i<window[word_index].n_lexical_choices(); i++) 
      wcerr<<L"lexchoicecount["<<i<<L"] = "<<lexchoices_count[i]<<L"\n";
    //getchar();
  }

  //Now the winner is calculated
  int winner=-1; //This will make the default one to be used if unchanged
  COUNT_DATA_TYPE winner_vote=-100000000;
  for(int i=0; i<window[word_index].n_lexical_choices(); i++) {
    if ((lexchoices_count[i]>0) && (lexchoices_count[i]>winner_vote)) {
      winner_vote=lexchoices_count[i];
      winner=i;
    } 
  }

  if (debug) 
    wcerr<<L"WINNER: "<<winner<<L" "<<window[word_index].get_lexical_choice(winner)<<L"\n";

  return winner;
}

double 
LexTor::cosine(map<wstring, double>& vcontext, const wstring& reduced_lexchoice) {
  map<wstring, double>::iterator itc;

  //We calculate the scalar product between vcontext and the lexchoice vector
  double scalar_product=0;
  for(itc=vcontext.begin(); itc!=vcontext.end(); itc++) {
    scalar_product+=(itc->second)*(lextor_data->vote_from_word(reduced_lexchoice, itc->first));
  }

  //We calculate the module of vcontext, ||vcontext||
  double module_vcontext=0;
  for(itc=vcontext.begin(); itc!=vcontext.end(); itc++) {
    module_vcontext+=(itc->second)*(itc->second);
  }
  module_vcontext=sqrt(module_vcontext);

  //We get the module of the lexchoice vector, ||lexchoice vector||
  double module_lexchoice_vector=lextor_data->get_module_lexchoice_vector(reduced_lexchoice);

  if (module_vcontext==0) { 
    wcerr<<L"Error in LexTor::vectors_cosine: module_vcontext is equal to zero.\n"
	<<L"The cosine cannot be computed\n";
    if (debug) {
      wcerr<<L"CONTEXT VECTOR\n-------------------\n";
      map<wstring, double>::iterator it;
      for(it=vcontext.begin(); it!=vcontext.end(); it++)
	wcerr<<it->first<<L", "<<it->second<<L"\n";
    }

    return -2;
    //exit(EXIT_FAILURE);
  }

  if (module_lexchoice_vector==0) {
    if (debug) {
      wcerr<<L"Warning in LexTor::vectors_cosine: module_lexchoice_vector is equal to zero.\n"
	  <<L"The cosine cannot be computed\n";
      wcerr<<L"reduced lexical choice: "<<reduced_lexchoice<<L"\n";
    }
    return -2;
  }

  return scalar_product/(module_vcontext*module_lexchoice_vector);
}
