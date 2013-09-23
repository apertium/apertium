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
#include <apertium/tagger_data.h>
#include <lttoolbox/compression.h>
#include <apertium/endian_double_util.h>
#include <apertium/string_utils.h>

using namespace Apertium;

void
TaggerData::copy(TaggerData const &o)
{
  open_class = o.open_class;
  forbid_rules = o.forbid_rules;
  tag_index = o.tag_index;
  array_tags = o.array_tags;
  enforce_rules = o.enforce_rules;
  prefer_rules = o.prefer_rules;
  constants = o.constants;
  output = o.output;  
  plist = o.plist;
}

TaggerData::TaggerData()
{
}

TaggerData::~TaggerData()
{
}

TaggerData::TaggerData(TaggerData const &o)
{
  copy(o);
}

TaggerData &
TaggerData::operator =(TaggerData const &o)
{
  if(this != &o)
  {
    copy(o);
  }
  return *this;
}

set<TTag> &
TaggerData::getOpenClass()
{
  return open_class;
}

void
TaggerData::setOpenClass(set<TTag> const &oc)
{
  open_class = oc;
}

vector<TForbidRule> &
TaggerData::getForbidRules()
{
  return forbid_rules;
}

void
TaggerData::setForbidRules(vector<TForbidRule> &fr)
{
  forbid_rules = fr;
}  

map<wstring, TTag, Ltstr> &
TaggerData::getTagIndex()
{
  return tag_index;
}

void
TaggerData::setTagIndex(map<wstring, TTag, Ltstr> const &ti)
{
  tag_index = ti;
}
  
vector<wstring> &
TaggerData::getArrayTags()
{
  return array_tags;
}

void
TaggerData::setArrayTags(vector<wstring> const &at)
{
  array_tags = at;
}

vector<TEnforceAfterRule> &
TaggerData::getEnforceRules()
{
  return enforce_rules;
}

void
TaggerData::setEnforceRules(vector<TEnforceAfterRule> const &tear)
{
  enforce_rules = tear;
}

vector<wstring> &
TaggerData::getPreferRules()
{
  return prefer_rules;
}

void
TaggerData::setPreferRules(vector<wstring> const &pr)
{
  prefer_rules = pr;
}

vector<wstring> &
TaggerData::getDiscardRules()
{
  return discard;
}

void
TaggerData::setDiscardRules(vector<wstring> const &v)
{
  discard = v;
}

ConstantManager &
TaggerData::getConstants()
{
  return constants;
}

void
TaggerData::setConstants(ConstantManager const &c)
{  
  constants = c;
}

Collection &
TaggerData::getOutput()
{
  return output;
}

void
TaggerData::setOutput(Collection const &c)
{
  output = c;
}

PatternList &
TaggerData::getPatternList()
{
  return plist;
}

void
TaggerData::setPatternList(PatternList const &pl)
{
  plist = pl;
}

void
TaggerData::addDiscard(wstring const &tags)
{
  discard.push_back(tags);
}
