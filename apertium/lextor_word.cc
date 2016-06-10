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

#include <apertium/lextor_word.h>
#include <apertium/string_utils.h>

using namespace Apertium;
LexTorWord::LexTorWord() {
  ignored_string = L"";
  word = L"";
  default_choice = 0;
}
  
LexTorWord::LexTorWord(const LexTorWord& ltw) {
  word=ltw.word;
  ignored_string=ltw.ignored_string;
  lexical_choices=ltw.lexical_choices;
  default_choice=ltw.default_choice;
}

LexTorWord::LexTorWord(const wstring &str, FSTProcessor *fstp) {
  word=str;
  ignored_string=L"";
  extract_lexical_choices(fstp);
}
  
LexTorWord::~LexTorWord() {
}

wstring 
LexTorWord::get_word_string() {
  return word;
}

int 
LexTorWord::n_lexical_choices() {
  return lexical_choices.size();
}

wstring 
LexTorWord::get_lexical_choice(int choice, bool include_ignored) {
  if (word == L"") {
    if (include_ignored)
      return ignored_string;
    else 
      return L"";
  }

  if (choice<0)
    choice=default_choice;

  if (choice>=(int)lexical_choices.size()) {
    wcerr<<L"Error in LexTorWord::get_lexical_choice, choice position given is "
	<<L"greater than the number of choices available\n";
    wcerr<<L"position requested: "<<choice<<"\n";
    wcerr<<L"number of lexical choices: "<<lexical_choices.size()<<"\n";
    wcerr<<L"Returning default choice\n";
    choice=default_choice;
  }

  if (include_ignored)
    return ignored_string+L"^"+lexical_choices[choice]+L"$";
  else
    return lexical_choices[choice];
}

wstring 
LexTorWord::translate(FSTProcessor& bildic, int lexchoice) {
  wstring t;

  //wcerr<<"In LexTorWord::translate, called with: "<<word<<", lexchoice: "<<lexchoice<<"\n";

  if (lexchoice<0)
    lexchoice=default_choice;
  else if(lexchoice>=(int)lexical_choices.size()) {
    wcerr<<L"Error in LexTorWord::translate, choice position given is "
	 <<L"greater than the number of choices available\n";
    wcerr<<L"position requested: "<<lexchoice<<"\n";
    wcerr<<L"number of lexical choices: "<<lexical_choices.size()<<"\n";
    wcerr<<L"Returning default choice\n";
    lexchoice=default_choice;
  }

  t=bildic.biltrans(lexical_choices[lexchoice], false);

  //wcerr<<"Translation: "<<t<<"\n";

  return t;
}

void 
LexTorWord::extract_lexical_choices(FSTProcessor *fstp) {

  lexical_choices=StringUtils::split_wstring(fstp->biltrans(word,false), L"/");
  default_choice=0;

  if (lexical_choices.size()>1) { //lexically ambiguous word
    for(unsigned int i=0; i<lexical_choices.size(); i++) {

      unsigned int p=lexical_choices[i].find(L" D<");
      if (p!=static_cast<unsigned int>(string::npos)) {
	if (!((lexical_choices[i].length()>p+2) && (lexical_choices[i][p+2]=='<'))) {
	  wcerr<<L"Error in LexTorWord::next_word when analyzing lexical options\n";
	  wcerr<<L"Word: "<<word<<"; lexical choices: "<<fstp->biltrans(word,false)<<L"\n";
	  exit(EXIT_FAILURE);
	}
	default_choice=i;
      }
    }
  }
}

LexTorWord* 
LexTorWord::next_word(wistream& is, FSTProcessor *fstp) {
  LexTorWord w;
  wchar_t c, prev_c=L' ';
  bool finish=false;
  bool reading_word=false;

  while (!finish) {
    is>>c;

    if (is.fail()) {
      if (reading_word) {
	wcerr<<L"Error in LexTorWord::next_word while reading input word\n";
	wcerr<<L"Malformed input string, at '"<<c<<L"'\n";
	exit(EXIT_FAILURE);
      } else {
	if ((w.word.length()>0)||(w.ignored_string.length()>0)) {
	  if(fstp!=NULL)
	    w.extract_lexical_choices(fstp);
	  return new LexTorWord(w);
	} else 
	  return NULL;
      }
    }

    if ((c==L'^') && (prev_c!=L'\\') && (!reading_word)) {
      reading_word=true;
    } else if ((c==L'$') && (prev_c!=L'\\') && (reading_word)) {
      finish=true;
    } else {
      if (reading_word)
	w.word+=c;
      else
	w.ignored_string+=c;
    }
    prev_c=c;
  }

  if ((w.word.length()==0) && (w.ignored_string.length()==0))
    return NULL;

  if(fstp!=NULL)
    w.extract_lexical_choices(fstp);

  /*
    wcerr<<"word: "<<w.word<<"\n";
    for (unsigned int i=0; i<w.lexical_choices.size(); i++) {
    wcerr<<"Lex choice at "<<i<<": "<<w.lexical_choices[i]<<"\n";
    }
    wcerr<<"Default: "<<w.default_choice<<"\n\n";
  */

  return new LexTorWord(w);
}

