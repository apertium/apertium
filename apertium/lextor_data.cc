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

#include <apertium/lextor_data.h>
#include <apertium/string_utils.h>
#include <apertium/lextor_word.h>
#include <apertium/lextor.h>

#include <lttoolbox/compression.h>
#include <apertium/endian_double_util.h>
#include <apertium/string_utils.h>

using namespace Apertium;
LexTorData::LexTorData() {
  n_stopwords=0;
  n_words=0;
  n_words_per_set=0;
  n_set=0;

  index2word.push_back(NULLWORD);
  word2index[NULLWORD]=0;
  n_words++;
}
  
LexTorData::LexTorData(const LexTorData& ltd) {
  n_stopwords=ltd.n_stopwords;
  n_words=ltd.n_words;
  n_words_per_set=ltd.n_words_per_set;
  n_set=ltd.n_set;

  word2index=ltd.word2index;
  index2word=ltd.index2word;

  lexchoice_set=ltd.lexchoice_set;
  lexchoice_sum=ltd.lexchoice_sum;
  //lexchoice_prob=ltd.lexchoice_prob;

  stopwords=ltd.stopwords;
  words=ltd.words;
  lexical_choices=ltd.lexical_choices;
  reduced_lexical_choices=ltd.reduced_lexical_choices;
}
  
LexTorData::~LexTorData() {
}

COUNT_DATA_TYPE
LexTorData::vote_from_word(const wstring& lexical_choice, const wstring& word) {
  WORD_DATA_TYPE ind_lexchoice=word2index[StringUtils::tolower(lexical_choice)];
  WORD_DATA_TYPE ind_word=word2index[StringUtils::tolower(word)];

  //To avoid creating a null entry in lexchoice_set[lexical_choice]
  if (lexchoice_set[ind_lexchoice].find(ind_word)==lexchoice_set[ind_lexchoice].end())
    return 0;
  else
    return lexchoice_set[ind_lexchoice][ind_word];
}

//double 
//LexTorData::get_lexchoice_prob(const string& lexical_choice) {
//  return lexchoice_prob[word2index[lexical_choice]];
//}


void 
LexTorData::set_wordcount(const wstring& word, COUNT_DATA_TYPE c) {
  WORD_DATA_TYPE ind_word=word2index[StringUtils::tolower(word)];
  wordcount[ind_word]=c;
}

COUNT_DATA_TYPE 
LexTorData::get_wordcount(const wstring& word) {
  WORD_DATA_TYPE ind_word=word2index[StringUtils::tolower(word)];

  if (wordcount.find(ind_word)==wordcount.end())
    return 0;
  else
    return wordcount[ind_word];
}

COUNT_DATA_TYPE
LexTorData::get_lexchoice_sum(const wstring& lexical_choice) {
  return lexchoice_sum[word2index[StringUtils::tolower(lexical_choice)]];
}

void 
LexTorData::set_lexchoice_sum(const wstring& lexical_choice, COUNT_DATA_TYPE sum) {
  lexchoice_sum[word2index[StringUtils::tolower(lexical_choice)]]=sum;
}

bool
LexTorData::is_stopword(const wstring& word) {
  return (stopwords.find(StringUtils::tolower(word))!=stopwords.end());
}

void 
LexTorData::read(FILE *is) {
  //wcerr<<"LexTorData::read------------------------------------\n";
  n_stopwords=(WORD_DATA_TYPE)Compression::multibyte_read(is);
  n_words=(WORD_DATA_TYPE)Compression::multibyte_read(is);
  n_words_per_set=(WORD_DATA_TYPE)Compression::multibyte_read(is);
  n_set=(WORD_DATA_TYPE)Compression::multibyte_read(is);

  //wcerr<<n_stopwords<<"\n";
  //wcerr<<n_words<<"\n";
  //wcerr<<n_words_per_set<<"\n";
  //wcerr<<n_set<<"\n";


  //Read the set of stopwords
  //wcerr<<"stopwords--------------------------------------------\n";
  for (unsigned int i=0; i<n_stopwords; i++) {
    stopwords.insert(Compression::wstring_read(is));
    //wcerr<<"len: "<<len<<" str: "<<str<<"\n";
  }

  //Read the list of words
  //wcerr<<"list of words----------------------------------------\n";
  for(unsigned int i=1; i<n_words; i++) {
    wstring str = Compression::wstring_read(is);
    index2word.push_back(str);
    word2index[str]=i;
    wordcount[i]=EndianDoubleUtil::read(is);
    //wcerr<<"len: "<<len<<" str: "<<str<<" index: "<<i<<" word_count: "<<wordcount[i]<<"\n";
  }

  //Read data of each set associate to each lexical choice (or word)
  for(unsigned int i=0; i<n_set; i++) {
    WORD_DATA_TYPE lexchoice;
    COUNT_DATA_TYPE sum;
    //double prob;

    lexchoice=(WORD_DATA_TYPE)Compression::multibyte_read(is);
    sum=EndianDoubleUtil::read(is);

    //wcerr<<"lexchoice: "<<lexchoice<<" sum: "<<sum<<" "<<index2word[lexchoice]<<"\n";

    reduced_lexical_choices.insert(index2word[lexchoice]);

    lexchoice_sum[lexchoice]=sum;
    //lexchoice_prob[lexchoice]=prob;

    /////lexical_choices.insert(index2word[lexchoice]);

    for(unsigned int j=0; j<n_words_per_set; j++) {
      WORD_DATA_TYPE word;
      COUNT_DATA_TYPE count;

      word=(WORD_DATA_TYPE)Compression::multibyte_read(is);
      count=EndianDoubleUtil::read(is);
      //wcerr<<"     word: "<<word<<" count: "<<count<<"\n";
      lexchoice_set[lexchoice][word]=count;
    }
  }

  //First we read the number of words to take into account
  WORD_DATA_TYPE nwords2workwith;

  nwords2workwith=(WORD_DATA_TYPE)Compression::multibyte_read(is);
  for (unsigned int i=0; i<nwords2workwith; i++) {
    WORD_DATA_TYPE word;

    word=(WORD_DATA_TYPE)Compression::multibyte_read(is);
    words.insert(index2word[word]);
    //wcerr<<"word: "<<index2word[word]<<"\n";
  }
}

void 
LexTorData::write(FILE *os) {
  //wcerr<<"LexTorData::write------------------------------------\n";
  //wcerr<<n_stopwords<<"\n";
  //wcerr<<n_words<<"\n";
  //wcerr<<n_words_per_set<<"\n";
  //wcerr<<n_set<<"\n";
  Compression::multibyte_write(n_stopwords, os);
  Compression::multibyte_write(n_words, os);
  Compression::multibyte_write(n_words_per_set, os);
  Compression::multibyte_write(n_set, os);

  //Write the set of stopwords
  //wcerr<<"stopwords--------------------------------------------\n";
  set<wstring>::iterator it;
  for (it=stopwords.begin(); it!=stopwords.end(); it++) {
    Compression::wstring_write(*it, os);
  }

  //Write the list of words
  //wcerr<<"list of words----------------------------------------\n";
  for(unsigned int i=1; i<index2word.size(); i++) {
    Compression::wstring_write(index2word[i], os);
    EndianDoubleUtil::write(os, wordcount[i]);
  }

  //Write data of each set associate to each lexical choice (or word)
  map<WORD_DATA_TYPE, map<WORD_DATA_TYPE, COUNT_DATA_TYPE> >::iterator it_lch_set;
  map<WORD_DATA_TYPE, COUNT_DATA_TYPE>::iterator it_w_lch_set;
  //map<WORD_DATA_TYPE, double>::iterator it_lch_prob;

  for(it_lch_set=lexchoice_set.begin(); it_lch_set!=lexchoice_set.end(); it_lch_set++) {
    WORD_DATA_TYPE lexchoice=it_lch_set->first;
    COUNT_DATA_TYPE sum=lexchoice_sum[lexchoice];
    //double prob=lexchoice_prob[lexchoice];

    //wcerr<<"lexchoice: "<<lexchoice<<" sum: "<<sum<<" "<<index2word[lexchoice]<<"\n";
    Compression::multibyte_write(lexchoice, os);    
    //os.write(reinterpret_cast<char * const> (&prob), sizeof(double));
    EndianDoubleUtil::write(os, sum);

    int nwritten_words=0;
    for(it_w_lch_set=it_lch_set->second.begin(); 
        it_w_lch_set!=it_lch_set->second.end(); 
        it_w_lch_set++) {
      WORD_DATA_TYPE word=it_w_lch_set->first;
      COUNT_DATA_TYPE count=it_w_lch_set->second;
      //wcerr<<"     word: "<<word<<" count: "<<count<<"\n";
      Compression::multibyte_write(word, os);
      EndianDoubleUtil::write(os, count);
      nwritten_words++;
    }

    //If there were less written words than expected
    while (nwritten_words<n_words_per_set){
      WORD_DATA_TYPE word=word2index[NULLWORD];
      COUNT_DATA_TYPE count=0;
      //wcerr<<"     word: "<<word<<" count: "<<count<<"\n";
      Compression::multibyte_write(word, os);
      EndianDoubleUtil::write(os, count);
      nwritten_words++;
    }
  }

  //First we write the number of words to take into account
  WORD_DATA_TYPE nwords2workwith=words.size();
  Compression::multibyte_write(nwords2workwith, os);

  set<wstring>::iterator sit;
  for(sit=words.begin(); sit!=words.end(); sit++) {
    WORD_DATA_TYPE word=word2index[*sit];
    Compression::multibyte_write(word, os);
    //wcerr<<"word: "<<*sit<<"\n";
  }
}

void 
LexTorData::read_stopwords(wistream& is) {
  while (!is.eof()) {
    wstring w;
    getline(is,w);
    w=StringUtils::tolower(w);
    if (w.length()>0) {
      stopwords.insert(w);
      wcerr<<L"stopword: "<<w<<L"\n";
    }
  }
  n_stopwords=stopwords.size();
  wcerr<<L"# stopwords read from file: "<<n_stopwords<<L"\n";
}  

void 
LexTorData::read_words(wistream& is) {
  while(!is.eof()) {
    wstring w;
    getline(is,w);
    w=StringUtils::tolower(w);
    if (w.length()>0) {
      words.insert(w);
      new_word_register(w);
    }
  }
  n_set=words.size();
  wcerr<<L"# words: "<<n_set<<L"\n";
}

void 
LexTorData::read_lexical_choices(FSTProcessor& fstp) {
  set<wstring>::iterator it;
  int nlexchoices=0;

  for(it=words.begin(); it!=words.end(); it++) {
    LexTorWord ambiguousword(*it, &fstp);
    nlexchoices+=ambiguousword.n_lexical_choices();

    for(int i=0; i<ambiguousword.n_lexical_choices(); i++) {
      lexical_choices[*it].insert(ambiguousword.get_lexical_choice(i,false));
      //lexical_choices[*it].insert(reduce_lexical_choice(ambiguousword.get_lexical_choice(i,false)));
    }
  }

  n_set=nlexchoices;

  wcerr<<L"# lexical choices: "<<n_set<<L"\n";
}

set<wstring>
LexTorData::get_words() {
  return words;
}

set<wstring> 
LexTorData::get_lexical_choices(const wstring& word) {
  return lexical_choices[StringUtils::tolower(word)];
}

void 
LexTorData::set_nwords_per_set(int i){
  n_words_per_set=i;
  wcerr<<L"# words per co-ocurrence model: "<<n_words_per_set<<L"\n";
}

void 
LexTorData::set_cooccurrence_context(const wstring& lexical_choice, 
                                     const vector<pair<wstring, COUNT_DATA_TYPE> >& context) {
  wcerr<<L"Co-occurrence model for lexical_choice/word: "<<lexical_choice<<L"\n";

  if (context.size()==0) {
    wcerr<<L"Warning: co-occurrence model for lexical_choice/word: "<<lexical_choice<<L" is empty\n";
    wcerr<<L"It seems that training corpus is too small or thematically homogeneous\n";
    n_set--;
  }

  new_word_register(lexical_choice);

  for (unsigned int i=0; ((i<n_words_per_set)&&(i<context.size())); i++) {
    wcerr<<context[i].first<<L" "<<context[i].second<<L"\n";

    new_word_register(context[i].first);

    lexchoice_set[word2index[StringUtils::tolower(lexical_choice)]][word2index[StringUtils::tolower(context[i].first)]]=context[i].second;

    //////wordcount[word2index[StringUtils::tolower(context[i].first)]]+=context[i].second;
  }
}

void
LexTorData::ensure_stopwords_ok() {
  set<wstring>::iterator its, itw;
  set<wstring> swaux;

  //Notice that stopwords consist of lemma and first tag while words
  //consist of lemma and one (the first one) or more tags

  for(its=stopwords.begin(); its!=stopwords.end(); its++) {
    bool is_ok=true;
    for(itw=words.begin(); itw!=words.end(); itw++) {
      //wcerr<<"sw: "<<*its<<" w: "<<*itw<<"\n";
      if (itw->find(*its)==0) {
	wcerr<<L"Warning: Word '"<<*itw<<L"' for which co-ocurrence models will"
	    <<L" be estimated is also a stopword. ";
	wcerr<<L"Removing it from the stopwords list\n";
	is_ok=false;
	break;
      }
    }
    if(is_ok)
      swaux.insert(*its);
  }

  stopwords=swaux;

  wcerr<<n_stopwords-stopwords.size()<<L" stopwords were discarded\n";

  n_stopwords=stopwords.size();

  wcerr<<L"# stopwords finally taken into account: "<<n_stopwords<<L"\n";
}

wstring 
LexTorData::reduce(const wstring& s) {
  wstring str;

  if ((s.length()>0) && (s[0]=='^') && (s[s.length()-1]=='$'))
    str=StringUtils::tolower(s.substr(1, s.length()-1));
  else
    str=StringUtils::tolower(s);

  set<wstring>::iterator it;
  for(it=words.begin(); it!=words.end(); it++) {
    if (str.find(*it)==0) {
      return (*it);
    }
  }

  unsigned int p=str.find(L">");
  unsigned int i=0;
  if (p==static_cast<unsigned int>(wstring::npos)) { //s could correspond to an unknown word
    p=str.length();
    if ((str.length()>0) && (str[0]=='*'))
      i=1; // to remove the star (unknown word mark)
  }
  else
    p++;
  
  if (i>=p) {
    wcerr<<L"Warning in LexTorData::reduce: input string: '"<<s<<L"', string after operation: '"<<str<<L"'\n";
    wcerr<<L"begin index: "<<i<<", end index: "<<p<<L"\n";
    i=0;
  }

  return str.substr(i,p);
}

wstring 
LexTorData::reduce_lexical_choice(const wstring& s) {
  wstring str;

  if ((s.length()>0) && (s[0]=='^') && (s[s.length()-1]=='$'))
    str=StringUtils::tolower(s.substr(1, s.length()-1));
  else
    str=StringUtils::tolower(s);

  set<wstring>::iterator it;
  for(it=reduced_lexical_choices.begin(); it!=reduced_lexical_choices.end(); it++) {
    if (str.find(*it)==0) {
      return (*it);
    }
  }

  //return StringUtils::substitute(str," d<", " D<");

  return str;
}

void 
LexTorData::new_word_register(const wstring& word) {
  wstring w=StringUtils::tolower(word);

  if (word2index.find(w)==word2index.end()) {
    index2word.push_back(w);
    int ind=index2word.size()-1;
    if (ind>MAX_WORD_INDEX) {
      wcerr<<L"Error: The number of words to be considered is greater that the maximum allowed by\n";
      wcerr<<L"the data type used to store words\n";
      wcerr<<L"Edit source file LexTorData.H and change the WORD_DATA_TYPE define\n";
      exit(EXIT_FAILURE);
    }
    word2index[w]=(WORD_DATA_TYPE)ind;
    n_words=index2word.size();
    wordcount[(WORD_DATA_TYPE)ind]=0;
  }
}

/*
vector<pair<WORD_DATA_TYPE, double> > 
LexTorData::get_cooccurrence_vector(const string& lexical_choice) {
  vector<pair<WORD_DATA_TYPE, double> > v;
  WORD_DATA_TYPE ind_lexchoice=word2index[StringUtils::tolower(lexical_choice)];
  map<WORD_DATA_TYPE, COUNT_DATA_TYPE>::iterator it;

  for(it=lexchoice_set[ind_lexchoice].begin(); it!= lexchoice_set[ind_lexchoice].end(); it++) 
    v.push_back(*it);
  
  return v;
}
*/


double 
LexTorData::get_module_lexchoice_vector(const wstring& lexical_choice) {
  WORD_DATA_TYPE ind_lexchoice=word2index[StringUtils::tolower(lexical_choice)];
  map<WORD_DATA_TYPE, COUNT_DATA_TYPE>::iterator it;

  double module=0;

  for(it=lexchoice_set[ind_lexchoice].begin(); it!= lexchoice_set[ind_lexchoice].end(); it++) 
    module+=(it->second)*(it->second);

  module=sqrt(module);

  return module;
}

double 
LexTorData::cosine(const wstring& reduced_lexch1, const wstring& reduced_lexch2) {
  WORD_DATA_TYPE ind_lexchoice1=word2index[StringUtils::tolower(reduced_lexch1)];
  WORD_DATA_TYPE ind_lexchoice2=word2index[StringUtils::tolower(reduced_lexch2)];
  map<WORD_DATA_TYPE, COUNT_DATA_TYPE>::iterator it;

  //We calculate the scalar product
  double scalar_product=0;
  for(it=lexchoice_set[ind_lexchoice1].begin(); it!= lexchoice_set[ind_lexchoice1].end(); it++) {
    if (lexchoice_set[ind_lexchoice2].find(it->first)!=
        lexchoice_set[ind_lexchoice2].end()) {
      scalar_product+=(it->second)*lexchoice_set[ind_lexchoice2][it->first];
    }
  }

  //We get the module of the lexchoice vectors, ||lexchoice vector||
  double module_lexch1_vector=get_module_lexchoice_vector(reduced_lexch1);
  double module_lexch2_vector=get_module_lexchoice_vector(reduced_lexch2);


  if (module_lexch1_vector==0) {
    if (LexTor::debug) {
      wcerr<<L"Warning in LexTorData::cosine: module_lexch1_vector is equal zero.\n"
	  <<L"The cosine cannot be compute\n";
      wcerr<<L"reduced lexical choice: "<<reduced_lexch1<<L"\n";
    }
    return -2;
  }

  if (module_lexch2_vector==0) {
    if (LexTor::debug) {
      wcerr<<L"Warning in LexTorData::cosine: module_lexch2_vector is equal zero.\n"
	  <<L"The cosine cannot be compute\n";
      wcerr<<L"reduced lexical choice: "<<reduced_lexch2<<L"\n";
    }
    return -2;
  }

  return scalar_product/(module_lexch1_vector*module_lexch2_vector);
}
