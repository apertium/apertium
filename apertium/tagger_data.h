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
  void setOpenClass(set<TTag> const &oc);

  vector<TForbidRule> & getForbidRules();
  void setForbidRules(vector<TForbidRule> &fr);
  
  map<wstring, TTag, Ltstr> & getTagIndex();
  void setTagIndex(map<wstring, TTag, Ltstr> const &ti);
  
  vector<wstring> & getArrayTags();
  void setArrayTags(vector<wstring> const &at);

  vector<TEnforceAfterRule> & getEnforceRules();
  void setEnforceRules(vector<TEnforceAfterRule> const &tear);

  vector<wstring> & getPreferRules();
  void setPreferRules(vector<wstring> const &pr);
  
  vector<wstring> & getDiscardRules();
  void setDiscardRules(vector<wstring> const &dr);

  ConstantManager & getConstants();
  void setConstants(ConstantManager const &c);
  
  virtual Collection & getOutput();
  void setOutput(Collection const &c);
 
  void setPatternList(PatternList const &pl);
  void addDiscard(wstring const &tags);
  PatternList & getPatternList();
};

#endif
