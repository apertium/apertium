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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <apertium/tagger_utils.h>
#include <apertium/file_morpho_stream.h>

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <climits>
#include <apertium/string_utils.h>
#ifdef _MSC_VER
#define wcstok wcstok_s
#endif
#ifdef __MINGW32__

wchar_t *_wcstok(wchar_t *wcs, const wchar_t *delim, wchar_t **ptr) {
  (void)ptr;
  return wcstok(wcs, delim);
}

#define wcstok _wcstok
#endif

using namespace Apertium;


void tagger_utils::fatal_error (wstring const &s) {
  wcerr<<L"Error: "<<s<<L"\n";
  exit(1);
}

void tagger_utils::file_name_error (string const &s) { 
  wcerr << "Error: " << s << endl;
  exit(1);
}

char * tagger_utils::itoa(int i) {                 
  static char buf[512];
  sprintf(buf,"%d",i);
  return buf;
}

void tagger_utils::clear_array_double(double a[], int l) {
  for(int i=0; i<l; i++)
    a[i]=0.0;
}

void tagger_utils::clear_array_vector(vector<TTag> v[], int l) {
  for(int i=0; i<l; i++)
    v[i].clear();
}

int tagger_utils::ntokens_multiword(wstring const &s) 
{
   wchar_t *news = new wchar_t[s.size()+1];
   wcscpy(news, s.c_str());
   news[s.size()] = 0;
   wcerr << news << endl;
   
   wchar_t const *delim = L"_";
   wchar_t *ptr;
   int n=0;
   
   if (wcstok(news, delim, &ptr))
     n++;  
   while (wcstok(NULL, delim, &ptr))
     n++;
     
   delete[] news;
     
   return n;   
}
 
int tagger_utils::nguiones_fs(wstring const & s) {
   wchar_t *news = new wchar_t[s.size()+1];
   wcscpy(news, s.c_str());
   news[s.size()] = 0;
   wcerr << news << endl;   
   wchar_t const *delim = L"-";
   wchar_t *ptr;
   int n=0;
   
   if (wcstok(news, delim, &ptr))
     n++;  
   while (wcstok(NULL, delim, &ptr))
     n++;
     
   delete[] news;
     
   return n;   
} 

wstring tagger_utils::trim(wstring s) 
{
  if (s.length()==0)
    return L"";
      
  for (unsigned int i=0; i<(s.length()-1); i++) {
    if ((s.at(i)==L' ')&&(s.at(i+1)==L' ')) {
      s.erase(i,1);
      i--;
    }
  }
                              
  if ((s.length()>0)&&(s.at(s.length()-1)==L' '))
    s.erase(s.length()-1,1);
  if ((s.length()>0)&&(s.at(0)==L' '))
    s.erase(0,1);  

  return s;
}

void tagger_utils::scan_for_ambg_classes(FILE *fdic, TaggerData &td) {
  Collection &output = td.getOutput();
  FileMorphoStream morpho_stream(fdic, true, &td);
  tagger_utils::scan_for_ambg_classes(output, morpho_stream);
}

void tagger_utils::scan_for_ambg_classes(Collection &output, MorphoStream &morpho_stream) {
  int nw = 0;
  set <TTag> tags;
  TaggerWord *word = NULL;

  // In the input dictionary there must be all punctuation marks, including the end-of-sentence mark

  word = morpho_stream.get_next_word();

  while (word) {
    if (++nw % 10000 == 0)
      wcerr << L'.' << flush;

    tags = word->get_tags();

    if (tags.size() > 0)
      output[tags];

    delete word;
    word = morpho_stream.get_next_word();
  }
  wcerr << L"\n";
}

void
tagger_utils::add_neccesary_ambg_classes(TaggerData &td) {
  int i;
  Collection &output = td.getOutput();

  // OPEN AMBIGUITY CLASS
  // It contains all tags that are not closed.
  // Unknown words are assigned the open ambiguity class
  output[td.getOpenClass()];

  // Create ambiguity class holding one single tag for each tag.
  // If not created yet
  int N = (td.getTagIndex()).size();
  for(i = 0; i != N; i++) {
    set<TTag> amb_class;
    amb_class.insert(i);
    output[amb_class];
  }
}

set<TTag> &
tagger_utils::find_similar_ambiguity_class(TaggerData &td, set<TTag> &c) {
  set<TTag> &ret = td.getOpenClass();
  Collection &output = td.getOutput();
  int ret_idx = output[ret];

  for (int k=0; k<output.size(); k++) {
    const set<TTag> &ambg_class = output[k];
    if (ambg_class.size() > ret.size() ||
        (ambg_class.size() == ret.size())) {
      continue;
    }
    if (includes(ambg_class.begin(), ambg_class.end(), c.begin(), c.end())) {
      ret_idx = k;
      ret = ambg_class;
    }
  }
  return ret;
}

void
tagger_utils::require_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, int nw) {
  if (td.getOutput().has_not(tags)) {
    wstring errors;
    errors = L"A new ambiguity class was found. I cannot continue.\n";
    errors+= L"Word '" + word.get_superficial_form() + L"' not found in the dictionary.\n";
    errors+= L"New ambiguity class: " + word.get_string_tags() + L"\n";
    if (nw >= 0) {
      std::wostringstream ws;
      ws << (nw + 1);
      errors+= L"Line number: " + ws.str() + L"\n";
    }
    errors+= L"Take a look at the dictionary, then retrain.";
    fatal_error(errors);
  }
}

static void _warn_absent_ambiguity_class(TaggerWord &word) {
  wstring errors;
  errors = L"A new ambiguity class was found. \n";
  errors += L"Retraining the tagger is necessary so as to take it into account.\n";
  errors += L"Word '" + word.get_superficial_form() + L"'.\n";
  errors += L"New ambiguity class: " + word.get_string_tags() + L"\n";
  wcerr << L"Error: " << errors;
}

set<TTag> &
tagger_utils::require_similar_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, bool warn) {
  if (td.getOutput().has_not(tags)) {
    if (warn) {
      _warn_absent_ambiguity_class(word);
    }
    return find_similar_ambiguity_class(td, tags);
  }
  return tags;
}

set<TTag> &
tagger_utils::require_similar_ambiguity_class(TaggerData &td, set<TTag> &tags) {
  if (td.getOutput().has_not(tags)) {
    return find_similar_ambiguity_class(td, tags);
  }
  return tags;
}

void
tagger_utils::warn_absent_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, bool warn) {
  if (warn && td.getOutput().has_not(tags)) {
    _warn_absent_ambiguity_class(word);
  }
}

template <class T>
ostream& operator<< (ostream& os, const map <int, T> & f){
  typename map <int, T>::const_iterator it;
  os<<f.size();
  for (it=f.begin(); it!=f.end(); it++) 
    os<<' '<<it->first<<' '<<it->second;
  return os;
}

template <class T>
istream& operator>> (istream& is, map <int, T> & f) {
  int n, i, k;
  f.clear();
  is>>n; 
  for (k=0; k<n; k++) {
    is>>i;     // warning: does not work if both
    is>>f[i];  // lines merged in a single one
  }
  if (is.bad()) tagger_utils::fatal_error(L"reading map");
  return is;
}

template <class T>
ostream& operator<< (ostream& os, const set<T>& s) {
  typename set<T>::iterator it = s.begin();
  os<<'{';
  if (it!=s.end()) {
    os<<*it;
    while (++it!=s.end()) os<<','<<*it;
  }
  os<<'}';
  return os;
}

