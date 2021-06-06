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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#ifndef _TAGGERDATA_
#define _TAGGERDATA_

#include <apertium/constant_manager.h>
#include <apertium/ttag.h>
#include <apertium/collection.h>
#include <lttoolbox/pattern_list.h>
#include <lttoolbox/ltstr.h>

#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

class TaggerData
{
protected:
  set<TTag> open_class;
  vector<TForbidRule> forbid_rules;
  map<UString, TTag> tag_index;
  vector<UString> array_tags;
  vector<TEnforceAfterRule> enforce_rules;
  vector<UString> prefer_rules;
  ConstantManager constants;
  Collection output;
  PatternList plist;
  vector<UString> discard;

  void copy(TaggerData const &o);
public:
  TaggerData();
  virtual ~TaggerData();
  TaggerData(TaggerData const &o);
  TaggerData & operator =(TaggerData const &o);

  set<TTag> & getOpenClass();
  const set<TTag> & getOpenClass() const;
  void setOpenClass(set<TTag> const &oc);

  vector<TForbidRule> & getForbidRules();
  const vector<TForbidRule> & getForbidRules() const;
  void setForbidRules(vector<TForbidRule> &fr);

  map<UString, TTag> & getTagIndex();
  const map<UString, TTag> & getTagIndex() const;
  void setTagIndex(map<UString, TTag> const &ti);

  vector<UString> & getArrayTags();
  const vector<UString> & getArrayTags() const;
  void setArrayTags(vector<UString> const &at);

  vector<TEnforceAfterRule> & getEnforceRules();
  const vector<TEnforceAfterRule> & getEnforceRules() const;
  void setEnforceRules(vector<TEnforceAfterRule> const &tear);

  vector<UString> & getPreferRules();
  const vector<UString> & getPreferRules() const;
  void setPreferRules(vector<UString> const &pr);

  vector<UString> & getDiscardRules();
  const vector<UString> & getDiscardRules() const;
  void setDiscardRules(vector<UString> const &dr);

  ConstantManager & getConstants();
  const ConstantManager & getConstants() const;
  void setConstants(ConstantManager const &c);

  virtual Collection & getOutput();
  const virtual Collection & getOutput() const;
  void setOutput(Collection const &c);

  void setPatternList(PatternList const &pl);
  PatternList & getPatternList();
  const PatternList & getPatternList() const;

  void addDiscard(UString const &tags);
};

#endif
