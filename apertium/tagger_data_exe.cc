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

#include <apertium/tagger_data_exe.h>

#include <apertium/perceptron_spec.h>

#include <lttoolbox/alphabet.h>
#include <lttoolbox/old_binary.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

#include <iostream>

TaggerDataExe::TaggerDataExe()
  : alpha(AlphabetExe(&str_write))
{}

void deserialise_tags(FILE* in, UString& s)
{
  for (uint64_t i = OldBinary::read_int(in, false); i > 0; i--) {
    s += '<';
    OldBinary::read_ustr(in, s, false);
    s += '>';
  }
}

StringRef deserialise_str(FILE* in, StringWriter& sw)
{
  UString s;
  OldBinary::read_ustr(in, s, false);
  return sw.add(s);
}

void
TaggerDataExe::read_compressed_unigram1(FILE* in)
{
  uni1_count = OldBinary::read_int(in, false);
  uni1 = new str_int[uni1_count];
  for (uint64_t i = 0; i < uni1_count; i++) {
    UString s;
    for (uint64_t j = OldBinary::read_int(in, false); j > 0; j--) {
      if (!s.empty()) {
        s += '+';
      }
      OldBinary::read_ustr(in, s, false);
      deserialise_tags(in, s);
    }
    uni1[i].s = str_write.add(s);
    uni1[i].i = OldBinary::read_int(in, false);
  }
}

void
TaggerDataExe::read_compressed_unigram2(FILE* in)
{
  std::vector<StringRef> as;
  std::vector<StringRef> lems;
  std::vector<uint64_t> counts;

  for (uint64_t ans = OldBinary::read_int(in, false); ans > 0; ans--) {
    UString a;
    deserialise_tags(in, a);
    for (uint64_t c = OldBinary::read_int(in, false); c > 0; c--) {
      a += '+';
      OldBinary::read_ustr(in, a, false);
      deserialise_tags(in, a);
    }
    StringRef ar = str_write.add(a);
    for (uint64_t i = OldBinary::read_int(in, false); i > 0; i--) {
      as.push_back(ar);
      lems.push_back(deserialise_str(in, str_write));
      counts.push_back(OldBinary::read_int(in, false));
      uni2_count++;
    }
  }

  uni2 = new str_str_int[uni2_count];
  for (uint64_t i = 0; i < uni2_count; i++) {
    uni2[i].s1 = as[i];
    uni2[i].s2 = lems[i];
    uni2[i].i = counts[i];
  }
}

void
TaggerDataExe::read_compressed_unigram3(FILE* in)
{
  std::vector<StringRef> s1;
  std::vector<StringRef> s2;
  std::vector<uint64_t> counts;

  for (uint64_t ans = OldBinary::read_int(in, false); ans > 0; ans--) {
    UString tg;
    deserialise_tags(in, tg);
    StringRef tgr = str_write.add(tg);
    for (uint64_t i = OldBinary::read_int(in, false); i > 0; i--) {
      s1.push_back(tgr);
      s2.push_back(deserialise_str(in, str_write));
      counts.push_back(OldBinary::read_int(in, false));
      uni3_l_t_count++;
    }
  }
  uni3_l_t = new str_str_int[uni3_l_t_count];
  for (uint64_t i = 0; i < uni3_l_t_count; i++) {
    uni3_l_t[i].s1 = s1[i];
    uni3_l_t[i].s2 = s2[i];
    uni3_l_t[i].i = counts[i];
  }

  s1.clear();
  s2.clear();
  counts.clear();

  for (uint64_t ans = OldBinary::read_int(in, false); ans > 0; ans--) {
    UString tg;
    deserialise_tags(in, tg);
    StringRef tgr = str_write.add(tg);
    for (uint64_t i = OldBinary::read_int(in, false); i > 0; i--) {
      s1.push_back(tgr);
      s2.push_back(deserialise_str(in, str_write));
      counts.push_back(OldBinary::read_int(in, false));
      uni3_cl_ct_count++;
    }
  }
  uni3_cl_ct = new str_str_int[uni3_cl_ct_count];
  for (uint64_t i = 0; i < uni3_cl_ct_count; i++) {
    uni3_cl_ct[i].s1 = s1[i];
    uni3_cl_ct[i].s2 = s2[i];
    uni3_cl_ct[i].i = counts[i];
  }

  s1.clear();
  s2.clear();
  counts.clear();

  for (uint64_t ans = OldBinary::read_int(in, false); ans > 0; ans--) {
    StringRef lm = deserialise_str(in, str_write);
    for (uint64_t i = OldBinary::read_int(in, false); i > 0; i--) {
      s1.push_back(lm);
      UString tg;
      deserialise_tags(in, tg);
      s2.push_back(str_write.add(tg));
      counts.push_back(OldBinary::read_int(in, false));
      uni3_ct_cl_count++;
    }
  }
  uni3_ct_cl = new str_str_int[uni3_ct_cl_count];
  for (uint64_t i = 0; i < uni3_ct_cl_count; i++) {
    uni3_ct_cl[i].s1 = s1[i];
    uni3_ct_cl[i].s2 = s2[i];
    uni3_ct_cl[i].i = counts[i];
  }
}

void
TaggerDataExe::read_compressed_hmm_lsw(FILE* in, bool is_hmm)
{
  // open_class
  std::vector<uint64_t> open_class;
  uint64_t val = 0;
  for (uint64_t i = OldBinary::read_int(in); i > 0; i--) {
    val += OldBinary::read_int(in);
    open_class.push_back(val);
  }

  // forbid_rules
  forbid_rules_count = OldBinary::read_int(in);
  forbid_rules = new int_int[forbid_rules_count];
  for (uint64_t i = 0; i < forbid_rules_count; i++) {
    forbid_rules[i].i1 = OldBinary::read_int(in);
    forbid_rules[i].i2 = OldBinary::read_int(in);
  }

  // array_tags
  array_tags_count = OldBinary::read_int(in);
  array_tags = new StringRef[array_tags_count];
  for (uint64_t i = 0; i < array_tags_count; i++) {
    UString temp;
    OldBinary::read_ustr(in, temp);
    array_tags[i] = str_write.add(temp);
  }

  // tag_index
  tag_index_count = OldBinary::read_int(in);
  tag_index = new str_int[tag_index_count];
  for (uint64_t i = 0; i < tag_index_count; i++) {
    UString temp;
    OldBinary::read_ustr(in, temp);
    tag_index[i].s = str_write.add(temp);
    tag_index[i].i = OldBinary::read_int(in);
  }

  // enforce_rules
  enforce_rules_count = OldBinary::read_int(in);
  enforce_rules_offsets = new uint64_t[enforce_rules_count+1];
  std::vector<uint64_t> enf;
  for (uint64_t i = 0; i < enforce_rules_count; i++) {
    enforce_rules_offsets[i] = enf.size();
    enf.push_back(OldBinary::read_int(in));
    for (uint64_t j = OldBinary::read_int(in); j > 0; j--) {
      enf.push_back(OldBinary::read_int(in));
    }
  }
  enforce_rules_offsets[enforce_rules_count] = enf.size();
  enforce_rules = new uint64_t[enf.size()];
  for (uint64_t i = 0; i < enf.size(); i++) {
    enforce_rules[i] = enf[i];
  }

  // prefer_rules
  prefer_rules_count = OldBinary::read_int(in);
  prefer_rules = new StringRef[prefer_rules_count];
  for (uint64_t i = 0; i < prefer_rules_count; i++) {
    UString temp;
    OldBinary::read_ustr(in, temp);
    prefer_rules[i] = str_write.add(temp);
  }

  // constants
  constants_count = OldBinary::read_int(in);
  constants = new str_int[constants_count];
  for (uint64_t i = 0; i < constants_count; i++) {
    UString temp;
    OldBinary::read_ustr(in, temp);
    constants[i].s = str_write.add(temp);
    constants[i].i = OldBinary::read_int(in);
  }

  // output
  output_count = OldBinary::read_int(in);
  // +2 in case we need to append open_class
  output_offsets = new uint64_t[output_count+2];
  std::vector<uint64_t> out;
  for (uint64_t i = 0; i < output_count; i++) {
    output_offsets[i] = out.size();
    for (uint64_t j = OldBinary::read_int(in); j > 0; j--) {
      out.push_back(OldBinary::read_int(in));
    }
  }
  output_offsets[output_count] = out.size();
  open_class_index = output_count;
  for (uint64_t i = 0; i < output_count; i++) {
    if (output_offsets[i+1] - output_offsets[i] == open_class.size()) {
      bool match = true;
      for (uint64_t j = 0; j < open_class.size(); j++) {
        if (open_class[j] != out[output_offsets[i]+j]) {
          match = false;
          break;
        }
      }
      if (match) {
        open_class_index = i;
        break;
      }
    }
  }
  if (open_class_index == output_count) {
    output_count++;
    out.insert(out.end(), open_class.begin(), open_class.end());
    output_offsets[output_count] = out.size();
  }
  output = new uint64_t[out.size()];
  for (uint64_t i = 0; i < out.size(); i++) {
    output[i] = out[i];
  }

  if (is_hmm) {
    // dimensions
    N = OldBinary::read_int(in);
    M = OldBinary::read_int(in);

    // matrix a
    hmm_a = new double[N*N];
    for (uint64_t i = 0; i < N; i++) {
      for (uint64_t j = 0; j < N; j++) {
        hmm_a[i*N+j] = OldBinary::read_double(in, true, true);
      }
    }

    // matrix b
    hmm_b = new double[N*M];
    for (uint64_t i = 0; i < N*M; i++) {
      hmm_b[i] = 1e-10;
    }
    for (uint64_t count = OldBinary::read_int(in); count > 0; count--) {
      uint64_t i = OldBinary::read_int(in);
      uint64_t j = OldBinary::read_int(in);
      hmm_b[i*M+j] = OldBinary::read_double(in, true, true);
    }
  } else {
    // dimensions
    N = OldBinary::read_int(in);

    // matrix d
    lsw_d = new double[N*N*N];
    memset(lsw_d, 0, N*N*N*sizeof(double));
    for (uint64_t count = OldBinary::read_int(in); count > 0; count--) {
      uint64_t i = OldBinary::read_int(in);
      uint64_t j = OldBinary::read_int(in);
      uint64_t k = OldBinary::read_int(in);
      lsw_d[(i*N*N)+(j*N)+k] = OldBinary::read_double(in, true, true);
    }
  }

  // pattern list
  Alphabet temp;
  fpos_t pos;
  fgetpos(in, &pos);
  alpha.read(in, false);
  fsetpos(in, &pos);
  temp.read(in);
  int len = OldBinary::read_int(in);
  if (len == 1) {
    UString name;
    OldBinary::read_ustr(in, name);
    trans.read_compressed(in, temp, true);
    finals_count = OldBinary::read_int(in);
    finals = new int_int[finals_count];
    for (uint64_t i = 0; i < finals_count; i++) {
      finals[i].i1 = OldBinary::read_int(in);
      finals[i].i2 = OldBinary::read_int(in);
    }
  }

  // discard
  discard_count = OldBinary::read_int(in);
  if (feof(in)) {
    discard_count = 0;
  }
  discard = new StringRef[discard_count];
  for (uint64_t i = 0; i < discard_count; i++) {
    UString temp;
    OldBinary::read_ustr(in, temp);
    discard[i] = str_write.add(temp);
  }
}

void
TaggerDataExe::read_compressed_perceptron(FILE* in)
{
  spec = new Apertium::PerceptronSpec();
  spec->read_compressed(in);
  if (OldBinary::read_int(in, false) == 1) {
    // open_class
    std::vector<uint64_t> open_class;
    uint64_t val = 0;
    for (uint64_t i = OldBinary::read_int(in, false); i > 0; i--) {
      val += OldBinary::read_int(in, false);
      open_class.push_back(val);
    }

    // array_tags
    array_tags_count = OldBinary::read_int(in, false);
    array_tags = new StringRef[array_tags_count];
    for (uint64_t i = 0; i < array_tags_count; i++) {
      array_tags[i] = deserialise_str(in, str_write);
    }

    // tag_index
    tag_index_count = OldBinary::read_int(in, false);
    tag_index = new str_int[tag_index_count];
    for (uint64_t i = 0; i < tag_index_count; i++) {
      tag_index[i].s = deserialise_str(in, str_write);
      tag_index[i].i = OldBinary::read_int(in, false);
    }

    // constants
    constants_count = OldBinary::read_int(in, false);
    constants = new str_int[constants_count];
    for (uint64_t i = 0; i < constants_count; i++) {
      constants[i].s = deserialise_str(in, str_write);
      constants[i].i = OldBinary::read_int(in, false);
    }

    // output
    output_count = OldBinary::read_int(in, false);
    // +2 in case we need to append open_class
    output_offsets = new uint64_t[output_count+2];
    std::vector<uint64_t> out;
    for (uint64_t i = 0; i < output_count; i++) {
      output_offsets[i] = out.size();
      for (uint64_t j = OldBinary::read_int(in, false); j > 0; j--) {
        out.push_back(OldBinary::read_int(in, false));
      }
    }
    output_offsets[output_count] = out.size();
    open_class_index = output_count;
    for (uint64_t i = 0; i < output_count; i++) {
      if (output_offsets[i+1] - output_offsets[i] == open_class.size()) {
        bool match = true;
        for (uint64_t j = 0; j < open_class.size(); j++) {
          if (open_class[j] != out[output_offsets[i]+j]) {
            match = false;
            break;
          }
        }
        if (match) {
          open_class_index = i;
          break;
        }
      }
    }
    if (open_class_index == output_count) {
      output_count++;
      out.insert(out.end(), open_class.begin(), open_class.end());
      output_offsets[output_count] = out.size();
    }
    output = new uint64_t[out.size()];
    for (uint64_t i = 0; i < out.size(); i++) {
      output[i] = out[i];
    }

    // pattern list
    Alphabet temp;
    fpos_t pos;
    fgetpos(in, &pos);
    alpha.read(in, false, false);
    fsetpos(in, &pos);
    temp.read_serialised(in);
    trans.read_serialised(in, temp, true);
    finals_count = OldBinary::read_int(in, false);
    finals = new int_int[finals_count];
    for (uint64_t i = 0; i < finals_count; i++) {
      finals[i].i1 = OldBinary::read_int(in, false);
      finals[i].i2 = OldBinary::read_int(in, false);
    }
  }

  // weights
  //percep_weights;
  uint64_t count = OldBinary::read_int(in, false);
  for (uint64_t i = 0; i < count; i++) {
    std::vector<std::string> v;
    uint64_t count2 = OldBinary::read_int(in, false);
    for (uint64_t j = 0; j < count2; j++) {
      std::string s;
      OldBinary::read_str(in, s, false);
      v.push_back(s);
    }
    percep_weights.data[v] = OldBinary::read_double(in, false);
  }
}

uint64_t
TaggerDataExe::get_ambiguity_class(const std::set<uint64_t>& tags)
{
  uint64_t ret = open_class_index;
  uint64_t len = output_offsets[ret+1] - output_offsets[ret];
  for (uint64_t i = 0; i < output_count; i++) {
    uint64_t loc_len = output_offsets[i+1] - output_offsets[i];
    if (loc_len < tags.size() || loc_len >= len) {
      continue;
    }
    if (std::includes(output+output_offsets[i], output+output_offsets[i+1],
                      tags.begin(), tags.end())) {
      ret = i;
      len = loc_len;
    }
  }
  return ret;
}

bool
TaggerDataExe::search(str_int* ptr, uint64_t count, UString_view key,
                      uint64_t& val)
{
  int64_t l = 0, r = count-1, m;
  while (l <= r) {
    m = (l + r) / 2;
    if (str_write.get(ptr[m].s) == key) {
      val = ptr[m].i;
      return true;
    } else if (str_write.get(ptr[m].s) < key) {
      l = m + 1;
    } else {
      r = m - 1;
    }
  }
  return false;
}

bool
TaggerDataExe::search(str_str_int* ptr, uint64_t count,
                      UString_view key1, UString_view key2, uint64_t& val)
{
  int64_t l = 0, r = count-1, m;
  while (l <= r) {
    m = (l + r) / 2;
    if (str_write.get(ptr[m].s1) == key1 && str_write.get(ptr[m].s2) == key2) {
      val = ptr[m].i;
      return true;
    } else if ((str_write.get(ptr[m].s1) < key1) ||
               (str_write.get(ptr[m].s1) == key1 &&
                str_write.get(ptr[m].s2) < key2)) {
      l = m + 1;
    } else {
      r = m - 1;
    }
  }
  return false;
}

bool
TaggerDataExe::search(int_int* ptr, uint64_t count, uint64_t key, uint64_t& val)
{
  int64_t l = 0, r = count-1, m;
  while (l <= r) {
    m = (l + r) / 2;
    if (ptr[m].i1 == key) {
      val = ptr[m].i2;
      return true;
    } else if (ptr[m].i1 < key) {
      l = m + 1;
    } else {
      r = m - 1;
    }
  }
  return false;
}

std::map<UString_view, std::pair<uint64_t, uint64_t>>
TaggerDataExe::summarize(str_str_int* ptr, uint64_t count)
{
  std::map<UString_view, std::pair<uint64_t, uint64_t>> ret;
  for (uint64_t i = 0; i < count; i++) {
    UString_view key = str_write.get(ptr[i].s1);
    ret[key].first++;
    ret[key].second += ptr[i].i;
  }
  return ret;
}
