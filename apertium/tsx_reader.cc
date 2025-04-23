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
#include <apertium/tsx_reader.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/string_utils.h>

#include <cstdlib>
#include <iostream>

void
TSXReader::copy(TSXReader const &o)
{
}

void
TSXReader::destroy()
{
}

TSXReader::TSXReader()
{
  open_class = &(tdata.getOpenClass());
  forbid_rules = &(tdata.getForbidRules());
  tag_index = &(tdata.getTagIndex());
  array_tags = &(tdata.getArrayTags());
  enforce_rules = &(tdata.getEnforceRules());
  prefer_rules = &(tdata.getPreferRules());
  plist = &(tdata.getPatternList());
  constants = &(tdata.getConstants());
}

TSXReader::~TSXReader()
{
  destroy();
}

TSXReader::TSXReader(TSXReader const &o)
{
  copy(o);
}


void
TSXReader::clearTagIndex()
{
  tag_index->clear();
  array_tags->clear();
  newTagIndex("LPAR"_u);
  newTagIndex("RPAR"_u);
  newTagIndex("LQUEST"_u);
  newTagIndex("CM"_u);
  newTagIndex("SENT"_u);
  newTagIndex("kEOF"_u);
  newTagIndex("kUNDEF"_u);
}

TSXReader &
TSXReader::operator =(TSXReader const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
}

void
TSXReader::newTagIndex(UString const &tag)
{
  if(tag_index->find("TAG_"_u + tag) != tag_index->end())
  {
    parseError("'"_u + tag + "' already defined"_u);
  }

  array_tags->push_back("TAG_"_u + tag);
  (*tag_index)["TAG_"_u + tag] = array_tags->size() - 1;
}

void
TSXReader::newDefTag(UString const &tag)
{
  if(tag_index->find("TAG_"_u + tag) != tag_index->end())
  {
    parseError("'"_u + tag + "' already defined"_u);
  }

  array_tags->push_back(tag);
  (*tag_index)["TAG_"_u + tag] = array_tags->size() - 1;
}

void
TSXReader::newConstant(UString const &constant)
{
  constants->setConstant(constant, array_tags->size());
  array_tags->push_back(constant);
}

void
TSXReader::procDiscardOnAmbiguity()
{
  while(type != XML_READER_TYPE_END_ELEMENT || name != "discard-on-ambiguity"_u)
  {
    step();

    if(name == "discard"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        tdata.addDiscard("<"_u + StringUtils::substitute(attrib("tags"_u), "."_u, "><"_u) + ">"_u);
      }
    }
    else if(name == "#text"_u)
    {
      // do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else if(name == "discard-on-ambiguity"_u)
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError("Unexpected 'discard-on-ambiguity' open tag"_u);
      }
    }
    else
    {
      unexpectedTag();
    }
  }
}

void
TSXReader::procDefLabel()
{
  UString name_attr = attrib("name"_u);
  UString closed_attr = attrib("closed"_u);
  newDefTag(name_attr);

  if(closed_attr != "true"_u)
  {
    open_class->insert((*tag_index)["TAG_"_u + name_attr]);
  }

  while(type != XML_READER_TYPE_END_ELEMENT || name != "def-label"_u)
  {
    step();

    if(name == "tags-item"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	plist->insert((*tag_index)["TAG_"_u + name_attr], attrib("lemma"_u),
		     attrib("tags"_u));
      }
    }
    else if(name == "def-label"_u)
    {
      return;
    }
    else if(name == "#text"_u)
    {
      // do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else
    {
      unexpectedTag();
    }
  }
}

void
TSXReader::procDefMult()
{
  UString name_attr = attrib("name"_u);
  UString closed_attr = attrib("closed"_u);
  newDefTag(name_attr);
  if(closed_attr != "true"_u)
  {
    open_class->insert((*tag_index)["TAG_"_u + name_attr]);
  }

  while(type != XML_READER_TYPE_END_ELEMENT || name != "def-mult"_u)
  {
    step();
    if(name == "sequence"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	plist->beginSequence();
	while(type != XML_READER_TYPE_END_ELEMENT || name != "sequence"_u)
	{
	  step();
	  if(name == "label-item"_u)
	  {
	    if(type != XML_READER_TYPE_END_ELEMENT)
	    {
	      plist->insert((*tag_index)["TAG_"_u + name_attr],
                            (*tag_index)["TAG_"_u + attrib("label"_u)]);
	    }
	  }
	  else if(name == "tags-item"_u)
	  {
	    if(type != XML_READER_TYPE_END_ELEMENT)
	    {
	      plist->insert((*tag_index)["TAG_"_u + name_attr],
			    attrib("lemma"_u), attrib("tags"_u));
	    }
	  }
	  else if(name == "sequence"_u)
	  {
	    break;
	  }
	  else if(name == "#text"_u)
	  {
	    // do nothing
	  }
	  else if(name == "#comment"_u)
	  {
	    // do nothing
          }
	}
	plist->endSequence();
      }
    }
    else if(name == "#text"_u)
    {
      // do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else if(name == "def-mult"_u)
    {
      // do nothing
    }
    else
    {
      unexpectedTag();
    }
  }
}

void
TSXReader::procTagset()
{
  while(type == XML_READER_TYPE_END_ELEMENT || name != "tagset"_u)
  {
    step();
    if(name != "#text"_u && name != "tagger"_u && name != "tagset"_u && name != "#comment"_u)
    {
      unexpectedTag();
    }
  }

  while(type != XML_READER_TYPE_END_ELEMENT || name != "tagset"_u)
  {
    step();
    if(name == "def-label"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	procDefLabel();
      }
    }
    else if(name == "def-mult"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        procDefMult();
      }
    }
    else if(name == "#text"_u)
    {
      // do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else if(name == "tagset"_u)
    {
      // do nothing
    }
    else
    {
      unexpectedTag();
    }
  }
}


void
TSXReader::procLabelSequence()
{
  TForbidRule forbid_rule;

  step();
  while(name == "#text"_u || name == "#comment"_u)
  {
    step();
  }
  if(name != "label-item"_u)
  {
    parseError("<label-item> tag expected"_u);
  }

  forbid_rule.tagi = (*tag_index)["TAG_"_u + attrib("label"_u)];

  step();
  while(name == "#text"_u || name == "#comment"_u)
  {
    step();
  }
  if(name != "label-item"_u)
  {
    parseError("<label-item> tag expected"_u);
  }
  forbid_rule.tagj = (*tag_index)["TAG_"_u + attrib("label"_u)];

  forbid_rules->push_back(forbid_rule);
}

void
TSXReader::procForbid()
{
  while(type != XML_READER_TYPE_END_ELEMENT || name != "forbid"_u)
  {
    step();
    if(name == "label-sequence"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	procLabelSequence();
      }
    }
    else if(name == "#text"_u)
    {
      // do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else if(name == "forbid"_u)
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError("Unexpected '"_u + name + "' open tag"_u);
      }
    }
    else
    {
      parseError("Unexpected '"_u + name + "' tag"_u);
    }
  }
}

void
TSXReader::procEnforce()
{
  TEnforceAfterRule aux;
  while(type != XML_READER_TYPE_END_ELEMENT || name != "enforce-rules"_u)
  {
    step();
    if(name == "enforce-after"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	aux.tagi = (*tag_index)["TAG_"_u + attrib("label"_u)];
      }
      else
      {
	enforce_rules->push_back(aux);
	aux.tagsj.clear();
      }
    }
    else if(name == "label-set"_u)
    {
      // do nothing
    }
    else if(name == "label-item"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	aux.tagsj.push_back((*tag_index)["TAG_"_u + attrib("label"_u)]);
      }
    }
    else if(name == "#text"_u)
    {
      // do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else if(name == "enforce-rules"_u)
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError("Unexpected 'enforce-rules' open tag"_u);
      }
    }
    else
    {
      parseError("Unexpected '"_u + name + "' tag"_u);
    }
  }
}

void
TSXReader::procPreferences()
{
  while(type != XML_READER_TYPE_END_ELEMENT || name != "preferences"_u)
  {
    step();
    if(name == "prefer"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        UString const tags = "<"_u + StringUtils::substitute(attrib("tags"_u), "."_u, "><"_u) + ">"_u;
	prefer_rules->push_back(tags);
      }
    }
    else if(name == "#text"_u)
    {
      //do nothing
    }
    else if(name == "#comment"_u)
    {
      // do nothing
    }
    else if(name == "preferences"_u)
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError("Unexpected 'preferences' open tag"_u);
      }
    }
    else
    {
      parseError("Unexpected '"_u + name + "' tag"_u);
    }
  }
}

void
TSXReader::parse()
{
  open_class->clear();
  forbid_rules->clear();
  clearTagIndex();
  enforce_rules->clear();

  procTagset();

  step();
  while(name == "#text"_u || name == "#comment"_u)
  {
    step();
  }
  if(name == "forbid"_u)
  {
    procForbid();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }
  if(name == "enforce-rules"_u)
  {
    procEnforce();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }
  if(name == "preferences"_u)
  {
    procPreferences();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }
  if(name == "discard-on-ambiguity"_u)
  {
    if(type != XML_READER_TYPE_END_ELEMENT)
    {
      procDiscardOnAmbiguity();
    }
  }

  newConstant("kMOT"_u);
  newConstant("kDOLLAR"_u);
  newConstant("kBARRA"_u);
  newConstant("kMAS"_u);
  newConstant("kIGNORAR"_u);
  newConstant("kBEGIN"_u);
  newConstant("kUNKNOWN"_u);

  plist->insert((*tag_index)["TAG_LPAR"_u], ""_u, "lpar"_u);
  plist->insert((*tag_index)["TAG_RPAR"_u], ""_u, "rpar"_u);
  plist->insert((*tag_index)["TAG_LQUEST"_u], ""_u, "lquest"_u);
  plist->insert((*tag_index)["TAG_CM"_u], ""_u, "cm"_u);
  plist->insert((*tag_index)["TAG_SENT"_u], ""_u, "sent"_u);
//  plist->insert((*tag_index)["TAG_kMAS"_u], "+"_u, ""_u);
  plist->buildTransducer();
}

TaggerData &
TSXReader::getTaggerData()
{
  return tdata;
}
