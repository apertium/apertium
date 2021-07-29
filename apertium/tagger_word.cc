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
#include <apertium/tagger_word.h>
#include <lttoolbox/string_utils.h>
#include <apertium/unlocked_cstdio.h>

bool TaggerWord::generate_marks=false;

vector<UString> TaggerWord::array_tags;

bool TaggerWord::show_ignored_string=true;

map<UString, ApertiumRE> TaggerWord::patterns;

TaggerWord::TaggerWord(bool prev_plus_cut) :
show_sf(false)
{
   plus_cut=false;
   previous_plus_cut=prev_plus_cut;
}

TaggerWord::TaggerWord(const TaggerWord &w){
  superficial_form = w.superficial_form;
  tags = w.tags;
  show_sf = false;
  lexical_forms = w.lexical_forms;
  ignored_string = w.ignored_string;
  plus_cut = w.plus_cut;
  previous_plus_cut=w.previous_plus_cut;
}

TaggerWord::~TaggerWord(){
}

void
TaggerWord::set_show_sf(bool sf){
  show_sf = sf;
}

bool
TaggerWord::get_show_sf(){
  return show_sf;
}

void
TaggerWord::set_superficial_form(const UString &sf){
  superficial_form = sf;
}

UString&
TaggerWord::get_superficial_form() {
  return superficial_form;
}

bool
TaggerWord::match(UString const &s, UString const &pattern)
{
  map<UString, ApertiumRE>::iterator it = patterns.find(pattern);

  if(it == patterns.end())
  {
    UString utfpattern = pattern;
    UString regexp;

    while(true)
    {
      size_t pos = utfpattern.find("<*>"_u);
      if(pos == UString::npos)
      {
        break;
      }
      utfpattern.replace(pos, 3, "(<[^>]+>)+"_u);
    }
    patterns[pattern].compile(utfpattern);
    return !patterns[pattern].match(s).empty();
  }
  else
  {
    return !it->second.match(s).empty();
  }
}

void
TaggerWord::add_tag(TTag &t, const UString &lf, vector<UString> const &prefer_rules){

  //Tag is added only is it is not present yet
  //Sometime one word can have more than one lexical form assigned to the same tag
  if (tags.find(t)==tags.end()) {
    tags.insert(t);
    lexical_forms[t]=lf;
  } else {
    //Take a look at the prefer rules
    for(int i=0; i < (int) prefer_rules.size(); i++)
    {
      if (match(lf, prefer_rules[i]))
      {
	lexical_forms[t]=lf;
	break;
      }
    }
  }
}

set<TTag>&
TaggerWord::get_tags() {
  return tags;
}

bool
TaggerWord::isAmbiguous() const
{
  return tags.size() > 1;
}

UString
TaggerWord::get_string_tags() {
  UString st;
  set<TTag>::iterator itag = tags.begin();

  st += '{';
  for(itag=tags.begin(); itag!=tags.end(); itag++) {
    if (itag!=tags.begin())
      st+=',';
    st+=array_tags[*itag];
  }
  st += '}';

  return st;
}

UString
TaggerWord::get_lexical_form(TTag &t, int const TAG_kEOF) {
  UString ret;

  if (show_ignored_string)
    ret.append(ignored_string);

  if(t==TAG_kEOF)
    return ret;

  if (!previous_plus_cut) {
    if(TaggerWord::generate_marks && isAmbiguous()) {
      ret.append("^="_u);
    } else {
      ret += '^';
    }

    if(get_show_sf()) {   // append the superficial form
      ret.append(superficial_form);
      ret += '/';
    }
  }

  if (lexical_forms.size()==0) { // This is an UNKNOWN WORD
    ret += '*';
    ret.append(superficial_form);
  } else if ((*lexical_forms.begin()).second[0]=='*') { //This is an
							//unknown word
							//that has
							//been guessed
    ret += '*';
    ret.append(superficial_form);
  } else if (lexical_forms.size()>1) {  //This is an ambiguous word
    ret.append(lexical_forms[t]);
  } else {
    ret.append(lexical_forms[t]);
  }

  if (ret != ignored_string) {
    if (plus_cut)
      ret += '+';
    else {
      ret += '$';
    }
  }


  //if ((superficial_form.length()>0)&&(superficial_form[superficial_form.length()-1]=='\''))
  //   //Si la forma superficial termina en apostrofo metemos un espacio en blanco tras la cadena '/$'
  //   //o '/'. De no hacerlo en la traducción aparecerán dos palabras sin blanco alguno.
  //   ret+=" "; //Quizá este no sea el sitio apropiado para hacer esto, lo suyo sería un módulo
  //             //antes del tagger o del anmor.

  return ret;
}

UString
TaggerWord::get_all_chosen_tag_first(TTag &t, int const TAG_kEOF) {
  UString ret;

  if (show_ignored_string) {
    ret.append(ignored_string);
  }

  if(t==TAG_kEOF) {
    return ret;
  }

  if (!previous_plus_cut) {
    if(TaggerWord::generate_marks && isAmbiguous()) {
      ret.append("^="_u);
    } else {
      ret += '^';
    }
  }

  ret.append(superficial_form);

  if (lexical_forms.size()==0) { // This is an UNKNOWN WORD
    ret += "/*"_u;
    ret.append(superficial_form);
  } else {
    ret+="/"_u;
    ret.append(lexical_forms[t]);
    if (lexical_forms.size()>1) {
      for (auto& it : tags) {
        if (it != t) {
          ret += '/';
          ret.append(lexical_forms[it]);
        }
      }
    }
  }

  if (ret != ignored_string) {
    if (plus_cut) {
      ret += '+';
    } else {
      ret += '$';
    }
  }

  return ret;
}

//OBSOLETE
UString
TaggerWord::get_lexical_form_without_ignored_string(TTag &t, int const TAG_kEOF) {
  UString ret;

  if(t==TAG_kEOF) {
     return ret;
  }

  if (lexical_forms.size()==0) { //This is an unknown word
    ret.append("*^"_u);
    ret.append(superficial_form);
  } else if ((*lexical_forms.begin()).second[0]=='*') {  //This is an unknown word that has been guessed
    ret.append("*^"_u);
    ret.append(superficial_form);
  } else {
    ret += '^';
    ret.append(lexical_forms[t]);
  }

  if (ret.length() != 0) {
    if (plus_cut)
      ret += '+';
    else {
      ret += '$';
    }
  }

  return ret;
}

void
TaggerWord::add_ignored_string(UString const &s) {
  ignored_string.append(s);
}

void
TaggerWord::set_plus_cut(const bool &c) {
  plus_cut=c;
}

bool
TaggerWord::get_plus_cut() {
  return plus_cut;
}

ostream&
operator<< (ostream& os, TaggerWord &w) {
  os<<w.get_string_tags()<< " \t Word: " << w.get_superficial_form();
  return os;
}

void
TaggerWord::setArrayTags(vector<UString> const &at)
{
  array_tags = at;
}

void
TaggerWord::print()
{
  cout << "[#" << superficial_form << "# ";
  for(set<TTag>::iterator it=tags.begin(), limit = tags.end(); it != limit; it++)
  {
    cout << "(" << *it << " " << lexical_forms[*it] << ") ";
  }
  cout << "\b]\n";
}

void
TaggerWord::outputOriginal(UFILE *output) {

  UString s=superficial_form;

  for (auto& it : lexical_forms) {
    if (!it.second.empty()) {
      s += '/';
      s.append(it.second);
    }
  }

  if (!s.empty()) {
    u_fprintf(output, "^%S$\n", s.c_str());
  }
}

void
TaggerWord::discardOnAmbiguity(UString const &tags)
{
  if(isAmbiguous())
  {
    map<TTag, UString>::iterator it = lexical_forms.begin(),
                              limit = lexical_forms.end();
    set<TTag> newsettag;
    while(it != limit)
    {
      if(match(it->second, tags))
      {
        lexical_forms.erase(it);
        it = lexical_forms.begin();
      }
      else
      {
        newsettag.insert(it->first);
      }

      if(lexical_forms.size() == 1)
      {
        newsettag.insert(lexical_forms.begin()->first);
        break;
      }
      it++;
    }
    if(tags.size() != newsettag.size())
    {
      this->tags = newsettag;
    }
  }
}
