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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
 */
#include <apertium/interchunk.h>
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
Interchunk::destroy()
{
  delete me;
  me = NULL;

  if(doc)
  {
    xmlFreeDoc(doc);
    doc = NULL;
  }
}

Interchunk::Interchunk() :
word(0),
lword(0),
last_lword(0),
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
  in_out = false;
}

Interchunk::~Interchunk()
{
  destroy();
}

void
Interchunk::readData(FILE *in)
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
  //bool recompile_attrs = Compression::string_read(in) != pcre_version_endian();
  Compression::string_read(in); // version
  bool recompile_attrs = true;
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    attr_items[cad_k].read(in);
    UString fallback = Compression::string_read(in);
    if(recompile_attrs) {
      //attr_items[cad_k].compile(UtfConverter::toUtf8(fallback));
      // TODO regexs
    }
  }

  // variables
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    variables[cad_k] = Compression::string_read(in);
  }

  // macros
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);
    macros[cad_k] = Compression::multibyte_read(in);
  }

  // lists
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    UString const cad_k = Compression::string_read(in);

    for(int j = 0, limit2 = Compression::multibyte_read(in); j != limit2; j++)
    {
      UString const cad_v = Compression::string_read(in);
      lists[cad_k].insert(cad_v);
      listslow[cad_k].insert(StringUtils::tolower(cad_v));
    }
  }
}

void
Interchunk::read(const char* transferfile, const char* datafile)
{
  readInterchunk(transferfile);

  // datafile
  FILE *in = fopen(datafile, "rb");
  if(!in)
  {
    cerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

}

void
Interchunk::readInterchunk(const char* in)
{
  doc = xmlReadFile(in, NULL, 0);

  if(doc == NULL)
  {
    cerr << "Error: Could not parse file '" << in << "'." << endl;
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
Interchunk::collectRules(xmlNode *localroot)
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
Interchunk::collectMacros(xmlNode *localroot)
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
Interchunk::checkIndex(xmlNode *element, int index, int limit)
{
  if(index >= limit)
  {
    cerr << "Error in " << (char *) doc->URL << ": line " << element->line << ": index >= limit" << endl;
    return false;
  }
  if(index < 0) {
    cerr << "Error in " << (char *) doc->URL << ": line " << element->line << ": index < 0" << endl;
    return false;
  }
  if(word[index] == 0)
  {
    cerr << "Error in " << (char *) doc->URL << ": line " << element->line << ": Null access at word[index]" << endl;
    return false;
  }
  return true;
}


UString
Interchunk::evalString(xmlNode *element)
{
  if (element == 0)
  {
    throw "Interchunk::evalString() was passed a NULL element";
  }

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
          if(ti.getContent() == "content"_u) // jacob's new 'part'
          {
            UString wf = word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
            return wf.substr(1, wf.length()-2); // trim away the { and }
          }
          else
          {
            return word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
          }
        }
        break;

      case ti_var:
        return variables[ti.getContent()];

      case ti_lit_tag:
      case ti_lit:
        return ti.getContent();

      case ti_b:
        if(!blank_queue.empty())
        {
          UString retblank = blank_queue.front();
          
          if(in_out)
          {
            blank_queue.pop();
          }
          
          return retblank;
        }
        else
        {
          return " "_u;
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
        return ""_u;
    }
    return ""_u;
  }

  if(!xmlStrcmp(element->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    UString part;

    for(xmlAttr *i = element->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((char*)i->children->content);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *)i->children->content) - 1;
      }
    }

    evalStringCache[element] = TransferInstr(ti_clip_tl, part, pos, NULL);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "lit-tag"))
  {
    evalStringCache[element] = TransferInstr(ti_lit_tag,
                                             tags(to_ustring((const char *) element->properties->children->content)), 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "lit"))
  {
    evalStringCache[element] = TransferInstr(ti_lit, to_ustring((const char *) element->properties->children->content), 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "b"))
  {
    if(element->properties == NULL)
    {
      evalStringCache[element] = TransferInstr(ti_b, " "_u, -1);
    }
    else
    {
      int pos = atoi((const char *) element->properties->children->content) - 1;
      evalStringCache[element] = TransferInstr(ti_b, ""_u, pos);
    }
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "get-case-from"))
  {
    int pos = atoi((const char *) element->properties->children->content) - 1;
    xmlNode *param = NULL;
    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
	param = i;
	break;
      }
    }

    evalStringCache[element] = TransferInstr(ti_get_case_from, "lem"_u, pos, param);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "var"))
  {
    evalStringCache[element] = TransferInstr(ti_var, to_ustring((const char *) element->properties->children->content), 0);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "case-of"))
  {
    int pos = 0;
    UString part;

    for(xmlAttr *i = element->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((char*)i->children->content);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
        pos = atoi((const char *) i->children->content) - 1;
      }
    }

    evalStringCache[element] = TransferInstr(ti_case_of_tl, part, pos);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "concat"))
  {
    UString value;
    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
        value.append(evalString(i));
      }
    }
    return value;
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "chunk"))
  {
    return processChunk(element);
  }
  else
  {
    cerr << "Error: unexpected rvalue expression '" << element->name << "'" << endl;
    exit(EXIT_FAILURE);
  }

  return evalString(element);
}

void
Interchunk::processOut(xmlNode *localroot)
{
  in_out = true;
  
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "chunk"))
      {
        write(processChunk(i), output);
      }
      else // 'b'
      {
        write(evalString(i), output);
      }
    }
  }
  
  in_out = false;
}

UString
Interchunk::processChunk(xmlNode *localroot)
{
  UString result;
  result.append("^"_u);

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      result.append(evalString(i));
    }
  }

  result.append("$"_u);
  return result;
}

void
Interchunk::processInstruction(xmlNode *localroot)
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
Interchunk::processLet(xmlNode *localroot)
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
      {
        bool match = word[ti.getPos()]->setChunkPart(attr_items[ti.getContent()], evalString(rightSide));
        if(!match && trace)
        {
          cerr << "apertium-interchunk warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
        }
      }
        return;

      default:
        return;
    }
  }
  if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    UString const val = to_ustring((const char *) leftSide->properties->children->content);
    variables[val] = evalString(rightSide);
    evalStringCache[leftSide] = TransferInstr(ti_var, val, 0);
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    UString part;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((char*)i->children->content);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
        pos = atoi((const char *) i->children->content) - 1;
      }
    }


    bool match = word[pos]->setChunkPart(attr_items[part],
					 evalString(rightSide));
    if(!match && trace)
    {
      cerr << "apertium-interchunk warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
    }
    evalStringCache[leftSide] = TransferInstr(ti_clip_tl,
					      part,
					      pos, NULL);
  }
}

void
Interchunk::processAppend(xmlNode *localroot)
{
  UString name;
  for(xmlAttr *i = localroot->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "n"))
    {
      name = to_ustring((char *) i->children->content);
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
Interchunk::processModifyCase(xmlNode *localroot)
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

  if(leftSide->name != NULL && !xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    UString part;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((char*)i->children->content);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
        pos = atoi((const char *) i->children->content) - 1;
      }
    }

    UString const result = copycase(evalString(rightSide),
				   word[pos]->chunkPart(attr_items[part]));
    bool match = word[pos]->setChunkPart(attr_items[part], result);
    if(!match && trace)
    {
      cerr << "apertium-interchunk warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
    }
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    UString const val = to_ustring((const char *) leftSide->properties->children->content);
    variables[val] = copycase(evalString(rightSide), variables[val]);
  }
}

void
Interchunk::processCallMacro(xmlNode *localroot)
{
  UString n = to_ustring((const char *) localroot->properties->children->content);
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

  // ToDo: Is it at all valid if npar <= 0 ?

  InterchunkWord **myword = NULL;
  if(npar > 0)
  {
    myword = new InterchunkWord *[npar];
  }

  int idx = 0;
  for(xmlNode *i = localroot->children; npar && i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      int pos = atoi((const char *) i->properties->children->content)-1;
      myword[idx] = word[pos];
      idx++;
    }
  }

  swap(myword, word);
  swap(npar, lword);

  for(xmlNode *i = macro->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      processInstruction(i);
    }
  }

  swap(myword, word);
  swap(npar, lword);

  delete[] myword;
}

void
Interchunk::processChoose(xmlNode *localroot)
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
Interchunk::processLogical(xmlNode *localroot)
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
Interchunk::processIn(xmlNode *localroot)
{
  xmlNode *value = NULL;
  UString idlist;

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
        idlist = to_ustring((char*)i->properties->children->content);
        break;
      }
    }
  }

  UString sval = evalString(value);

  if(localroot->properties != NULL)
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      set<UString> &myset = listslow[idlist];
      return (myset.find(tolower(sval)) != myset.end());
    }
  }

  set<UString> &myset = lists[idlist];
  return (myset.find(sval) != myset.end());
}

bool
Interchunk::processTest(xmlNode *localroot)
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
Interchunk::processAnd(xmlNode *localroot)
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
Interchunk::processOr(xmlNode *localroot)
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
Interchunk::processNot(xmlNode *localroot)
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
Interchunk::processEqual(xmlNode *localroot)
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
Interchunk::beginsWith(UString const &s1, UString const &s2) const
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
Interchunk::endsWith(UString const &s1, UString const &s2) const
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
Interchunk::processBeginsWith(xmlNode *localroot)
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
Interchunk::processEndsWith(xmlNode *localroot)
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
Interchunk::processBeginsWithList(xmlNode *localroot)
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

  UString idlist = to_ustring((char*)second->properties->children->content);
  UString needle = evalString(first);
  set<UString>::iterator it, limit;

  if(localroot->properties == NULL ||
     xmlStrcmp(localroot->properties->children->content, (const xmlChar *) "yes"))
  {
    it = lists[idlist].begin();
    limit = lists[idlist].end();
  }
  else
  {
    needle = tolower(needle);
    it = listslow[idlist].begin();
    limit = listslow[idlist].end();
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
Interchunk::processEndsWithList(xmlNode *localroot)
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

  UString idlist = to_ustring((char*)second->properties->children->content);
  UString needle = evalString(first);
  set<UString>::iterator it, limit;

  if(localroot->properties == NULL ||
     xmlStrcmp(localroot->properties->children->content, (const xmlChar *) "yes"))
  {
    it = lists[idlist].begin();
    limit = lists[idlist].end();
  }
  else
  {
    needle = tolower(needle);
    it = listslow[idlist].begin();
    limit = listslow[idlist].end();
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
Interchunk::processContainsSubstring(xmlNode *localroot)
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
    return evalString(first).find(evalString(second)) != UString::npos;
  }
  else
  {
    if(!xmlStrcmp(localroot->properties->children->content,
		  (const xmlChar *) "yes"))
    {
      return tolower(evalString(first)).find(tolower(evalString(second))) != UString::npos;
    }
    else
    {
      return evalString(first).find(evalString(second)) != UString::npos;
    }
  }
}

UString
Interchunk::copycase(UString const &source_word, UString const &target_word)
{
  UString result;
  UString const s_word = source_word;
  UString const t_word = target_word;

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

  return result;
}

UString
Interchunk::caseOf(UString const &s)
{
  if(s.size() > 1)
  {
    if(!iswupper(s[0]))
    {
      return "aa"_u;
    }
    else if(!iswupper(s[s.size()-1]))
    {
      return "Aa"_u;
    }
    else
    {
      return "AA"_u;
    }
  }
  else if(s.size() == 1)
  {
    if(!iswupper(s[0]))
    {
      return "aa"_u;
    }
    else
    {
      return "Aa"_u;
    }
  }
  else
  {
    return "aa"_u;
  }
}

UString
Interchunk::tolower(UString const &str) const
{
  return StringUtils::tolower(str);
}

UString
Interchunk::tags(UString const &str) const
{
  UString result = "<"_u;

  for(unsigned int i = 0, limit = str.size(); i != limit; i++)
  {
    if(str[i] == '.')
    {
      result.append("><"_u);
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
Interchunk::processRule(xmlNode *localroot)
{
  // localroot is suposed to be an 'action' tag
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      processInstruction(i);
    }
  }
  
  while(!blank_queue.empty()) //flush remaining blanks that are not spaces
  {
    if(blank_queue.front().compare(" "_u) != 0) {
      write(blank_queue.front(), output);
    }
    blank_queue.pop();
  }
}

TransferToken &
Interchunk::readToken(InputFile& in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  UString content;
  while(true)
  {
    int val = in.get();
    if(in.eof() || (internal_null_flush && val == 0))
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == '\\')
    {
      content += '\\';
      content += in.get();
    }
    else if(val == '[')
    {
      content += '[';
      while(true)
      {
        UChar32 val2 = in.get();
        if(val2 == '\\') {
          content += '\\';
          content += in.get();
        } else if(val2 == ']') {
          content += ']';
          break;
        } else {
          content += val2;
        }
      }
    }
    else if(inword && val == '{') {
      content += '{';
      while(true) {
        UChar32 val2 = in.get();
        if(val2 == '\\') {
          content += '\\';
          content += in.get();
        } else if(val2 == '}') {
          UChar32 val3 = in.peek();
          
          content += '}';
          if(val3 == '$') {
            break;
          }
        } else {
          content += val2;
        }
      }
    }
    else if(inword && val == '$')
    {
      inword = false;
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == '^')
    {
      inword = true;
      return input_buffer.add(TransferToken(content, tt_blank));
    }
    else
    {
      content += val;
    }
  }
}

bool
Interchunk::getNullFlush(void)
{
  return null_flush;
}

void
Interchunk::setNullFlush(bool null_flush)
{
  this->null_flush = null_flush;
}

void
Interchunk::setTrace(bool trace)
{
  this->trace = trace;
}

void
Interchunk::interchunk_wrapper_null_flush(InputFile& in, UFILE* out)
{
  null_flush = false;
  internal_null_flush = true;

  while(!in.eof()) {
    interchunk(in, out);
    u_fputc('\0', out);
    u_fflush(out);
  }
  internal_null_flush = false;
  null_flush = true;
}


void
Interchunk::interchunk(InputFile& in, UFILE* out)
{
  if(getNullFlush())
  {
    interchunk_wrapper_null_flush(in, out);
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
      u_fprintf(output, "^%S$", tmpword[0]->c_str());
	  tmpword.clear();
	  input_buffer.setPos(last);
	  input_buffer.next();
	  last = input_buffer.getPos();
	  ms.init(me->getInitial());
	}
	else if(tmpblank.size() != 0)
	{
      write(*tmpblank[0], output);
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
      
      last_lword = tmpword.size();

      if(trace)
      {
        cerr << endl << "apertium-interchunk: Rule " << val << " line " << lastrule_line;
        for (auto& it : tmpword) {
          cerr << " " << *it;
        }
        cerr << endl;
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
	ms.step(' ');
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
      write(current.getContent(), output);
	  tmpblank.clear();
	  return;
	}
	break;

      default:
	cerr << "Error: Unknown input token." << endl;
	return;
    }
  }
}

void
Interchunk::applyRule()
{
  unsigned int limit = tmpword.size();

  for(unsigned int i = 0; i != limit; i++)
  {
    if(i == 0)
    {
      word = new InterchunkWord *[limit];
      lword = limit;
    }
    else
    {
      if(int(blank_queue.size()) < last_lword - 1)
      {
        UString blank_to_add = UString(*tmpblank[i-1]);
        blank_queue.push(blank_to_add);
      }
    }

    word[i] = new InterchunkWord(*tmpword[i]);
  }

  processRule(lastrule);
  lastrule = NULL;

  if(word)
  {
    for(unsigned int i = 0; i != limit; i++)
    {
      delete word[i];
    }
    delete[] word;
  }

  word = NULL;
  tmpword.clear();
  tmpblank.clear();
  ms.init(me->getInitial());
}

void
Interchunk::applyWord(UString const &word_str)
{
  ms.step('^');
  for(unsigned int i = 0, limit = word_str.size(); i < limit; i++)
  {
    switch(word_str[i])
    {
      case '\\':
        i++;
	ms.step(towlower(word_str[i]), any_char);
	break;

      case '<':
	for(unsigned int j = i+1; j != limit; j++)
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
	break;

      case '{':  // ignore the unmodifiable part of the chunk
        ms.step('$');
        return;

      default:
	ms.step(towlower(word_str[i]), any_char);
	break;
    }
  }
  ms.step('$');
}
