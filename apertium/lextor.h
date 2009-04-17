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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#ifndef __LEXTOR_H
#define __LEXTOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <deque>

#include <apertium/lextor_data.h>
#include <apertium/lextor_word.h>
#include <apertium/lextor_eval.h>

using namespace std;

/** Class LexTor (Lexical Selector class)
 */  

class LexTor {
private:
  LexTorData *lextor_data;

  //For usew when tl information is used to perform lexical selection
  LexTorData *tlmodel;
  FSTProcessor *fstpbil;

  int estimate_winner_lch(deque<LexTorWord>& window, int word_index,  double weigth_exponent);
  int estimate_winner_lch_voting(deque<LexTorWord>& window, int word_index,  double weigth_exponent);
  int estimate_winner_lch_cosine(deque<LexTorWord>& window, int word_index,  double weigth_exponent);
  int estimate_winner_lch_mostprob(deque<LexTorWord>& window, int word_index,  double weigth_exponent);
  int estimate_winner_lch_votingtl(deque<LexTorWord>& window, int word_index,  double weigth_exponent);

  double cosine(map<wstring, double>& vcontext, const wstring& reduced_lexchoice);
public:

  static bool debug;
  static double angleth;

  LexTor();
  
  LexTor(const LexTor& lt);
  
  ~LexTor();

  void set_lextor_data(LexTorData* ltd);

  //Use to set the tlmodel to be used when tl information is used to
  //perform lexical selection
  void set_tlmodel(LexTorData* tlm);
  void set_bildic(FSTProcessor *fstp);

  void trainwrd(wistream& wis, int left, int right, double weigth_exponent=0);

  void trainlch(wistream& wis, int left, int right, LexTorData& wordmodel, 
                FSTProcessor& dic, FSTProcessor& bildic, double weigth_exponent=0);

  void lexical_selector(wistream& wis, FSTProcessor &fstp, int left, int right, 
                        double weigth_exponent=0, LexTorEval* lteval=NULL);

  /** NOTE on the weigth_exponent parameter: This parameter is used to
      change the influence of surrounding words on the decision to
      take on an ambiguous word (word with more than one lexical
      choice). For example, if a decision is being take on word w_i,
      the the weigth of the surrounding words is: 
      Score(w_i-2) = count(w_i-2)/pow(2,weigth_exponent), 
      Score(w_i-1) = count(w_i-1)/pow(1,weigth_exponent), 
      Score(w_i+1) = count(w_i+1)/pow(1,weigth_exponent), 
      Score(w_i+2) = count(w_i+2)/pow(2,weigth_exponent).
  */
};

class PairStringCountComparer {
public:
  bool operator()(const pair<wstring, COUNT_DATA_TYPE>& e1, const pair<wstring, COUNT_DATA_TYPE>& e2)  const {
    //True if e1>e2

    if (e1.second > e2.second)
      return true;
    else if (e1.second == e2.second)
      return (e1.first>e2.first);
    else
      return false;
  }
};

#endif
