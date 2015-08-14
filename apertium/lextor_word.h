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
#ifndef __LEXTORWORD_H
#define __LEXTORWORD_H

#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

#include <lttoolbox/fst_processor.h>
#include <apertium/lextor_data.h>

using namespace std;

/** Class LexTorWord. (Lexical Selector Word)
 */  

class LexTorWord{
private:
  wstring word;
  wstring ignored_string;
  vector<wstring> lexical_choices;
  int default_choice;
  
  void extract_lexical_choices(FSTProcessor *fstp);
public:

  LexTorWord();
  
  LexTorWord(const LexTorWord& ltw);

  LexTorWord(const wstring& str, FSTProcessor *fstp);
  
  ~LexTorWord();

  /** Return the lexical choice at position 'choice', if 'choice' is not
   *  given the default one is returned
   */
  wstring get_lexical_choice(int choice=-1, bool include_ignored=true);

  /** Returns the number of lexical choices for this word 
   */
  int n_lexical_choices();

  wstring get_word_string();

  wstring translate(FSTProcessor& bildic, int choice=-1);


  /** When calling this method the set of lexical choice for each word
   *  will be extracted from the FSTProcessor object if present.
   *  Moreover the input stream (is) is supossed to be in the
   *  intermediate format used by the apertium MT system.
   */
  static LexTorWord* next_word(wistream& is, FSTProcessor *fstp=NULL);
};

#endif
