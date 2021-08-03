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

#ifndef _TAGGER_DATA_EXE_
#define _TAGGER_DATA_EXE_

#include <cstdint>
#include <lttoolbox/string_writer.h>

struct str_int {
  StringRef s;
  uint64_t i;
};

struct str_str_int {
  StringRef s1;
  StringRef s2;
  uint64_t i;
};

struct int_int {
  int32_t i1;
  int32_t i2;
};

class TaggerDataExe {
private:
  bool mmapping = false;

protected:
  StringWriter str_write;

  /**
   * Unigram Model 1
   */
  // form => count
  str_int* uni1 = nullptr;
  uint64_t uni1_count = 0;

  /**
   * Unigram Model 2
   */
  // (analysis, lemma) => count
  str_str_int* uni2 = nullptr;
  uint64_t uni2_count = 0;

  /**
   * Unigram Model 3
   */
  // (first tags, first lemma) => count
  str_str_int* uni3_l_t = nullptr;
  uint64_t uni3_l_t_count = 0;
  // (nth tags, n+1th tags) => count
  str_str_int* uni3_cl_ct = nullptr;
  uint64_t uni3_cl_ct_count = 0;
  // (nth lemma, nth tags) => count
  str_str_int* uni3_ct_cl = nullptr;
  uint64_t uni3_ct_cl_count = 0;

  /**
   * HMM & LSW shared data
   */
  int32_t* open_class = nullptr;
  uint64_t open_class_count = 0;

  int_int* forbid_rules = nullptr;
  uint64_t forbid_rules_count = 0;

  StringRef* array_tags = nullptr;
  uint64_t array_tags_count = 0;

  str_int* tag_index = nullptr;
  uint64_t tag_index_count = 0;

  // enforce_rules

  // prefer_rules

  str_int* constants = nullptr;
  uint64_t constants_count = 0;

  // output

  // pattern_list

  StringRef* discard = nullptr;
  uint64_t discard_count = 0;

  /**
   * HMM and LSW weight matrices
   */
  uint64_t M = 0;
  uint64_t N = 0;
  double* hmm_a = nullptr; // NxN
  double* hmm_b = nullptr; // NxM
  double* lsw_d = nullptr; // NxNxN

  /* HMM
vector<TForbidRule> forbid_rules
vector<TEnforceAfterRule> enforce_rules
vector<wstring> prefer_rules
Collection output
PatternList plist
vector<wstring> discard
  */

  /* perceptron
map<vector<string>, double> weights
  int beam_width
  vector<string> str_consts
  vector<set<string>> set_consts
  vector<FeatureDefn> features
  vector<FeatureDefn> global_defns
  FeatureDefn global_pred
  Collection output
  PatternList plist
  */

public:
  void read_compressed_unigram1(FILE* in);
  void read_compressed_unigram2(FILE* in);
  void read_compressed_unigram3(FILE* in);
  void read_compressed_hmm_lsw(FILE* in, bool is_hmm=true);
  void read_compressed_perceptron(FILE* in);
};

#endif
