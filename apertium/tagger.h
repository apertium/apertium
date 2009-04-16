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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
#ifndef __TAGGER_
#define __TAGGER_

#include <cstdio>
#include <fstream>
#include <map>
#include <set>

#include <apertium/constant_manager.h>
#include <apertium/tagger_data.h>
#include <apertium/ttag.h>

using namespace std;

class Tagger
{
private:
  enum Mode{UNKNOWN_MODE,
            TRAIN_MODE,
            TAGGER_MODE,
            RETRAIN_MODE,
            TAGGER_SUPERVISED_MODE,
            TRAIN_SUPERVISED_MODE,
            RETRAIN_SUPERVISED_MODE,
            TAGGER_EVAL_MODE,
            TAGGER_FIRST_MODE};
            
  vector<string> filenames;
  int nit;
  string name;
  bool debug;
  
  bool showSF; // show superficial forms
  void setShowSF(bool val);
  bool getShowSF();

  int getMode(int argc, char *argv[]);
  void tagger(bool model_first=false);
  void train();
  void retrain();
  void trainSupervised();
  void help();
  void filerror(string const &filename);  
  bool isNumber(const char *str);
public:
  Tagger();
  void main(int argc, char *argv[]);
};

#endif
