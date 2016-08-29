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
#include <apertium/tsx_reader.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/compression.h>
#include <apertium/string_utils.h>

#include <cstdlib>
#include <iostream>
#include <apertium/string_utils.h>

using namespace Apertium;
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
  newTagIndex(L"LPAR");
  newTagIndex(L"RPAR");
  newTagIndex(L"LQUEST");
  newTagIndex(L"CM");
  newTagIndex(L"SENT");
  newTagIndex(L"kEOF");
  newTagIndex(L"kUNDEF");
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
TSXReader::newTagIndex(wstring const &tag)
{
  if(tag_index->find(L"TAG_" + tag) != tag_index->end())
  {
    parseError(L"'" + tag + L"' already defined");
  }

  array_tags->push_back(L"TAG_" + tag);
  (*tag_index)[L"TAG_" + tag] = array_tags->size() - 1;
}

void
TSXReader::newDefTag(wstring const &tag)
{
  if(tag_index->find(L"TAG_" + tag) != tag_index->end())
  {
    parseError(L"'" + tag + L"' already defined");
  }

  array_tags->push_back(tag);
  (*tag_index)[L"TAG_" + tag] = array_tags->size() - 1;
}

void
TSXReader::newConstant(wstring const &constant)
{
  constants->setConstant(constant, array_tags->size());
  array_tags->push_back(constant);
}

void
TSXReader::procDiscardOnAmbiguity()
{
  while(type != XML_READER_TYPE_END_ELEMENT || name != L"discard-on-ambiguity")
  {
    step();

    if(name == L"discard")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        tdata.addDiscard(L"<" + StringUtils::substitute(attrib(L"tags"), L".", L"><") + L">");
      }
    }
    else if(name == L"#text")
    {
      // do nothing
    }
    else if(name == L"#comment")
    {
      // do nothing
    }
    else if(name == L"discard-on-ambiguity")
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError(L"Unexpected 'discard-on-ambiguity' open tag");
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
  wstring name_attr = attrib(L"name");
  wstring closed_attr = attrib(L"closed");
  newDefTag(name_attr);

  if(closed_attr != L"true")
  {
    open_class->insert((*tag_index)[L"TAG_"+name_attr]);
  }

  while(type != XML_READER_TYPE_END_ELEMENT || name != L"def-label")
  {
    step();

    if(name == L"tags-item")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	plist->insert((*tag_index)[L"TAG_"+name_attr], attrib(L"lemma"),
		     attrib(L"tags"));
      }
    }
    else if(name == L"def-label")
    {
      return;
    }
    else if(name == L"#text")
    {
      // do nothing
    }
    else if(name == L"#comment")
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
  wstring name_attr = attrib(L"name");
  wstring closed_attr = attrib(L"closed");
  newDefTag(name_attr);
  if(closed_attr != L"true")
  {
    open_class->insert((*tag_index)[L"TAG_"+name_attr]);
  }

  while(type != XML_READER_TYPE_END_ELEMENT || name != L"def-mult")
  {
    step();
    if(name == L"sequence")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	plist->beginSequence();
	while(type != XML_READER_TYPE_END_ELEMENT || name != L"sequence")
	{
	  step();
	  if(name == L"label-item")
	  {
	    if(type != XML_READER_TYPE_END_ELEMENT)
	    {
	      plist->insert((*tag_index)[L"TAG_"+name_attr],
                            (*tag_index)[L"TAG_"+attrib(L"label")]);
	    }
	  }
	  else if(name == L"tags-item")
	  {
	    if(type != XML_READER_TYPE_END_ELEMENT)
	    {
	      plist->insert((*tag_index)[L"TAG_"+name_attr],
			    attrib(L"lemma"), attrib(L"tags"));
	    }
	  }
	  else if(name == L"sequence")
	  {
	    break;
	  }
	  else if(name == L"#text")
	  {
	    // do nothing
	  }
	  else if(name == L"#comment")
	  {
	    // do nothing
          }
	}
	plist->endSequence();
      }
    }
    else if(name == L"#text")
    {
      // do nothing
    }
    else if(name == L"#comment")
    {
      // do nothing
    }
    else if(name == L"def-mult")
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
  while(type == XML_READER_TYPE_END_ELEMENT || name != L"tagset")
  {
    step();
    if(name != L"#text" && name != L"tagger" && name != L"tagset")
    {
      unexpectedTag();
    }
  }
  
  while(type != XML_READER_TYPE_END_ELEMENT || name != L"tagset")
  {
    step();
    if(name == L"def-label")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	procDefLabel();
      }
    }
    else if(name == L"def-mult")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        procDefMult();
      }
    }
    else if(name == L"#text")
    {
      // do nothing
    }
    else if(name == L"#comment")
    {
      // do nothing
    }
    else if(name == L"tagset")
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
  while(name == L"#text" || name == L"#comment")
  {
    step();
  }
  if(name != L"label-item")
  {
    parseError(L"<label-item> tag expected");
  }
  
  forbid_rule.tagi = (*tag_index)[L"TAG_" + attrib(L"label")];

  step();
  while(name == L"#text" || name == L"#comment")
  {
    step();
  }
  if(name != L"label-item")
  {
    parseError(L"<label-item> tag expected");
  }
  forbid_rule.tagj = (*tag_index)[L"TAG_" + attrib(L"label")];
  
  forbid_rules->push_back(forbid_rule);
}

void
TSXReader::procForbid()
{
  while(type != XML_READER_TYPE_END_ELEMENT || name != L"forbid")
  {
    step();
    if(name == L"label-sequence")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	procLabelSequence();
      }
    }
    else if(name == L"#text")
    {
      // do nothing
    }
    else if(name == L"#comment")
    {
      // do nothing
    }
    else if(name == L"forbid")
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError(L"Unexpected '" + name + L"' open tag");
      }
    }
    else
    {
      parseError(L"Unexpected '" + name + L"' tag");
    }
  }  
}

void
TSXReader::procEnforce()
{
  TEnforceAfterRule aux;
  while(type != XML_READER_TYPE_END_ELEMENT || name != L"enforce-rules")
  {
    step();
    if(name == L"enforce-after")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	aux.tagi = (*tag_index)[L"TAG_" + attrib(L"label")];
      }
      else
      {
	enforce_rules->push_back(aux);
	aux.tagsj.clear();
      }
    }
    else if(name == L"label-set")
    {
      // do nothing
    }
    else if(name == L"label-item")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	aux.tagsj.push_back((*tag_index)[L"TAG_" + attrib(L"label")]);
      }
    }
    else if(name == L"#text")
    {
      // do nothing
    }
    else if(name == L"#comment")
    {
      // do nothing
    }
    else if(name == L"enforce-rules")
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError(L"Unexpected 'enforce-rules' open tag");
      }
    }
    else
    {
      parseError(L"Unexpected '" + name + L"' tag");
    }
  }
}

void
TSXReader::procPreferences()
{
  while(type != XML_READER_TYPE_END_ELEMENT || name != L"preferences")
  {
    step();
    if(name == L"prefer")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        wstring const tags = L"<" + StringUtils::substitute(attrib(L"tags"), L".", L"><") + L">";
	prefer_rules->push_back(tags);
      }
    }
    else if(name == L"#text")
    {
      //do nothing
    }
    else if(name == L"#comment")
    {
      // do nothing
    }
    else if(name == L"preferences")
    {
      if(type == XML_READER_TYPE_END_ELEMENT)
      {
	break;
      }
      else
      {
	parseError(L"Unexpected 'preferences' open tag");
      }
    }
    else
    {
      parseError(L"Unexpected '" + name + L"' tag");
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
  while(name == L"#text" || name == L"#comment")
  {
    step();
  }
  if(name == L"forbid")
  {
    procForbid();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }
  if(name == L"enforce-rules")
  {
    procEnforce();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }
  if(name == L"preferences")
  {
    procPreferences();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }
  if(name == L"discard-on-ambiguity")
  {
    if(type != XML_READER_TYPE_END_ELEMENT)
    {
      procDiscardOnAmbiguity();
    }
  }

  newConstant(L"kMOT");
  newConstant(L"kDOLLAR");
  newConstant(L"kBARRA");
  newConstant(L"kMAS");
  newConstant(L"kIGNORAR");
  newConstant(L"kBEGIN");
  newConstant(L"kUNKNOWN");
  
  plist->insert((*tag_index)[L"TAG_LPAR"], L"", L"lpar");
  plist->insert((*tag_index)[L"TAG_RPAR"], L"", L"rpar");
  plist->insert((*tag_index)[L"TAG_LQUEST"], L"", L"lquest");
  plist->insert((*tag_index)[L"TAG_CM"], L"", L"cm");
  plist->insert((*tag_index)[L"TAG_SENT"], L"", L"sent");
//  plist->insert((*tag_index)[L"TAG_kMAS"], L"+", L"");
  plist->buildTransducer();
}

TaggerData &
TSXReader::getTaggerData()
{
  return tdata;
}
