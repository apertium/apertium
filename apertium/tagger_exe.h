/*
 * Copyright (C) 2021 Apertium
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

#ifndef _TAGGER_EXE_
#define _TAGGER_EXE_

#include <lttoolbox/input_file.h>
#include <apertium/apertium_re.h>
#include <apertium/streamed_type.h>
#include <apertium/tagger_data_exe.h>
#include <apertium/tagger_word.h>
#include <unicode/ustdio.h>
#include <map>
#include <vector>

class TaggerExe {
private:
  bool null_flush = true;
  bool debug = false;
  bool show_superficial = false;

  bool end_of_file = false;
  std::vector<TaggerWord*> tagger_word_buffer;
  std::map<uint64_t, int> match_finals;
  void build_match_finals();
  std::vector<ApertiumRE> prefer_rules;
  void build_prefer_rules();

  std::map<UString_view, std::pair<uint64_t, uint64_t>> uni2_counts;
  void build_uni2_counts();
  long double score_unigram1(UString_view lu);
  long double score_unigram2(UString_view lu);
public:
  TaggerDataExe tde;
  Apertium::StreamedType read_streamed_type(InputFile& input);
  TaggerWord* read_tagger_word(InputFile& input);
  void tag_unigram(InputFile& input, UFILE* output, int model);
  void tag_hmm(InputFile& input, UFILE* output);
  void load(FILE* in);
};

#endif
