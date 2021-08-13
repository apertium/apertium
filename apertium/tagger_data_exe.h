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
#include <lttoolbox/alphabet_exe.h>
#include <lttoolbox/string_writer.h>
#include <lttoolbox/transducer_exe.h>
#include <map>
#include <set>

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
  uint64_t i1;
  uint64_t i2;
};

class TaggerDataExe {
private:
  bool mmapping = false;

public:
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
  int_int* forbid_rules = nullptr;
  uint64_t forbid_rules_count = 0;

  StringRef* array_tags = nullptr;
  uint64_t array_tags_count = 0;

  str_int* tag_index = nullptr;
  uint64_t tag_index_count = 0;

  // [ tagi, taj1, tagj2, tagi, tagj1, tagj2, tagj3 ]
  // [ 0, 3, 7 ]
  uint64_t* enforce_rules = nullptr;
  uint64_t* enforce_rules_offsets = nullptr;
  uint64_t enforce_rules_count = 0;

  StringRef* prefer_rules = nullptr;
  uint64_t prefer_rules_count = 0;

  str_int* constants = nullptr;
  uint64_t constants_count = 0;

  // output = ambiguity classes
  uint64_t* output = nullptr;
  uint64_t* output_offsets = nullptr;
  uint64_t output_count = 0;
  uint64_t open_class_index = 0; // which ambiguity class is open_class

  // patterns
  AlphabetExe alpha;
  TransducerExe trans;
  int_int* finals = nullptr;
  uint64_t finals_count = 0;

  StringRef* discard = nullptr;
  uint64_t discard_count = 0;

  /**
   * HMM and LSW weight matrices
   */
  uint64_t M = 0; // HMM number of ambiguity classes
  uint64_t N = 0; // HMM number of known tags
  double* hmm_a = nullptr; // NxN
  double* hmm_b = nullptr; // NxM
  double* lsw_d = nullptr; // NxNxN

  /* perceptron
map<vector<string>, double> weights
  int beam_width
  vector<string> str_consts
  vector<set<string>> set_consts
  vector<FeatureDefn> features
  vector<FeatureDefn> global_defns
  FeatureDefn global_pred
  */

public:
  TaggerDataExe();

  void read_compressed_unigram1(FILE* in);
  void read_compressed_unigram2(FILE* in);
  void read_compressed_unigram3(FILE* in);
  void read_compressed_hmm_lsw(FILE* in, bool is_hmm=true);
  void read_compressed_perceptron(FILE* in);

  uint64_t get_ambiguity_class(const std::set<uint64_t>& tags);

  inline double getA(uint64_t i, uint64_t j) const {
    return hmm_a[i*N + j];
  }
  inline double getB(uint64_t i, uint64_t j) const {
    return hmm_b[i*M + j];
  }
  inline double getD(uint64_t i, uint64_t j, uint64_t k) const {
    return lsw_d[i*N*N + j*N + k];
  }

  bool search(str_int* ptr, uint64_t count, UString_view key, uint64_t& val);
  bool search(str_str_int* ptr, uint64_t count, UString_view k1,
              UString_view k2, uint64_t& val);
  bool search(int_int* ptr, uint64_t count, uint64_t key, uint64_t& val);

  std::map<UString_view,
           std::pair<uint64_t, uint64_t>> summarize(str_str_int* ptr,
                                                    uint64_t count);
};

#endif
