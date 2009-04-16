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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __LEXTOR_EVAL_H
#define __LEXTOR_EVAL_H

#include <string>
#include <vector>
#include <set>
#include <map>
#include <istream>
#include <apertium/lextor_data.h>
#include <apertium/lextor_word.h>

using namespace std;

class LexTorEval {
private:

  double nwords;
  //double nunknown;
  double nignored;
  double npol;
  //double nerrors_nopol;
  double nerrors_pol;
  //double nerrors_unk;

  double ndefault;

  map<wstring, double> nwords_per_word;
  map<wstring, double> nerrors_per_word;
  map<wstring, double> ndefault_per_word;

  wistream* refer;

  set<wstring> words2ignore;
public:  
 
  LexTorEval(wistream *iref);

  ~LexTorEval();

  void evalword(LexTorWord& ltword, int winner, LexTorData* lextor_data);

  void print_evaluation();
};

#endif
