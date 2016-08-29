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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _TAGGERDATA_
#define _TAGGERDATA_

#include <apertium/constant_manager.h>
#include <apertium/ttag.h>
#include <apertium/collection.h>
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
  map<wstring, TTag, Ltstr> tag_index;
  vector<wstring> array_tags;
  vector<TEnforceAfterRule> enforce_rules;
  vector<wstring> prefer_rules;
  ConstantManager constants;
  Collection output;
  PatternList plist;
  vector<wstring> discard;
  
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
  
  map<wstring, TTag, Ltstr> & getTagIndex();
  const map<wstring, TTag, Ltstr> & getTagIndex() const;
  void setTagIndex(map<wstring, TTag, Ltstr> const &ti);
  
  vector<wstring> & getArrayTags();
  const vector<wstring> & getArrayTags() const;
  void setArrayTags(vector<wstring> const &at);

  vector<TEnforceAfterRule> & getEnforceRules();
  const vector<TEnforceAfterRule> & getEnforceRules() const;
  void setEnforceRules(vector<TEnforceAfterRule> const &tear);

  vector<wstring> & getPreferRules();
  const vector<wstring> & getPreferRules() const;
  void setPreferRules(vector<wstring> const &pr);
  
  vector<wstring> & getDiscardRules();
  const vector<wstring> & getDiscardRules() const;
  void setDiscardRules(vector<wstring> const &dr);

  ConstantManager & getConstants();
  const ConstantManager & getConstants() const;
  void setConstants(ConstantManager const &c);
  
  virtual Collection & getOutput();
  const virtual Collection & getOutput() const;
  void setOutput(Collection const &c);
 
  void setPatternList(PatternList const &pl);
  PatternList & getPatternList();
  const PatternList & getPatternList() const;

  void addDiscard(wstring const &tags);
};

#endif
