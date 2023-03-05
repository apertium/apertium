/*
 * Copyright (C) 2011--2012 Universitat d'Alacant
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

#ifndef __CAPS_RESTORER_H__
#define __CAPS_RESTORER_H__

#include <lttoolbox/input_file.h>
#include <lttoolbox/state.h>
#include <lttoolbox/trans_exe.h>

class CapsRestorer {
private:
  Alphabet alpha;
  TransExe trans;
  std::map<UString, double> weights;
  std::map<Node*, double> finals;
  std::set<UChar32> escaped_chars;
  State initial_state;
  State current_state;

  bool delete_wblanks = true;

  int32_t any_char = 0;
  int32_t any_upper = 0;
  int32_t any_lower = 0;
  int32_t any_tag = 0;
  int32_t word_boundary = 0;
  int32_t null_boundary = 0;

  struct LU {
    UString blank;
    UString wblank;
    UString form;
    UString surf;
    double w_dix = 0.0;
    double w_AA = 0.0;
    double w_Aa = 0.0;
    double w_aa = 0.0;
  };

  std::vector<LU> words;

  void read_word(InputFile& input);
  void output_all(UFILE* output);
  void step(int32_t sym);
  void read_seg(InputFile& input, UString& str);

public:
  CapsRestorer();
  ~CapsRestorer();

  void load(FILE* input);
  void process(InputFile& input, UFILE* output);
  void setDeleteWblanks(bool val) { delete_wblanks = val; } ;
};

#endif
