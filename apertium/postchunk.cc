/*
 * Copyright (C) 2005--2015 Universitat d'Alacant / Universidad de Alicante
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
#include <apertium/postchunk.h>
#include <apertium/trx_reader.h>
#include <apertium/utf_converter.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/xml_parse_util.h>

#include <cctype>
#include <cerrno>
#include <iostream>
#include <stack>
#include <apertium/string_utils.h>
#include "apertium_config.h"
#include <apertium/unlocked_cstdio.h>

using namespace Apertium;
using namespace std;

void
Postchunk::destroy()
{
  if(me)
  {
    delete me;
    me = NULL;
  }
  if(doc)
  {
    xmlFreeDoc(doc);
    doc = NULL;
  }
}

Postchunk::Postchunk() :
word(0),
blank(0),
lword(0),
lblank(0),
output(0),
any_char(0),
any_tag(0),
nwords(0)
{
  me = NULL;
  doc = NULL;
  root_element = NULL;
  lastrule = NULL;
  inword = false;
  null_flush = false;
  internal_null_flush = false;
  trace = false;
}

Postchunk::~Postchunk()
{
  destroy();
}

void
Postchunk::readData(FILE *in)
{
  alphabet.read(in);
  any_char = alphabet(TRXReader::ANY_CHAR);
  any_tag = alphabet(TRXReader::ANY_TAG);

  Transducer t;
  t.read(in, alphabet.size());

  map<int, int> finals;

  // finals
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    int key = Compression::multibyte_read(in);
    finals[key] = Compression::multibyte_read(in);
  }

  me = new MatchExe(t, finals);

  // attr_items
  bool recompile_attrs = Compression::string_read(in) != string(pcre_version());
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    attr_items[cad_k].read(in);
    wstring fallback = Compression::wstring_read(in);
    if(recompile_attrs) {
      attr_items[cad_k].compile(UtfConverter::toUtf8(fallback));
    }
  }

  // variables
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    variables[cad_k] = UtfConverter::toUtf8(Compression::wstring_read(in));
  }

  // macros
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    macros[cad_k] = Compression::multibyte_read(in);
  }

  // lists
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));

    for(int j = 0, limit2 = Compression::multibyte_read(in); j != limit2; j++)
    {
      wstring const cad_v = Compression::wstring_read(in);
      lists[cad_k].insert(UtfConverter::toUtf8(cad_v));
      listslow[cad_k].insert(UtfConverter::toUtf8(StringUtils::tolower(cad_v)));
    }
  }
}

void
Postchunk::read(string const &transferfile, string const &datafile)
{
  readPostchunk(transferfile);

  // datafile
  FILE *in = fopen(datafile.c_str(), "rb");
  if(!in)
  {
    wcerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

}

void
Postchunk::readPostchunk(string const &in)
{
  doc = xmlReadFile(in.c_str(), NULL, 0);

  if(doc == NULL)
  {
    wcerr << "Error: Could not parse file '" << in << "'." << endl;
    exit(EXIT_FAILURE);
  }

  root_element = xmlDocGetRootElement(doc);

  // search for macros & rules
  for(xmlNode *i = root_element->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "section-def-macros"))
      {
        collectMacros(i);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "section-rules"))
      {
        collectRules(i);
      }
    }
  }
}

void
Postchunk::collectRules(xmlNode *localroot)
{
  for(xmlNode *rule = localroot->children; rule != NULL; rule = rule->next)
  {
    if(rule->type == XML_ELEMENT_NODE)
    {
      size_t line = rule->line;
      for(xmlNode *rulechild = rule->children; ; rulechild = rulechild->next)
      {
        if(rulechild->type == XML_ELEMENT_NODE && !xmlStrcmp(rulechild->name, (const xmlChar *) "action"))
        {
          rule_map.push_back(rulechild);
          rule_lines.push_back(line);
          break;
        }
      }
    }
  }
}

void
Postchunk::collectMacros(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      macro_map.push_back(i);
    }
  }
}

bool
Postchunk::checkIndex(xmlNode *element, int index, int limit)
{
  if(index > limit) // Note: Unlike transfer/interchunk, we allow index==limit!
  {
    wcerr << L"Error in " << UtfConverter::fromUtf8((char *) doc->URL) << L": line " << element->line << L": index > limit" << endl;
    return false;
  }
  if(index < 0) {
    wcerr << L"Error in " << UtfConverter::fromUtf8((char *) doc->URL) << L": line " << element->line << L": index < 0" << endl;
    return false;
  }
  if(word[index] == 0)
  {
    wcerr << L"Error in " << UtfConverter::fromUtf8((char *) doc->URL) << L": line " << element->line << L": Null access at word[index]" << endl;
    return false;
  }
  return true;
}


string
Postchunk::evalString(xmlNode *element)
{
  map<xmlNode *, TransferInstr>::iterator it;
  it = evalStringCache.find(element);
  if(it != evalStringCache.end())
  {
    TransferInstr &ti = it->second;
    switch(ti.getType())
    {
      case ti_clip_tl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
        }
        break;

      case ti_lu_count:
        return StringUtils::itoa_string(tmpword.size());

      case ti_var:
        return variables[ti.getContent()];

      case ti_lit_tag:
      case ti_lit:
        return ti.getContent();

      case ti_b:
        if(ti.getPos() >= 0 && checkIndex(element, ti.getPos(), lblank))
        {
          return !blank?"":*(blank[ti.getPos()]);
        }
        else {
          return " ";
        }
        break;

      case ti_get_case_from:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return copycase(word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]),
                          evalString((xmlNode *) ti.getPointer()));
        }
        break;

      case ti_case_of_tl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return caseOf(word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]));
        }
        break;

      default:
        return "";
    }
    return "";
  }

  if(!xmlStrcmp(element->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    xmlChar *part = NULL;

    for(xmlAttr *i = element->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *)i->children->content);
      }
    }

    evalStringCache[element] = TransferInstr(ti_clip_tl, (const char *) part, pos, NULL);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "lit-tag"))
  {
    evalStringCache[element] = TransferInstr(ti_lit_tag,
                                             tags((const char *) element->properties->children->content), 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "lit"))
  {
    evalStringCache[element] = TransferInstr(ti_lit, string((char *) element->properties->children->content), 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "b"))
  {
    if(element->properties == NULL)
    {
      evalStringCache[element] = TransferInstr(ti_b, " ", -1);
    }
    else
    {
      int pos = atoi((const char *) element->properties->children->content) - 1;
      evalStringCache[element] = TransferInstr(ti_b, "", pos);
    }
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "get-case-from"))
  {
    int pos = atoi((const char *) element->properties->children->content);
    xmlNode *param = NULL;
    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
	param = i;
	break;
      }
    }

    evalStringCache[element] = TransferInstr(ti_get_case_from, "lem", pos, param);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "var"))
  {
    evalStringCache[element] = TransferInstr(ti_var, (const char *) element->properties->children->content, 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "lu-count"))
  {
    evalStringCache[element] = TransferInstr(ti_lu_count, "", 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "case-of"))
  {
    int pos = 0;
    xmlChar *part = NULL;

    for(xmlAttr *i = element->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content);
      }
    }

    evalStringCache[element] = TransferInstr(ti_case_of_tl, (const char *) part, pos);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "concat"))
  {
    string value;
    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
        value.append(evalString(i));
      }
    }
    return value;
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "lu"))
  {
    string myword;
    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
       if(i->type == XML_ELEMENT_NODE)
       {
         myword.append(evalString(i));
       }
    }

    if(myword != "")
    {
      return "^"+myword+"$";
    }
    else
    {
      return "";
    }
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "mlu"))
  {
    string value;

    bool first_time = true;

    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
        string myword;

        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
	  {
            myword.append(evalString(j));
	  }
        }

	if(!first_time)
	{
	  if(myword != "" && myword[0] != '#')  //'+#' problem
	  {
	    value.append("+");
          }
	}
	else
	{
	  if(myword != "")
	  {
	    first_time = false;
          }
	}

	value.append(myword);
      }
    }

    if(value != "")
    {
      return "^"+value+"$";
    }
    else
    {
      return "";
    }
  }

  else
  {
    wcerr << "Error: unexpected rvalue expression '" << element->name << "'" << endl;
    exit(EXIT_FAILURE);
  }

  return evalString(element);
}

void
Postchunk::processOut(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "lu"))
      {
        string myword;
        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            myword.append(evalString(j));
          }
        }
        if(myword != "")
        {
          fputwc_unlocked(L'^', output);
          fputws_unlocked(UtfConverter::fromUtf8(myword).c_str(), output);
          fputwc_unlocked(L'$', output);
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "mlu"))
      {
        fputwc_unlocked(L'^', output);
        bool first_time = true;
        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            string myword;
            for(xmlNode *k = j->children; k != NULL; k = k->next)
            {
              if(k->type == XML_ELEMENT_NODE)
              {
                myword.append(evalString(k));
              }
            }

            if(!first_time)
            {
              if(myword != "")
              {
                fputwc_unlocked('+', output);
              }
            }
	    else
	    {
	      if(myword != "")
	      {
	        first_time = false;
              }
	    }
	    fputws_unlocked(UtfConverter::fromUtf8(myword).c_str(), output);
	  }
        }
        fputwc_unlocked(L'$', output);
      }
      else // 'b'
      {
        fputws_unlocked(UtfConverter::fromUtf8(evalString(i)).c_str(), output);
      }
    }
  }
}

void
Postchunk::processTags(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (xmlChar const *) "tag"))
      {
        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            fputws_unlocked(UtfConverter::fromUtf8(evalString(j)).c_str(), output);
          }
        }
      }
    }
  }
}

void
Postchunk::processInstruction(xmlNode *localroot)
{
  if(!xmlStrcmp(localroot->name, (const xmlChar *) "choose"))
  {
    processChoose(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "let"))
  {
    processLet(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "append"))
  {
    processAppend(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "out"))
  {
    processOut(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "call-macro"))
  {
    processCallMacro(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "modify-case"))
  {
    processModifyCase(localroot);
  }
}

void
Postchunk::processLet(xmlNode *localroot)
{
  xmlNode *leftSide = NULL, *rightSide = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(leftSide == NULL)
      {
	leftSide = i;
      }
      else
      {
	rightSide = i;
	break;
      }
    }
  }

  map<xmlNode *, TransferInstr>::iterator it = evalStringCache.find(leftSide);
  if(it != evalStringCache.end())
  {
    TransferInstr &ti = it->second;
    switch(ti.getType())
    {
      case ti_var:
        variables[ti.getContent()] = evalString(rightSide);
        return;

      case ti_clip_tl:
        word[ti.getPos()]->setChunkPart(attr_items[ti.getContent()], evalString(rightSide));
        return;

      default:
        return;
    }
  }
  if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    string const val = (const char *) leftSide->properties->children->content;
    variables[val] = evalString(rightSide);
    evalStringCache[leftSide] = TransferInstr(ti_var, val, 0);
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    xmlChar *part = NULL;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content);
      }
    }


    word[pos]->setChunkPart(attr_items[(const char *) part],
			    evalString(rightSide));
    evalStringCache[leftSide] = TransferInstr(ti_clip_tl, (const char *) part,
					      pos, NULL);
  }
}

void
Postchunk::processAppend(xmlNode *localroot)
{
  string name;
  for(xmlAttr *i = localroot->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "n"))
    {
      name = (char *) i->children->content;
      break;
    }
  }

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      variables[name].append(evalString(i));
    }
  }
}

void
Postchunk::processModifyCase(xmlNode *localroot)
{
  xmlNode *leftSide = NULL, *rightSide = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(leftSide == NULL)
      {
	leftSide = i;
      }
      else
      {
	rightSide = i;
	break;
      }
    }
  }

  if(!xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    xmlChar *part = NULL;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content);
      }
    }

    string const result = copycase(evalString(rightSide),
				   word[pos]->chunkPart(attr_items[(const char *) part]));
    word[pos]->setChunkPart(attr_items[(const char *) part], result);

  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    string const val = (const char *) leftSide->properties->children->content;
    variables[val] = copycase(evalString(rightSide), variables[val]);
  }
}

void
Postchunk::processCallMacro(xmlNode *localroot)
{
  const char *n = (const char *) localroot->properties->children->content;
  int npar = 0;

  xmlNode *macro = macro_map[macros[n]];

  for(xmlAttr *i = macro->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "npar"))
    {
      npar = atoi((const char *) i->children->content);
      break;
    }
  }

  if (npar <= 0)
  {
    throw "Postchunk::processCallMacro() assumes npar > 0, but got npar <= 0";
  }

  InterchunkWord **myword = NULL;
  if(npar > 0)
  {
    myword = new InterchunkWord *[npar+1];
  }
  string **myblank = NULL;
  if(npar > 0)
  {
    myblank = new string *[npar];
  }

  myword[0] = word[0];

  bool indexesOK = true;
  int idx = 1;
  int lastpos = 0;
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      int pos = atoi((const char *) i->properties->children->content);
      if(!checkIndex(localroot, pos, lword)) {
        indexesOK = false;      // avoid segfaulting on empty chunks, e.g. ^x<x>{}$
        pos = 1;
      }
      myword[idx] = word[pos];
      if(blank)
      {
        myblank[idx-1] = blank[lastpos];
      }

      idx++;
      lastpos = pos;
    }
  }

  swap(myword, word);
  swap(myblank, blank);
  swap(npar, lword);

  if(indexesOK) {
    for(xmlNode *i = macro->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
        processInstruction(i);
      }
    }
  }
  else {
    wcerr << "Warning: Not calling macro \"" << n << "\" from line " << localroot->line << " (empty word?)" << endl;
  }

  swap(myword, word);
  swap(myblank, blank);
  swap(npar, lword);

  delete[] myword;
  delete[] myblank;
}

void
Postchunk::processChoose(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "when"))
      {
        bool picked_option = false;

	for(xmlNode *j = i->children; j != NULL; j = j->next)
	{
	  if(j->type == XML_ELEMENT_NODE)
	  {
	    if(!xmlStrcmp(j->name, (const xmlChar *) "test"))
	    {
	      if(!processTest(j))
	      {
		break;
	      }
	      else
	      {
	        picked_option = true;
              }
	    }
	    else
	    {
	      processInstruction(j);
	    }
	  }
	}
        if(picked_option)
        {
          return;
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "otherwise"))
      {
	for(xmlNode *j = i->children; j != NULL; j = j->next)
	{
	  if(j->type == XML_ELEMENT_NODE)
	  {
	    processInstruction(j);
	  }
	}
      }
    }
  }
}

bool
Postchunk::processLogical(xmlNode *localroot)
{
  if(!xmlStrcmp(localroot->name, (const xmlChar *) "equal"))
  {
    return processEqual(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "begins-with"))
  {
    return processBeginsWith(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "begins-with-list"))
  {
    return processBeginsWithList(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "ends-with"))
  {
    return processEndsWith(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "ends-with-list"))
  {
    return processEndsWithList(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "contains-substring"))
  {
    return processContainsSubstring(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "or"))
  {
    return processOr(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "and"))
  {
    return processAnd(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "not"))
  {
    return processNot(localroot);
  }
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "in"))
  {
    return processIn(localroot);
  }

  return false;
}

bool
Postchunk::processIn(xmlNode *localroot)
{
  xmlNode *value = NULL;
  xmlChar *idlist = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(value == NULL)
      {
	value = i;
      }
      else
      {
	idlist = i->properties->children->content;
	break;
      }
    }
  }

  string sval = evalString(value);

  if(localroot->properties != NULL)
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      set<string, Ltstr> &myset = listslow[(const char *) idlist];
      if(myset.find(tolower(sval)) != myset.end())
      {
	return true;
      }
      else
      {
	return false;
      }
    }
  }

  set<string, Ltstr> &myset = lists[(const char *) idlist];
  if(myset.find(sval) != myset.end())
  {
    return true;
  }
  else
  {
    return false;
  }
}

bool
Postchunk::processTest(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      return processLogical(i);
    }
  }
  return false;
}

bool
Postchunk::processAnd(xmlNode *localroot)
{
  bool val = true;
  for(xmlNode *i = localroot->children; val && i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      val = val && processLogical(i);
    }
  }

  return val;
}

bool
Postchunk::processOr(xmlNode *localroot)
{
  bool val = false;
  for(xmlNode *i = localroot->children; !val && i != NULL ; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      val = val || processLogical(i);
    }
  }

  return val;
}

bool
Postchunk::processNot(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      return !processLogical(i);
    }
  }
  return false;
}

bool
Postchunk::processEqual(xmlNode *localroot)
{
  xmlNode *first = NULL, *second = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(first == NULL)
      {
        first = i;
      }
      else
      {
	second = i;
	break;
      }
    }
  }

  if(localroot->properties == NULL)
  {
    return evalString(first) == evalString(second);
  }
  else
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      return tolower(evalString(first)) == tolower(evalString(second));
    }
    else
    {
      return evalString(first) == evalString(second);
    }
  }
}

bool
Postchunk::beginsWith(string const &s1, string const &s2) const
{
  int const limit = s2.size(), constraint = s1.size();

  if(constraint < limit)
  {
    return false;
  }
  for(int i = 0; i != limit; i++)
  {
    if(s1[i] != s2[i])
    {
      return false;
    }
  }

  return true;
}

bool
Postchunk::endsWith(string const &s1, string const &s2) const
{
  int const limit = s2.size(), constraint = s1.size();

  if(constraint < limit)
  {
    return false;
  }
  for(int i = limit-1, j = constraint - 1; i >= 0; i--, j--)
  {
    if(s1[j] != s2[i])
    {
      return false;
    }
  }

  return true;
}


bool
Postchunk::processBeginsWith(xmlNode *localroot)
{
  xmlNode *first = NULL, *second = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(first == NULL)
      {
        first = i;
      }
      else
      {
	second = i;
	break;
      }
    }
  }

  if(localroot->properties == NULL)
  {
    return beginsWith(evalString(first), evalString(second));
  }
  else
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      return beginsWith(tolower(evalString(first)), tolower(evalString(second)));
    }
    else
    {
      return beginsWith(evalString(first), evalString(second));
    }
  }
}

bool
Postchunk::processEndsWith(xmlNode *localroot)
{
  xmlNode *first = NULL, *second = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(first == NULL)
      {
        first = i;
      }
      else
      {
	second = i;
	break;
      }
    }
  }

  if(localroot->properties == NULL)
  {
    return endsWith(evalString(first), evalString(second));
  }
  else
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      return endsWith(tolower(evalString(first)), tolower(evalString(second)));
    }
    else
    {
      return endsWith(evalString(first), evalString(second));
    }
  }
}

bool
Postchunk::processBeginsWithList(xmlNode *localroot)
{
  xmlNode *first = NULL, *second = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(first == NULL)
      {
        first = i;
      }
      else
      {
	second = i;
	break;
      }
    }
  }

  xmlChar *idlist = second->properties->children->content;
  string needle = evalString(first);
  set<string, Ltstr>::iterator it, limit;

  if(localroot->properties == NULL ||
     xmlStrcmp(localroot->properties->children->content, (const xmlChar *) "yes"))
  {
    it = lists[(const char *) idlist].begin();
    limit = lists[(const char *) idlist].end();
  }
  else
  {
    needle = tolower(needle);
    it = listslow[(const char *) idlist].begin();
    limit = listslow[(const char *) idlist].end();
  }

  for(; it != limit; it++)
  {
    if(beginsWith(needle, *it))
    {
      return true;
    }
  }
  return false;
}

bool
Postchunk::processEndsWithList(xmlNode *localroot)
{
  xmlNode *first = NULL, *second = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(first == NULL)
      {
        first = i;
      }
      else
      {
	second = i;
	break;
      }
    }
  }

  xmlChar *idlist = second->properties->children->content;
  string needle = evalString(first);
  set<string, Ltstr>::iterator it, limit;

  if(localroot->properties == NULL ||
     xmlStrcmp(localroot->properties->children->content, (const xmlChar *) "yes"))
  {
    it = lists[(const char *) idlist].begin();
    limit = lists[(const char *) idlist].end();
  }
  else
  {
    needle = tolower(needle);
    it = listslow[(const char *) idlist].begin();
    limit = listslow[(const char *) idlist].end();
  }

  for(; it != limit; it++)
  {
    if(endsWith(needle, *it))
    {
      return true;
    }
  }
  return false;
}


bool
Postchunk::processContainsSubstring(xmlNode *localroot)
{
  xmlNode *first = NULL, *second = NULL;

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(first == NULL)
      {
        first = i;
      }
      else
      {
	second = i;
	break;
      }
    }
  }

  if(localroot->properties == NULL)
  {
    return evalString(first).find(evalString(second)) != string::npos;
  }
  else
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      return tolower(evalString(first)).find(tolower(evalString(second))) != string::npos;
    }
    else
    {
      return evalString(first).find(evalString(second)) != string::npos;
    }
  }
}

string
Postchunk::copycase(string const &source_word, string const &target_word)
{
  wstring result;
  wstring const s_word = UtfConverter::fromUtf8(source_word);
  wstring const t_word = UtfConverter::fromUtf8(target_word);

  bool firstupper = iswupper(s_word[0]);
  bool uppercase = firstupper && iswupper(s_word[s_word.size()-1]);
  bool sizeone = s_word.size() == 1;

  if(!uppercase || (sizeone && uppercase))
  {
    result = StringUtils::tolower(t_word);
  }
  else
  {
    result = StringUtils::toupper(t_word);
  }

  if(firstupper)
  {
    result[0] = towupper(result[0]);
  }

  return UtfConverter::toUtf8(result);
}

string
Postchunk::caseOf(string const &str)
{
  wstring const s = UtfConverter::fromUtf8(str);

  if(s.size() > 1)
  {
    if(!iswupper(s[0]))
    {
      return "aa";
    }
    else if(!iswupper(s[s.size()-1]))
    {
      return "Aa";
    }
    else
    {
      return "AA";
    }
  }
  else if(s.size() == 1)
  {
    if(!iswupper(s[0]))
    {
      return "aa";
    }
    else
    {
      return "Aa";
    }
  }
  else
  {
    return "aa";
  }
}

wstring
Postchunk::caseOf(wstring const &str)
{
  if(str.size() > 1)
  {
    if(!iswupper(str[0]))
    {
      return L"aa";
    }
    else if(!iswupper(str[str.size()-1]))
    {
      return L"Aa";
    }
    else
    {
      return L"AA";
    }
  }
  else if(str.size() == 1)
  {
    if(!iswupper(str[0]))
    {
      return L"aa";
    }
    else
    {
      return L"Aa";
    }
  }
  else
  {
    return L"aa";
  }
}

string
Postchunk::tolower(string const &str) const
{
  return UtfConverter::toUtf8(StringUtils::tolower(UtfConverter::fromUtf8(str)));
}

string
Postchunk::tags(string const &str) const
{
  string result = "<";

  for(unsigned int i = 0, limit = str.size(); i != limit; i++)
  {
    if(str[i] == '.')
    {
      result.append("><");
    }
    else
    {
      result += str[i];
    }
  }

  result += '>';

  return result;
}

void
Postchunk::processRule(xmlNode *localroot)
{
  // localroot is suposed to be an 'action' tag
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      processInstruction(i);
    }
  }
}

TransferToken &
Postchunk::readToken(FILE *in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  wstring content;
  while(true)
  {
    int val = fgetwc_unlocked(in);
    if(feof(in) || (internal_null_flush && val == 0))
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == L'\\')
    {
      content += L'\\';
      content += wchar_t(fgetwc_unlocked(in));
    }
    else if(val == L'[')
    {
      content += L'[';
      while(true)
      {
	int val2 = fgetwc_unlocked(in);
	if(val2 == L'\\')
	{
	  content += L'\\';
	  content += wchar_t(fgetwc_unlocked(in));
	}
	else if(val2 == L']')
	{
	  content += L']';
	  break;
	}
	else
	{
	  content += wchar_t(val2);
	}
      }
    }
    else if(inword && val == L'{')
    {
      content += L'{';
      while(true)
      {
	int val2 = fgetwc_unlocked(in);
	if(val2 == L'\\')
	{
	  content += L'\\';
	  content += wchar_t(fgetwc_unlocked(in));
	}
	else if(val2 == L'}')
	{
	  int val3 = wchar_t(fgetwc_unlocked(in));
	  ungetwc(val3, in);

	  content += L'}';
	  if(val3 == L'$')
	  {
	    break;
	  }
	}
	else
	{
	  content += wchar_t(val2);
	}
      }
    }
    else if(inword && val == L'$')
    {
      inword = false;
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == L'^')
    {
      inword = true;
      return input_buffer.add(TransferToken(content, tt_blank));
    }
    else
    {
      content += wchar_t(val);
    }
  }
}

bool
Postchunk::getNullFlush(void)
{
  return null_flush;
}

void
Postchunk::setNullFlush(bool null_flush)
{
  this->null_flush = null_flush;
}

void
Postchunk::setTrace(bool trace)
{
  this->trace = trace;
}

void
Postchunk::postchunk_wrapper_null_flush(FILE *in, FILE *out)
{
  null_flush = false;
  internal_null_flush = true;

  while(!feof(in))
  {
    postchunk(in, out);
    fputwc_unlocked(L'\0', out);
    int code = fflush(out);
    if(code != 0)
    {
      wcerr << L"Could not flush output " << errno << endl;
    }
  }

  internal_null_flush = false;
  null_flush = true;
}

void
Postchunk::postchunk(FILE *in, FILE *out)
{
  if(getNullFlush())
  {
    postchunk_wrapper_null_flush(in, out);
  }

  unsigned int last = input_buffer.getPos();

  output = out;
  ms.init(me->getInitial());

  while(true)
  {
    if(ms.size() == 0)
    {
      if(lastrule != NULL)
      {
	applyRule();
	input_buffer.setPos(last);
      }
      else
      {
	if(tmpword.size() != 0)
	{
	  unchunk(*tmpword[0], output);
	  tmpword.clear();
	  input_buffer.setPos(last);
	  input_buffer.next();
	  last = input_buffer.getPos();
	  ms.init(me->getInitial());
	}
	else if(tmpblank.size() != 0)
	{
	  fputws_unlocked(tmpblank[0]->c_str(), output);
	  tmpblank.clear();
	  last = input_buffer.getPos();
	  ms.init(me->getInitial());
	}
      }
    }
    int val = ms.classifyFinals(me->getFinals());
    if(val != -1)
    {
      size_t lastrule_line = rule_lines[val-1];
      lastrule = rule_map[val-1];
      last = input_buffer.getPos();

      if(trace)
      {
        wcerr << endl << L"apertium-postchunk: Rule " << val << L" line " << lastrule_line << L" ";
        for (unsigned int ind = 0; ind < tmpword.size(); ind++)
        {
          if (ind != 0)
          {
            wcerr << L" ";
          }
          fputws_unlocked(tmpword[ind]->c_str(), stderr);
        }
        wcerr << endl;
      }
    }

    TransferToken &current = readToken(in);

    switch(current.getType())
    {
      case tt_word:
	applyWord(current.getContent());
        tmpword.push_back(&current.getContent());
	break;

      case tt_blank:
	ms.step(L' ');
	tmpblank.push_back(&current.getContent());
	break;

      case tt_eof:
	if(tmpword.size() != 0)
	{
	  tmpblank.push_back(&current.getContent());
	  ms.clear();
	}
	else
	{
	  fputws_unlocked(current.getContent().c_str(), output);
	  return;
	}
	break;

      default:
	wcerr << "Error: Unknown input token." << endl;
	return;
    }
  }
}

void
Postchunk::applyRule()
{
  wstring const chunk = *tmpword[0];
  tmpword.clear();
  splitWordsAndBlanks(chunk, tmpword, tmpblank);

  word = new InterchunkWord *[tmpword.size()+1];
  lword = tmpword.size();
  word[0] = new InterchunkWord(UtfConverter::toUtf8(wordzero(chunk)));

  for(unsigned int i = 1, limit = tmpword.size()+1; i != limit; i++)
  {
    if(i == 1)
    {
      if(limit != 2)
      {
        blank = new string *[limit - 2];
        lblank = limit - 3;
      }
      else
      {
        blank = NULL;
        lblank = 0;
      }
    }
    else
    {
      blank[i-2] = new string(UtfConverter::toUtf8(*tmpblank[i-1]));
    }

    word[i] = new InterchunkWord(UtfConverter::toUtf8(*tmpword[i-1]));
  }

  processRule(lastrule);
  lastrule = NULL;

  if(word)
  {
    for(unsigned int i = 0, limit = tmpword.size() + 1; i != limit; i++)
    {
      delete word[i];
    }
    delete[] word;
  }
  if(blank)
  {
    for(unsigned int i = 0, limit = tmpword.size() - 1; i != limit; i++)
    {
      delete blank[i];
    }
    delete[] blank;
  }
  word = NULL;
  blank = NULL;

  for(unsigned int i = 0, limit = tmpword.size(); i != limit; i++)
  {
    if(i != 0)
    {
      delete tmpblank[i];
    }
    delete tmpword[i];
  }
  tmpword.clear();
  tmpblank.clear();
  ms.init(me->getInitial());
}

void
Postchunk::applyWord(wstring const &word_str)
{
  ms.step(L'^');
  for(unsigned int i = 0, limit = word_str.size(); i < limit; i++)
  {
    switch(word_str[i])
    {
      case L'\\':
        i++;
	ms.step(towlower(word_str[i]), any_char);
	break;

      case L'<':
/*	for(unsigned int j = i+1; j != limit; j++)
	{
	  if(word_str[j] == '>')
	  {
	    int symbol = alphabet(word_str.substr(i, j-i+1));
	    if(symbol)
	    {
	      ms.step(symbol, any_tag);
	    }
	    else
	    {
	      ms.step(any_tag);
	    }
	    i = j;
	    break;
	  }
	}
	break;*/

      case L'{':  // ignore the unmodifiable part of the chunk
        ms.step(L'$');
        return;

      default:
	ms.step(towlower(word_str[i]), any_char);
	break;
    }
  }
  ms.step(L'$');
}

vector<wstring>
Postchunk::getVecTags(wstring const &chunk)
{
  vector<wstring> vectags;

  for(int i = 0, limit = chunk.size(); i != limit; i++)
  {
    if(chunk[i] == L'\\')
    {
      i++;
    }
    else if(chunk[i] == L'<')
    {
      wstring mytag;
      do
      {
        mytag += chunk[i++];
      }
      while(chunk[i] != L'>');
      vectags.push_back(mytag + L'>');
    }
    else if(chunk[i] == L'{')
    {
      break;
    }
  }
  return vectags;
}

int
Postchunk::beginChunk(wstring const &chunk)
{
  for(int i = 0, limit = chunk.size(); i != limit; i++)
  {
    if(chunk[i] == L'\\')
    {
      i++;
    }
    else if(chunk[i] == L'{')
    {
      return i + 1;
    }
  }
  return chunk.size();
}

int
Postchunk::endChunk(wstring const &chunk)
{
  return chunk.size()-2;
}

wstring
Postchunk::wordzero(wstring const &chunk)
{
  for(unsigned int i = 0, limit = chunk.size(); i != limit ;i++)
  {
    if(chunk[i] == L'\\')
    {
      i++;
    }
    else if(chunk[i] == L'{')
    {
      return chunk.substr(0, i);
    }
  }

  return L"";
}

wstring
Postchunk::pseudolemma(wstring const &chunk)
{
  for(unsigned int i = 0, limit = chunk.size(); i != limit ;i++)
  {
    if(chunk[i] == L'\\')
    {
      i++;
    }
    else if(chunk[i] == L'<' || chunk[i] == L'{')
    {
      return chunk.substr(0, i);
    }
  }

  return L"";
}

void
Postchunk::unchunk(wstring const &chunk, FILE *output)
{
  vector<wstring> vectags = getVecTags(chunk);
  wstring case_info = caseOf(pseudolemma(chunk));
  bool uppercase_all = false;
  bool uppercase_first = false;

  if(case_info == L"AA")
  {
    uppercase_all = true;
  }
  else if(case_info == L"Aa")
  {
    uppercase_first = true;
  }

  for(int i = beginChunk(chunk), limit = endChunk(chunk); i < limit; i++)
  {
    if(chunk[i] == L'\\')
    {
      fputwc_unlocked(L'\\', output);
      fputwc_unlocked(chunk[++i], output);
    }
    else if(chunk[i] == L'^')
    {
      fputwc_unlocked(L'^', output);
      while(chunk[++i] != L'$')
      {
        if(chunk[i] == L'\\')
        {
          fputwc_unlocked(L'\\', output);
          fputwc_unlocked(chunk[++i], output);
        }
        else if(chunk[i] == L'<')
        {
          if(iswdigit(chunk[i+1]))
          {
            // replace tag
            unsigned long value = wcstoul(chunk.c_str()+i+1,
					  NULL, 0) - 1;
            //atoi(chunk.c_str()+i+1)-1;
            if(vectags.size() > value)
            {
              fputws_unlocked(vectags[value].c_str(), output);
            }
            while(chunk[++i] != L'>');
          }
          else
          {
            fputwc_unlocked(L'<', output);
	    while(chunk[++i] != L'>') fputwc_unlocked(chunk[i], output);
            fputwc_unlocked(L'>', output);
          }
        }
        else
        {
          if(uppercase_all)
          {
            fputwc_unlocked(towupper(chunk[i]), output);
          }
          else if(uppercase_first)
          {
	    if(iswalnum(chunk[i]))
	    {
	      fputwc_unlocked(towupper(chunk[i]), output);
	      uppercase_first = false;
	    }
            else
	    {
	      fputwc_unlocked(chunk[i], output);
	    }
          }
          else
          {
            fputwc_unlocked(chunk[i], output);
          }
        }
      }
      fputwc_unlocked(L'$', output);
    }
    else if(chunk[i] == L'[')
    {
      fputwc_unlocked(L'[', output);
      while(chunk[++i] != L']')
      {
        if(chunk[i] == L'\\')
        {
          fputwc_unlocked(L'\\', output);
          fputwc_unlocked(chunk[++i], output);
        }
        else
        {
          fputwc_unlocked(chunk[i], output);
        }
      }
      fputwc_unlocked(L']', output);
    }
    else
    {
      fputwc_unlocked(chunk[i], output);
    }
  }
}


void
Postchunk::splitWordsAndBlanks(wstring const &chunk, vector<wstring *> &words,
                               vector<wstring *> &blanks)
{
  vector<wstring> vectags = getVecTags(chunk);
  wstring case_info = caseOf(pseudolemma(chunk));
  bool uppercase_all = false;
  bool uppercase_first = false;
  bool lastblank = true;

  if(case_info == L"AA")
  {
    uppercase_all = true;
  }
  else if(case_info == L"Aa")
  {
    uppercase_first = true;
  }

  for(int i = beginChunk(chunk), limit = endChunk(chunk); i < limit; i++)
  {
    if(chunk[i] == L'^')
    {
      if(!lastblank)
      {
        blanks.push_back(new wstring(L""));
      }
      lastblank = false;
      wstring *myword = new wstring();
      wstring &ref = *myword;

      while(chunk[++i] != L'$')
      {
        if(chunk[i] == L'\\')
        {
          ref += L'\\';
          ref += chunk[++i];
        }
        else if(chunk[i] == L'<')
        {
          if(iswdigit(chunk[i+1]))
          {
            // replace tag
            unsigned long value = wcstoul(chunk.c_str()+i+1,
                                          NULL, 0) - 1;
            if(vectags.size() > value)
            {
              ref.append(vectags[value]);
            }
            while(chunk[++i] != L'>');
          }
          else
          {
            ref += L'<';
            while(chunk[++i] != L'>') ref += chunk[i];
            ref += L'>';
          }
        }
        else
        {
          if(uppercase_all)
          {
            ref += towupper(chunk[i]);
          }
          else if(uppercase_first)
          {
            if(iswalnum(chunk[i]))
            {
              ref += towupper(chunk[i]);
              uppercase_first = false;
            }
            else
            {
              ref += chunk[i];
            }
          }
          else
          {
            ref += chunk[i];
          }
        }
      }

      words.push_back(myword);
    }
    else if(chunk[i] == L'[')
    {
      if (!(lastblank && blanks.back()))
      {
        blanks.push_back(new wstring());
      }
      wstring &ref = *(blanks.back());
      ref += L'[';
      while(chunk[++i] != L']')
      {
        if(chunk[i] == L'\\')
        {
          ref += L'\\';
          ref += chunk[++i];
        }
        else
        {
          ref += chunk[i];
        }
      }
      ref += chunk[i];

      lastblank = true;
    }
    else
    {
      if (!lastblank)
      {
        wstring *myblank = new wstring(L"");
        blanks.push_back(myblank);
      }
      wstring &ref = *(blanks.back());
      if(chunk[i] == L'\\')
      {
        ref += L'\\';
        ref += chunk[++i];
      }
      else
      {
        ref += chunk[i];
      }
      lastblank = true;
    }
  }
}

