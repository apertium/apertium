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

#include <lttoolbox/alphabet.h>
#include <lttoolbox/compression.h>

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <vector>

uint64_t deserialise_int(FILE* in)
{
  uint64_t ret = 0;
  uint8_t size = fgetc_unlocked(in);
  if (size > 8) {
    throw std::runtime_error("can't deserialise int");
  }
  uint8_t buffer[8];
  if (fread_unlocked(buffer, 1, size, in) != size) {
    throw std::runtime_error("can't deserialise int");
  }
  for (uint8_t i = 0; i < size; i++) {
    ret += static_cast<uint64_t>(buffer[i]) << (8 * (size - i - 1));
  }
  return ret;
}

StringRef deserialise_str(FILE* in, StringWriter& sw)
{
  UString s;
  for (uint64_t i = deserialise_int(in); i > 0; i--) {
    s += static_cast<UChar>(deserialise_int(in));
  }
  return sw.add(s);
}

void deserialise_str(FILE* in, UString& s)
{
  for (uint64_t i = deserialise_int(in); i > 0; i--) {
    s += static_cast<UChar>(deserialise_int(in));
  }
}

void deserialise_tags(FILE* in, UString& s)
{
  for (uint64_t i = deserialise_int(in); i > 0; i--) {
    s += '<';
    deserialise_str(in, s);
    s += '>';
  }
}

double
read_compressed_double(FILE *input)
{
  double retval;
#ifdef WORDS_BIGENDIAN
  fread_unlocked(&retval, sizeof(double), 1, input);
#else
  char *s = reinterpret_cast<char *>(&retval);

  for(int i = sizeof(double)-1; i != -1; i--)
  {
    if(fread_unlocked(&(s[i]), 1, 1, input)==0)
    {
      return 0;
    }
  }
#endif
  return retval;
}

void
TaggerDataExe::read_compressed_unigram1(FILE* in)
{
  uni1_count = deserialise_int(in);
  uni1 = new str_int[uni1_count];
  for (uint64_t i = 0; i < uni1_count; i++) {
    UString s;
    for (uint64_t j = deserialise_int(in); j > 0; j--) {
      if (!s.empty()) {
        s += '+';
      }
      deserialise_str(in, s);
      deserialise_tags(in, s);
    }
    uni1[i].s = str_write.add(s);
    uni1[i].i = deserialise_int(in);
  }
}

void
TaggerDataExe::read_compressed_unigram2(FILE* in)
{
  std::vector<StringRef> as;
  std::vector<StringRef> lems;
  std::vector<uint64_t> counts;

  for (uint64_t ans = deserialise_int(in); ans > 0; ans--) {
    UString a;
    deserialise_tags(in, a);
    for (uint64_t c = deserialise_int(in); c > 0; c--) {
      a += '+';
      deserialise_str(in, a);
      deserialise_tags(in, a);
    }
    StringRef ar = str_write.add(a);
    for (uint64_t i = deserialise_int(in); i > 0; i--) {
      as.push_back(ar);
      lems.push_back(deserialise_str(in, str_write));
      counts.push_back(deserialise_int(in));
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

  for (uint64_t ans = deserialise_int(in); ans > 0; ans--) {
    UString tg;
    deserialise_tags(in, tg);
    StringRef tgr = str_write.add(tg);
    for (uint64_t i = deserialise_int(in); i > 0; i--) {
      s1.push_back(tgr);
      s2.push_back(deserialise_str(in, str_write));
      counts.push_back(deserialise_int(in));
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

  for (uint64_t ans = deserialise_int(in); ans > 0; ans--) {
    UString tg;
    deserialise_tags(in, tg);
    StringRef tgr = str_write.add(tg);
    for (uint64_t i = deserialise_int(in); i > 0; i--) {
      s1.push_back(tgr);
      s2.push_back(deserialise_str(in, str_write));
      counts.push_back(deserialise_int(in));
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

  for (uint64_t ans = deserialise_int(in); ans > 0; ans--) {
    StringRef lm = deserialise_str(in, str_write);
    for (uint64_t i = deserialise_int(in); i > 0; i--) {
      s1.push_back(lm);
      UString tg;
      deserialise_tags(in, tg);
      s2.push_back(str_write.add(tg));
      counts.push_back(deserialise_int(in));
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
  for (uint64_t i = Compression::multibyte_read(in); i > 0; i--) {
    val += Compression::multibyte_read(in);
    open_class.push_back(val);
  }

  // forbid_rules
  forbid_rules_count = Compression::multibyte_read(in);
  forbid_rules = new int_int[forbid_rules_count];
  for (uint64_t i = 0; i < forbid_rules_count; i++) {
    forbid_rules[i].i1 = Compression::multibyte_read(in);
    forbid_rules[i].i2 = Compression::multibyte_read(in);
  }

  // array_tags
  array_tags_count = Compression::multibyte_read(in);
  array_tags = new StringRef[array_tags_count];
  for (uint64_t i = 0; i < array_tags_count; i++) {
    array_tags[i] = str_write.add(Compression::string_read(in));
  }

  // tag_index
  tag_index_count = Compression::multibyte_read(in);
  tag_index = new str_int[tag_index_count];
  for (uint64_t i = 0; i < tag_index_count; i++) {
    tag_index[i].s = str_write.add(Compression::string_read(in));
    tag_index[i].i = Compression::multibyte_read(in);
  }

  // enforce_rules
  enforce_rules_count = Compression::multibyte_read(in);
  enforce_rules_offsets = new uint64_t[enforce_rules_count+1];
  std::vector<uint64_t> enf;
  for (uint64_t i = 0; i < enforce_rules_count; i++) {
    enforce_rules_offsets[i] = enf.size();
    enf.push_back(Compression::multibyte_read(in));
    for (uint64_t j = Compression::multibyte_read(in); j > 0; j--) {
      enf.push_back(Compression::multibyte_read(in));
    }
  }
  enforce_rules_offsets[enforce_rules_count] = enf.size();
  enforce_rules = new uint64_t[enf.size()];
  for (uint64_t i = 0; i < enf.size(); i++) {
    enforce_rules[i] = enf[i];
  }

  // prefer_rules
  prefer_rules_count = Compression::multibyte_read(in);
  prefer_rules = new StringRef[prefer_rules_count];
  for (uint64_t i = 0; i < prefer_rules_count; i++) {
    prefer_rules[i] = str_write.add(Compression::string_read(in));
  }

  // constants
  constants_count = Compression::multibyte_read(in);
  constants = new str_int[constants_count];
  for (uint64_t i = 0; i < constants_count; i++) {
    constants[i].s = str_write.add(Compression::string_read(in));
    constants[i].i = Compression::multibyte_read(in);
  }

  // output
  output_count = Compression::multibyte_read(in);
  // +2 in case we need to append open_class
  output_offsets = new uint64_t[output_count+2];
  std::vector<uint64_t> out;
  for (uint64_t i = 0; i < output_count; i++) {
    output_offsets[i] = out.size();
    for (uint64_t j = Compression::multibyte_read(in); j > 0; j--) {
      out.push_back(Compression::multibyte_read(in));
    }
  }
  output_offsets[output_count] = out.size();
  open_class_index = output_count;
  for (uint64_t i = 0; i < output_count; i++) {
    if (output_offsets[i+1] - output_offsets[i] == open_class.size()) {
      bool match = true;
      for (uint64_t j = 0; j < open_class.size(); j++) {
        if (open_class[j] != output[output_offsets[i]+j]) {
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
    N = Compression::multibyte_read(in);
    M = Compression::multibyte_read(in);

    // matrix a
    hmm_a = new double[N*N];
    for (uint64_t i = 0; i < N; i++) {
      for (uint64_t j = 0; j < N; j++) {
        hmm_a[i*N+j] = read_compressed_double(in);
      }
    }

    // matrix b
    hmm_b = new double[N*M];
    memset(hmm_b, 0, N*M*sizeof(double));
    for (uint64_t count = Compression::multibyte_read(in); count > 0; count--) {
      uint64_t i = Compression::multibyte_read(in);
      uint64_t j = Compression::multibyte_read(in);
      hmm_b[i*M+j] = read_compressed_double(in);
    }
  } else {
    // dimensions
    N = Compression::multibyte_read(in);

    // matrix d
    lsw_d = new double[N*N*N];
    memset(lsw_d, 0, N*N*N*sizeof(double));
    for (uint64_t count = Compression::multibyte_read(in); count > 0; count--) {
      uint64_t i = Compression::multibyte_read(in);
      uint64_t j = Compression::multibyte_read(in);
      uint64_t k = Compression::multibyte_read(in);
      lsw_d[(i*N*N)+(j*N)+k] = read_compressed_double(in);
    }
  }

  // pattern list
  Alphabet temp;
  fpos_t pos;
  fgetpos(in, &pos);
  alpha.read(in, false);
  fsetpos(in, &pos);
  temp.read(in);
  int len = Compression::multibyte_read(in);
  if (len == 1) {
    Compression::string_read(in);
    trans.read_compressed(in, temp);
    finals_count = Compression::multibyte_read(in);
    for (uint64_t i = 0; i < finals_count; i++) {
      finals[i].i1 = Compression::multibyte_read(in);
      finals[i].i2 = Compression::multibyte_read(in);
    }
  }

  // discard
  discard_count = Compression::multibyte_read(in);
  if (feof(in)) {
    discard_count = 0;
  }
  discard = new StringRef[discard_count];
  for (uint64_t i = 0; i < discard_count; i++) {
    discard[i] = str_write.add(Compression::string_read(in));
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
