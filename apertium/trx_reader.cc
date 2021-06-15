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
#include <apertium/trx_reader.h>
#include <lttoolbox/xml_parse_util.h>
#include <lttoolbox/compression.h>

#include <cstdlib>
#include <iostream>
#include <apertium/transfer_regex.h>

UString const TRXReader::ANY_TAG         = "<ANY_TAG>"_u;
UString const TRXReader::ANY_CHAR        = "<ANY_CHAR>"_u;

TRXReader::TRXReader()
{
  td.getAlphabet().includeSymbol(ANY_TAG);
  td.getAlphabet().includeSymbol(ANY_CHAR);
}

int
TRXReader::insertLemma(int const base, UString const &lemma)
{
  int retval = base;
  static int const any_char = td.getAlphabet()(ANY_CHAR);
  if(lemma.empty())
  {
    retval = td.getTransducer().insertSingleTransduction(any_char, retval);
    td.getTransducer().linkStates(retval, retval, any_char);
    int another = td.getTransducer().insertSingleTransduction('\\', retval);
    td.getTransducer().linkStates(another, retval, any_char);
  }
  else
  {
    for(unsigned int i = 0, limit = lemma.size();  i != limit; i++)
    {
      if(lemma[i] == '\\')
      {
        retval = td.getTransducer().insertSingleTransduction('\\', retval);
        i++;
        retval = td.getTransducer().insertSingleTransduction(int(lemma[i]),
                                                             retval);
      }
      else if(lemma[i] == '*')
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
TRXReader::insertTags(int const base, UString const &tags)
{
  int retval = base;
  static int const any_tag = td.getAlphabet()(ANY_TAG);
  if(tags.size() != 0)
  {
    for(unsigned int i = 0, limit = tags.size(); i < limit; i++)
    {
      if(tags[i] == '*')
      {
        retval = td.getTransducer().insertSingleTransduction(any_tag, retval);
        td.getTransducer().linkStates(retval, retval, any_tag);
        i++;
      }
      else
      {
        UString symbol = "<"_u;
        for(unsigned int j = i; j != limit; j++)
        {
          if(tags[j] == '.')
          {
            symbol.append(tags.substr(i, j-i));
            i = j;
            break;
          }
        }

        if(symbol == "<"_u)
        {
          symbol.append(tags.substr(i));
          i = limit;
        }
        symbol += '>';
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
  while(name == "#text"_u || name == "#comment"_u)
  {
    step();
  }

  if(name == "section-def-attrs"_u)
  {
    procDefAttrs();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }

  if(name == "section-def-vars"_u)
  {
    procDefVars();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }

  if(name == "section-def-lists"_u)
  {
    procDefLists();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }

  if(name == "section-def-macros"_u)
  {
    procDefMacros();
    step();
    while(name == "#text"_u || name == "#comment"_u)
    {
      step();
    }
  }

  if(name == "section-rules"_u)
  {
    procRules();
    step();
    while(name == "#text"_u || name == "#comment"_u)
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
	name != "section-rules"_u)
  {
    step();
    if(name == "rule"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        count++;
      }
    }
    else if(name == "pattern"_u)
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
            cerr << "Warning (" << xmlTextReaderGetParserLineNumber(reader);
            cerr << "): "
              << "Paths to rule " << count << " blocked by rule " << td.seen_rules[*it]
              << "." << endl;

          }
        }
      }
    }
    else if(name == "pattern-item"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        pair<multimap<UString, LemmaTags>::iterator,
             multimap<UString, LemmaTags>::iterator> range;

        range = cat_items.equal_range(attrib("n"_u));

        if(range.first == range.second)
        {
          parseError("Undefined cat-item '"_u + attrib("n"_u));
        }

// new code

        set<int> alive_states_new;

        for(; range.first != range.second; range.first++)
        {
          for(set<int>::iterator it = alive_states.begin(), limit = alive_states.end();
              it != limit; it++)
          {
            // mark of begin of word
            int tmp = td.getTransducer().insertSingleTransduction('^', *it);
            if(*it != td.getTransducer().getInitial())
            {
              // insert optional blank between two words
              int alt = td.getTransducer().insertSingleTransduction(' ', *it);
              td.getTransducer().linkStates(alt, tmp, '^');
            }

            // insert word
            tmp = insertLemma(tmp, range.first->second.lemma);
            tmp = insertTags(tmp, range.first->second.tags);

            // insert mark of end of word
            tmp = td.getTransducer().insertSingleTransduction('$', tmp);

            // set as alive_state
            alive_states_new.insert(tmp);
          }
        }

        // copy new alive states on alive_states set
        alive_states = alive_states_new;
      }
    }
    else if(name == "let"_u)
    {
      int count = 0;
      int lineno = xmlTextReaderGetParserLineNumber(reader);
      while(name != "let"_u || type != XML_READER_TYPE_END_ELEMENT)
      {
        step();
        if(type == XML_ELEMENT_NODE)
        {
          count++;

          if(name == "clip"_u && attrib("side"_u) == "sl"_u)
          {
            cerr << "Warning (" << lineno;
            cerr << "): assignment to 'sl' side has no effect." << endl;
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
    cerr << "Error: cannot open '" << filename;
    cerr << "' for writing" << endl;
    exit(EXIT_FAILURE);
  }

  td.write(out);

  fclose(out);
}

void
TRXReader::procDefAttrs()
{
  UString attrname;
  vector<UString> items;

  while(type != XML_READER_TYPE_END_ELEMENT ||
        name != "section-def-attrs"_u)
  {
    step();
    if(name == "attr-item"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        items.push_back(attrib("tags"_u));
      }
    }
    else if(name == "def-attr"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        attrname = attrib("n"_u);
      }
      else
      {
        td.getAttrItems()[attrname] = optimize_regex(items);
        items.clear();
        attrname.clear();
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
    else if(name == "section-def-attrs"_u)
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
  while(type == XML_READER_TYPE_END_ELEMENT || !(name == "transfer"_u || name == "interchunk"_u || name == "postchunk"_u))
  {
    step();
    if(name != "#text"_u && name != "transfer"_u &&  name != "interchunk"_u &&
       name != "postchunk"_u && name != "section-def-cats"_u && name != "#comment"_u)
    {
      unexpectedTag();
    }
  }

  UString catname;

  while(type != XML_READER_TYPE_END_ELEMENT ||
        name != "section-def-cats"_u)
  {
    step();
    if(name == "cat-item"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        if(!attrib("tags"_u).empty())
        {
          insertCatItem(catname, attrib("lemma"_u), attrib("tags"_u));
        }
        else
        {
          insertCatItem(catname, attrib("name"_u), ""_u);
        }
      }
    }
    else if(name == "def-cat"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        catname = attrib("n"_u);
      }
      else
      {
        catname.clear();
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
    else if(name == "section-def-cats"_u)
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
        name != "section-def-vars"_u)
  {
    step();
    if(name == "def-var"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        createVar(attrib("n"_u), attrib("v"_u));
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
    else if(name == "section-def-vars"_u)
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
  UString listname;

  while(type != XML_READER_TYPE_END_ELEMENT ||
	name != "section-def-lists"_u)
  {
    step();
    if(name == "list-item"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        insertListItem(listname, attrib("v"_u));
      }
    }
    else if(name == "def-list"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        listname = attrib("n"_u);
      }
      else
      {
        listname.clear();
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
    else if(name == "section-def-lists"_u)
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
	name != "section-def-macros"_u)
  {
    step();
    if(name == "def-macro"_u)
    {
      if(type != XML_READER_TYPE_END_ELEMENT)
      {
        createMacro(attrib("n"_u), count++);
      }
    }
  }
}

void
TRXReader::createMacro(UString const &name, int const value)
{
  if(td.getMacros().find(name) != td.getMacros().end())
  {
    parseError("Macro '"_u + name + "' defined at least twice"_u);
  }
  td.getMacros()[name] = value;
}

void
TRXReader::insertListItem(UString const &name, UString const &value)
{
  td.getLists()[name].insert(value);
}

void
TRXReader::createVar(UString const &name, UString const &initial_value)
{
  td.getVariables()[name] = initial_value;
}

void
TRXReader::insertCatItem(UString const &name, UString const &lemma,
                         UString const &tags)
{
  LemmaTags lt;
  lt.lemma = lemma;
  lt.tags = tags;
  cat_items.insert(pair<UString, LemmaTags>(name, lt));
}
