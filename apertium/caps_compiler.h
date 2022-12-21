/*
 * Copyright (C) 2022 Apertium
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

#ifndef __CAPS_COMPILER_H__
#define __CAPS_COMPILER_H__

#include <lttoolbox/transducer.h>
#include <libxml/parser.h>
#include <libxml/tree.h>

class CapsCompiler {
private:
  Alphabet alpha;
  Transducer trans;

  std::vector<double> rule_weights;

  int32_t any_tag = 0;
  int32_t any_char = 0;
  int32_t any_upper = 0;
  int32_t any_lower = 0;
  int32_t word_boundary = 0;
  int32_t null_boundary = 0;
  int32_t AA_sym = 0;
  int32_t Aa_sym = 0;
  int32_t aa_sym = 0;
  int32_t dix_sym = 0;
  int32_t skip_sym = 0;

  void compile_rule(xmlNode* node);
  int32_t compile_node(xmlNode* node, int32_t start_state);
  int32_t compile_or(xmlNode* node, int32_t start_state);
  int32_t compile_repeat(xmlNode* node, int32_t start_state);
  int32_t compile_match(xmlNode* node, int32_t start_state);
  int32_t add_loop(int32_t sym, int32_t state);
  int32_t compile_caps_specifier(const UString& spec, int32_t state);

public:
  const static UString CAPS_COMPILER_CAPITALIZATION_ELEM;
  const static UString CAPS_COMPILER_RULES_ELEM;
  const static UString CAPS_COMPILER_RULE_ELEM;
  const static UString CAPS_COMPILER_MATCH_ELEM;
  const static UString CAPS_COMPILER_OR_ELEM;
  const static UString CAPS_COMPILER_REPEAT_ELEM;
  const static UString CAPS_COMPILER_BEGIN_ELEM;

  const static UString CAPS_COMPILER_USE_ATTR;
  const static UString CAPS_COMPILER_WEIGHT_ATTR;
  const static UString CAPS_COMPILER_SELECT_ATTR;
  const static UString CAPS_COMPILER_LEMMA_ATTR;
  const static UString CAPS_COMPILER_TAGS_ATTR;
  const static UString CAPS_COMPILER_SURFACE_ATTR;
  const static UString CAPS_COMPILER_SRCSURF_ATTR;
  const static UString CAPS_COMPILER_TRGSURF_ATTR;
  const static UString CAPS_COMPILER_SRCLEM_ATTR;
  const static UString CAPS_COMPILER_TRGLEM_ATTR;
  const static UString CAPS_COMPILER_FROM_ATTR;
  const static UString CAPS_COMPILER_UPTO_ATTR;

  const static UString CAPS_COMPILER_AA_VAL;
  const static UString CAPS_COMPILER_Aa_VAL;
  const static UString CAPS_COMPILER_aa_VAL;
  const static UString CAPS_COMPILER_DIX_VAL;
  const static UString CAPS_COMPILER_LEFT_VAL;
  const static UString CAPS_COMPILER_RIGHT_VAL;

  const static UString CAPS_COMPILER_TYPE_AA;
  const static UString CAPS_COMPILER_TYPE_Aa;
  const static UString CAPS_COMPILER_TYPE_aa;
  const static UString CAPS_COMPILER_TYPE_DIX;
  const static UString CAPS_COMPILER_TYPE_SKIP;

  const static double  CAPS_COMPILER_DEFAULT_WEIGHT;

  CapsCompiler();
  ~CapsCompiler();
  void parse(const std::string& fname);
  void write(FILE* output);
};

#endif
