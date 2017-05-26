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
#include <apertium/trx_reader.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/compression.h>

#include <cstdlib>
#include <iostream>
#include <apertium/string_utils.h>

using namespace Apertium;
wstring const
TRXReader::ANY_TAG = L"<ANY_TAG>";

wstring const
TRXReader::ANY_CHAR = L"<ANY_CHAR>";

TRXReader::TRXReader()
{
  td.getAlphabet().includeSymbol(ANY_TAG);
  td.getAlphabet().includeSymbol(ANY_CHAR);
}

int
TRXReader::insertLemma(int const base, wstring const &lemma)
{
  int retval = base;
  static int const any_char = td.getAlphabet()(ANY_CHAR);
  if(lemma == L"")
  {
    retval = td.getTransducer().insertSingleTransduction(any_char, retval);
    td.getTransducer().linkStates(retval, retval, any_char);
    int another = td.getTransducer().insertSingleTransduction(L'\\', retval);
    td.getTransducer().linkStates(another, retval, any_char);
  }
  else
  {
    for(unsigned int i = 0, limit = lemma.size();  i != limit; i++)
    {
      if(lemma[i] == L'\\')
      {
        retval = td.getTransducer().insertSingleTransduction(L'\\', retval);
	i++;
        retval = td.getTransducer().insertSingleTransduction(int(lemma[i]), 
							     retval);
      }
      else if(lemma[i] == L'*')
      {
	retval = td.getTransducer().insertSingleTransduction(any_char, retval);
	td.getTransducer().linkStates(retval, retval, any_char);
      }
      else
      {
	retval = td.getTransducer().insertSingleTransduction(int(lemma[i]), 
							     retval);
      }
    }
  }
  
  return retval;
}

int
TRXReader::insertTags(int const base, wstring const &tags)
{
  int retval = base;
  static int const any_tag = td.getAlphabet()(ANY_TAG);
  if(tags.size() != 0)
  {
    for(unsigned int i = 0, limit = tags.size(); i < limit; i++)
    {
      if(tags[i] == L'*')
      {
        retval = td.getTransducer().insertSingleTransduction(any_tag, retval);
        td.getTransducer().linkStates(retval, retval, any_tag);
        i++;
      }  
      else
      {
        wstring symbol = L"<";
        for(unsigned int j = i; j != limit; j++)
        {
          if(tags[j] == L'.')
          {  
            symbol.append(tags.substr(i, j-i));
            i = j;
            break;
          }
        }
        
        if(symbol == L"<")
        {
          symbol.append(tags.substr(i));
          i = limit;
        }
        symbol += L'>';
        td.getAlphabet().includeSymbol(symbol);
        retval = td.getTransducer().insertSingleTransduction(td.getAlphabet()(symbol), retval);
      }
    }
  }
  else
  {
    return base; // new line
  }
  
  return retval;
}

void
TRXReader::parse()
{
  procDefCats();
  step();
  while(name == L"#text" || name == L"#comment")
  {
    step();
  }
 
  if(name == L"section-def-attrs")
  {
    procDefAttrs();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }
  
  if(name == L"section-def-vars")
  {
    procDefVars();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }

  if(name == L"section-def-lists")
  {
    procDefLists();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }

  if(name == L"section-def-macros")
  {
    procDefMacros();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }

  if(name == L"section-rules")
  {
    procRules();
    step();
    while(name == L"#text" || name == L"#comment")
    {
      step();
    }
  }  
}

void
TRXReader::procRules()
{
  int count = 0;
  set<int> alive_states;
  
  while(type != XML_READER_TYPE_END_ELEMENT || 
	name != L"section-rules")
  {
    step();
    if(name == L"rule")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
	count++;
      }
    }
    else if(name == L"pattern")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        alive_states.clear();
        alive_states.insert(td.getTransducer().getInitial());
      }
      else
      {
        for(set<int>::iterator it = alive_states.begin(), limit = alive_states.end();
            it != limit; it++)
        {
          if(td.seen_rules.find(*it) == td.seen_rules.end())
          {
            const int symbol = td.countToFinalSymbol(count);
            const int fin = td.getTransducer().insertSingleTransduction(symbol, *it);
            td.getTransducer().setFinal(fin);
            td.seen_rules[*it] = count;
          }
          else
          {
            wcerr << L"Warning (" << xmlTextReaderGetParserLineNumber(reader);
            wcerr << L"): "
              << L"Paths to rule " << count << " blocked by rule " << td.seen_rules[*it]
              << L"." << endl;

          }
        }
      }
    }
    else if(name == L"pattern-item")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        pair<multimap<wstring, LemmaTags, Ltstr>::iterator,
             multimap<wstring, LemmaTags, Ltstr>::iterator> range;

        range = cat_items.equal_range(attrib(L"n"));
      
        if(range.first == range.second)
        {
          parseError(L"Undefined cat-item '" + attrib(L"n"));
        }

// new code
      
        set<int> alive_states_new;
        
        for(; range.first != range.second; range.first++)
        {
          for(set<int>::iterator it = alive_states.begin(), limit = alive_states.end();
              it != limit; it++)
          {
            // mark of begin of word
            int tmp = td.getTransducer().insertSingleTransduction(L'^', *it);
            if(*it != td.getTransducer().getInitial())
            {
              // insert optional blank between two words
              int alt = td.getTransducer().insertSingleTransduction(L' ', *it);
              td.getTransducer().linkStates(alt, tmp, L'^');
            }
            
            // insert word
            tmp = insertLemma(tmp, range.first->second.lemma);
            tmp = insertTags(tmp, range.first->second.tags);
            
            // insert mark of end of word
            tmp = td.getTransducer().insertSingleTransduction(L'$', tmp);
            
            // set as alive_state
            alive_states_new.insert(tmp);
          }
        } 
        
        // copy new alive states on alive_states set
        alive_states = alive_states_new;      
      }
    }
    else if(name == L"let")
    {
      int count = 0;
      int lineno = xmlTextReaderGetParserLineNumber(reader); 
      while(name != L"let" || type != XML_READER_TYPE_END_ELEMENT)
      {
        step();
        if(type == XML_ELEMENT_NODE)
        {
          count++;
          
          if(name == L"clip" && attrib(L"side") == L"sl")
          {
            wcerr << L"Warning (" << lineno;
            wcerr << L"): assignment to 'sl' side has no effect." << endl;
          }    
        }
        
        if(count != 0)
        {
          break;
        }
      }
      
    }
  }
}

void
TRXReader::write(string const &filename)
{
  FILE *out = fopen(filename.c_str(), "wb");
  if(!out)
  {
    wcerr << "Error: cannot open '" << filename;
    wcerr << "' for writing" << endl;
    exit(EXIT_FAILURE);
  }
  
  td.write(out);

  fclose(out);
}

void
TRXReader::procDefAttrs()
{
  wstring attrname;

  while(type != XML_READER_TYPE_END_ELEMENT || 
	name != L"section-def-attrs")
  {
    step();
    if(name == L"attr-item")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        insertAttrItem(attrname, attrib(L"tags"));
      }
    }
    else if(name == L"def-attr")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        attrname = attrib(L"n");
      }
      else
      {
        wstring all = td.getAttrItems()[attrname];
        td.getAttrItems()[attrname] = L"(" + all + L")";
        attrname = L"";
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
    else if(name == L"section-def-attrs")
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
TRXReader::procDefCats()
{
  while(type == XML_READER_TYPE_END_ELEMENT || !(name == L"transfer" || name == L"interchunk" || name == L"postchunk"))
  {
    step();
    if(name != L"#text" && name != L"transfer" &&  name != L"interchunk" &&
       name != L"postchunk" && name != L"section-def-cats" && name != L"#comment")
    {
      unexpectedTag();
    }
  }
  
  wstring catname;

  while(type != XML_READER_TYPE_END_ELEMENT || 
	name != L"section-def-cats")
  {
    step();
    if(name == L"cat-item")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        if(attrib(L"tags") != L"")
        {
          insertCatItem(catname, attrib(L"lemma"), attrib(L"tags"));
        }
        else
        {
          insertCatItem(catname, attrib(L"name"), L"");
        }
      }
    }
    else if(name == L"def-cat")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        catname = attrib(L"n");
      }
      else
      {
        catname = L"";
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
    else if(name == L"section-def-cats")
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
TRXReader::procDefVars()
{
  while(type != XML_READER_TYPE_END_ELEMENT || 
	name != L"section-def-vars")
  {
    step();
    if(name == L"def-var")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        createVar(attrib(L"n"), attrib(L"v"));
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
    else if(name == L"section-def-vars")
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
TRXReader::procDefLists()
{
  wstring listname;

  while(type != XML_READER_TYPE_END_ELEMENT || 
	name != L"section-def-lists")
  {
    step();
    if(name == L"list-item")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        insertListItem(listname, attrib(L"v"));
      }
    }
    else if(name == L"def-list")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        listname = attrib(L"n");
      }
      else
      {
        listname = L"";
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
    else if(name == L"section-def-lists")
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
TRXReader::procDefMacros()
{
  int count = 0;
  while(type != XML_READER_TYPE_END_ELEMENT || 
	name != L"section-def-macros")
  {
    step();
    if(name == L"def-macro")
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        createMacro(attrib(L"n"), count++);
      }
    }
  }
}

void
TRXReader::createMacro(wstring const &name, int const value)
{
  if(td.getMacros().find(name) != td.getMacros().end())
  {
    parseError(L"Macro '" + name + L"' defined at least twice");    
  }
  td.getMacros()[name] = value;
}

void
TRXReader::insertListItem(wstring const &name, wstring const &value)
{
  td.getLists()[name].insert(value);
}

void
TRXReader::createVar(wstring const &name, wstring const &initial_value)
{
  td.getVariables()[name] = initial_value;
}

void
TRXReader::insertCatItem(wstring const &name, wstring const &lemma, 
			 wstring const &tags)
{
  LemmaTags lt;
  lt.lemma = lemma;
  lt.tags = tags;
  cat_items.insert(pair<wstring, LemmaTags>(name, lt));
}

void 
TRXReader::insertAttrItem(wstring const &name, wstring const &tags)
{
  if(td.getAttrItems()[name].size() != 0)
  {
    td.getAttrItems()[name] += L'|';
  }  
  
  td.getAttrItems()[name] += '<';

  for(unsigned int i = 0, limit = tags.size(); i != limit; i++)
  {
    if(tags[i] == L'.')
    {
      td.getAttrItems()[name].append(L"><");
    }
    else
    {
	td.getAttrItems()[name] += tags[i];
    }
  }
  td.getAttrItems()[name] += L'>';
  
}
