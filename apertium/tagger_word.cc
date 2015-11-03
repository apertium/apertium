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
#include <apertium/tagger_word.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

using namespace Apertium;

bool TaggerWord::generate_marks=false;

vector<wstring> TaggerWord::array_tags;

bool TaggerWord::show_ignored_string=true;

map<wstring, ApertiumRE, Ltstr> TaggerWord::patterns;

TaggerWord::TaggerWord(bool prev_plus_cut) :
show_sf(false)
{
   ignored_string = L"";
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
TaggerWord::set_superficial_form(const wstring &sf){
  superficial_form = sf;
}

wstring& 
TaggerWord::get_superficial_form() {
  return superficial_form;
}

bool
TaggerWord::match(wstring const &s, wstring const &pattern)
{
  map<wstring, ApertiumRE, Ltstr>::iterator it = patterns.find(pattern);
  string const utfs = UtfConverter::toUtf8(s);

  if(it == patterns.end())
  {
    string utfpattern = UtfConverter::toUtf8(pattern);
    string regexp = "";
    
    while(true)
    {
      size_t pos = utfpattern.find("<*>");
      if(pos == string::npos)
      {
        break;
      }
      utfpattern.replace(pos, 3, "(<[^>]+>)+");
    }
    patterns[pattern].compile(utfpattern);
    return patterns[pattern].match(utfs) != "";
  }
  else
  {
    return it->second.match(utfs) != "";
  }
}

void
TaggerWord::add_tag(TTag &t, const wstring &lf, vector<wstring> const &prefer_rules){

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

wstring
TaggerWord::get_string_tags() {
  wstring st;
  set<TTag>::iterator itag = tags.begin();
  
  st=L"{";  
  for(itag=tags.begin(); itag!=tags.end(); itag++) {
    if (itag!=tags.begin())
      st+=L',';
    st+=array_tags[*itag];
  }
  st += L'}';  
  
  return st;  
}

wstring
TaggerWord::get_lexical_form(TTag &t, int const TAG_kEOF) {
  wstring ret= L"";

  if (show_ignored_string)
    ret.append(ignored_string);
   
  if(t==TAG_kEOF)
    return ret;

  if (!previous_plus_cut){
    if(TaggerWord::generate_marks && isAmbiguous())
    {
      ret.append(L"^=");
    }
    else
    {
      ret += L'^';
    }

    if(get_show_sf()){   // append the superficial form
      ret.append(superficial_form);
      ret+=L'/'; 
    }
  }

  if (lexical_forms.size()==0) { // This is an UNKNOWN WORD
    ret +=L'*';
    ret.append(superficial_form);
  } else if ((*lexical_forms.begin()).second[0]==L'*') { //This is an
							//unknown word
							//that has
							//been guessed
    ret += L'*';
    ret.append(superficial_form);
  } else if (lexical_forms.size()>1) {  //This is an ambiguous word
    ret.append(lexical_forms[t]);
  } else {
    ret.append(lexical_forms[t]);
  }
  
  if (ret != ignored_string) {
    if (plus_cut)
      ret+=L'+';
    else {
      ret += L'$';	
    }
  }


  //if ((superficial_form.length()>0)&&(superficial_form[superficial_form.length()-1]=='\''))
  //   //Si la forma superficial termina en apostrofo metemos un espacio en blanco tras la cadena '/$'
  //   //o '/'. De no hacerlo en la traducción aparecerán dos palabras sin blanco alguno.
  //   ret+=" "; //Quizá este no sea el sitio apropiado para hacer esto, lo suyo sería un módulo
  //             //antes del tagger o del anmor.
     
  return ret;
}

wstring 
TaggerWord::get_all_chosen_tag_first(TTag &t, int const TAG_kEOF) {
  wstring ret=L"";

  if (show_ignored_string)
    ret.append(ignored_string);
   
  if(t==TAG_kEOF)
    return ret;
 
  if (!previous_plus_cut)
  {
    if(TaggerWord::generate_marks && isAmbiguous())
    {
      ret.append(L"^=");
    }
    else
    {
      ret += L'^';
    }
  }
 
  ret.append(superficial_form);
 
  if (lexical_forms.size()==0) { // This is an UNKNOWN WORD
    ret+=L"/*";
    ret.append(superficial_form);
  } else {
    ret+=L"/";
    ret.append(lexical_forms[t]);
    if (lexical_forms.size()>1) {
      set<TTag>::iterator it;
      for (it=tags.begin(); it!=tags.end(); it++) {
	if (*it != t) {
	  ret+=L"/";
          ret.append(lexical_forms[*it]);
	}
      }
    }
  }
   
  if (ret != ignored_string) {
    if (plus_cut)
      ret+=L"+";
    else {
      ret+=L"$";
    }
  }
      
  return ret;
}

//OBSOLETE
wstring
TaggerWord::get_lexical_form_without_ignored_string(TTag &t, int const TAG_kEOF) {
  wstring ret;
   
  if(t==TAG_kEOF)
     return ret;
 
  if (lexical_forms.size()==0) { //This is an unknown word
      ret.append(L"*^");
      ret.append(superficial_form);
  } else if ((*lexical_forms.begin()).second[0]=='*') {  //This is an unknown word that has been guessed
    ret.append(L"*^");
    ret.append(superficial_form);
  } else {
    ret += L'^';
    ret.append(lexical_forms[t]);
  }
  
  if (ret.length() != 0) {
    if (plus_cut)
      ret+=L'+';
    else {
      ret +=L'$';	
    }
  }

  return ret;
}

void
TaggerWord::add_ignored_string(wstring const &s) {
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

wostream&
operator<< (wostream& os, TaggerWord &w) {
  os<<w.get_string_tags()<< L" \t Word: " << w.get_superficial_form();
  return os;
}

void 
TaggerWord::setArrayTags(vector<wstring> const &at)
{
  array_tags = at;
}

void
TaggerWord::print()
{
  wcout << L"[#" << superficial_form << L"# ";
  for(set<TTag>::iterator it=tags.begin(), limit = tags.end(); it != limit; it++)
  {
    wcout << L"(" << *it << L" " << lexical_forms[*it] << L") ";
  }
  wcout << L"\b]\n";
}

void
TaggerWord::outputOriginal(FILE *output) {

  wstring s=superficial_form;

  map<TTag, wstring>::iterator it;
  for(it=lexical_forms.begin(); it!=lexical_forms.end(); it++) {
    if (it->second.length()>0)
    {
      s+=L'/';
      s.append(it->second);
    }
  }

  if (s.length()>0)
  {
    s=L"^"+s+L"$\n";
  }

  fputws_unlocked(s.c_str(), output);
}

void
TaggerWord::discardOnAmbiguity(wstring const &tags)
{
  if(isAmbiguous())
  {
    map<TTag, wstring>::iterator it = lexical_forms.begin(),
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
