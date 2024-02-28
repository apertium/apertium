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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */

#include <apertium/tagger_utils.h>
#include <apertium/file_morpho_stream.h>

#include <stdio.h>
#include <sstream>
#include <algorithm>
#include <climits>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/i18n.h>


void tagger_utils::fatal_error (UString const &s) {
  cerr<<"Error: "<<s<<"\n";
  exit(1);
}

void tagger_utils::file_name_error (string const &s) {
  cerr << "Error: " << s << endl;
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

int tagger_utils::ntokens_multiword(UString const &s)
{
  vector<UString> tmp = StringUtils::split(s, "_"_u);
  int n = 0;
  for (auto& it : tmp) {
    if (!it.empty()) {
      n++;
    }
  }
  return n;
}

int tagger_utils::nguiones_fs(UString const & s) {
  vector<UString> tmp = StringUtils::split(s, "-"_u);
  int n = 0;
  for (auto& it : tmp) {
    if (!it.empty()) {
      n++;
    }
  }
  return n;
}

UString tagger_utils::trim(UString s)
{
  if (s.empty()) {
    return ""_u;
  }

  for (unsigned int i=0; i<(s.length()-1); i++) {
    if ((s.at(i)==' ')&&(s.at(i+1)==' ')) {
      s.erase(i,1);
      i--;
    }
  }

  if ((s.length()>0)&&(s.at(s.length()-1)==' '))
    s.erase(s.length()-1,1);
  if ((s.length()>0)&&(s.at(0)==' '))
    s.erase(0,1);

  return s;
}

void tagger_utils::scan_for_ambg_classes(const char* fdic, TaggerData &td) {
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
      cerr << '.' << flush;

    tags = word->get_tags();

    if (tags.size() > 0)
      output[tags];

    delete word;
    word = morpho_stream.get_next_word();
  }
  cerr << "\n";
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

  for (int k=0; k<output.size(); k++) {
    const set<TTag> &ambg_class = output[k];
    if (ambg_class.size() > ret.size() ||
        (ambg_class.size() == ret.size())) {
      continue;
    }
    if (includes(ambg_class.begin(), ambg_class.end(), c.begin(), c.end())) {
      ret = ambg_class;
    }
  }
  return ret;
}

void
tagger_utils::require_ambiguity_class(TaggerData &td, set<TTag> &tags, TaggerWord &word, int nw) {
  if (td.getOutput().has_not(tags)) {
    std::ostringstream ws;
    if (nw >= 0)
      ws << (nw + 1);
    else 
      ws << "N/A";
    
    I18n(APR_I18N_DATA, "apertium").error("APR81040",
      {"word", "class", "line"},
      {icu::UnicodeString(word.get_superficial_form().data()),
       icu::UnicodeString(word.get_string_tags().data()),
       ws.str().c_str()}, true);
  }
}

static void _warn_absent_ambiguity_class(TaggerWord &word) {
  I18n(APR_I18N_DATA, "apertium").error("APR61050",
    {"word", "class"},
    {icu::UnicodeString(word.get_superficial_form().data()),
     icu::UnicodeString(word.get_string_tags().data())}, false);
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
  if (is.bad()) I18n(APR_I18N_DATA, "apertium").error("APR81060", true);
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
