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
Interchunk::copy(Interchunk const &o)
{
}

void
Interchunk::destroy()
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

Interchunk::Interchunk()
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

Interchunk::~Interchunk()
{
  destroy();
}

Interchunk::Interchunk(Interchunk const &o)
{
  copy(o);
}

Interchunk &
Interchunk::operator =(Interchunk const &o)
{
  if(this != &o)
  {
    destroy();
    copy(o);
  }
  return *this;
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
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::wstring_read(in));
    attr_items[cad_k].read(in);
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
Interchunk::read(string const &transferfile, string const &datafile)
{
  readInterchunk(transferfile);
  
  // datafile
  FILE *in = fopen(datafile.c_str(), "rb");
  if(!in)
  {
    cerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

}

void
Interchunk::readInterchunk(string const &in)
{
  doc = xmlReadFile(in.c_str(), NULL, 0);
  
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
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      for(xmlNode *j = i->children; ; j = j->next)
      {
        if(j->type == XML_ELEMENT_NODE && !xmlStrcmp(j->name, (const xmlChar *) "action"))
        {
          rule_map.push_back(j);
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
    wcerr << L"Error in " << UtfConverter::fromUtf8((char *) doc->URL) <<L": line " << element->line << endl;
    return false;
  }
  return true;
}


string 
Interchunk::evalString(xmlNode *element)
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
          if(ti.getContent() == "content") // jacob's new 'part'
          { 
            string wf = word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
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
        if(checkIndex(element, ti.getPos(), lblank))
        {
          if(ti.getPos() >= 0)
          {
            return !blank?"":*(blank[ti.getPos()]);
          }
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
	pos = atoi((const char *)i->children->content) - 1;
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
    evalStringCache[element] = TransferInstr(ti_lit, ((const char *) element->properties->children->content), 0);
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

    evalStringCache[element] = TransferInstr(ti_get_case_from, "lem", pos, param);
  }
  else if(!xmlStrcmp(element->name, (const xmlChar *) "var"))
  {
    evalStringCache[element] = TransferInstr(ti_var, (const char *) element->properties->children->content, 0);
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
	pos = atoi((const char *) i->children->content) - 1;
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
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "chunk"))
      {
        fputws_unlocked(UtfConverter::fromUtf8(processChunk(i)).c_str(), output);
      }
      else // 'b'
      {
        fputws_unlocked(UtfConverter::fromUtf8(evalString(i)).c_str(), output);
      }
    }
  }
}

string
Interchunk::processChunk(xmlNode *localroot)
{
  string result;
  result.append("^");
  
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      result.append(evalString(i));
    }      
  }
  
  result.append("$");
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
	pos = atoi((const char *) i->children->content) - 1;
      }
    }
    

    word[pos]->setChunkPart(attr_items[(const char *) part], 
			    evalString(rightSide));
    evalStringCache[leftSide] = TransferInstr(ti_clip_tl, 
					      (const char *) part, 
					      pos, NULL);
  }
}

void
Interchunk::processAppend(xmlNode *localroot)
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
	pos = atoi((const char *) i->children->content) - 1;
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
Interchunk::processCallMacro(xmlNode *localroot)
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

  InterchunkWord **myword = NULL;
  if(npar > 0)
  {
    myword = new InterchunkWord *[npar];  
  }
  string **myblank = NULL;
  if(npar > 1)
  {
    myblank = new string *[npar - 1];
  }

  int idx = 0;
  int lastpos = 0;
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      int pos = atoi((const char *) i->properties->children->content)-1;
      myword[idx] = word[pos];
      if(idx-1 >= 0)
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
  
  for(xmlNode *i = macro->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      processInstruction(i);
    }
  }

  swap(myword, word);
  swap(myblank, blank);
  swap(npar, lword);

  if(myword)
  {
    delete[] myword;
  }
  if(myblank)
  {
    delete[] myblank;
  }
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
Interchunk::beginsWith(string const &s1, string const &s2) const
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
Interchunk::endsWith(string const &s1, string const &s2) const
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
Interchunk::copycase(string const &source_word, string const &target_word)
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
Interchunk::caseOf(string const &str)
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

string
Interchunk::tolower(string const &str) const
{
  return UtfConverter::toUtf8(StringUtils::tolower(UtfConverter::fromUtf8(str)));
}

string
Interchunk::tags(string const &str) const
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
}

TransferToken &
Interchunk::readToken(FILE *in)
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
	  wint_t val3 = wchar_t(fgetwc_unlocked(in));
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
Interchunk::interchunk_wrapper_null_flush(FILE *in, FILE *out)
{
  null_flush = false;
  internal_null_flush = true;
  
  while(!feof(in))
  {
    interchunk(in, out);
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
Interchunk::interchunk(FILE *in, FILE *out)
{
  if(getNullFlush())
  {
    interchunk_wrapper_null_flush(in, out);
  }
  
  int last = 0;

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
          fputwc_unlocked(L'^', output);
          fputws_unlocked(tmpword[0]->c_str(), output);
          fputwc_unlocked(L'$', output);
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
      lastrule = rule_map[val-1];      
      last = input_buffer.getPos();

      if(trace)
      {
        wcerr << endl << L"apertium-interchunk: Rule " << val << L" ";
        for (unsigned int ind = 0; ind < tmpword.size(); ind++)
        {
          if (ind != 0)
          {
            wcerr << L" ";
          }
          wcerr << *tmpword[ind];
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
      if(limit != 1)
      {
        blank = new string *[limit - 1];
        lblank = limit - 1;
      }
      else
      {
        blank = NULL;
        lblank = 0;
      }
    }
    else
    {
      blank[i-1] = new string(UtfConverter::toUtf8(*tmpblank[i-1]));
    }
    
    word[i] = new InterchunkWord(UtfConverter::toUtf8(*tmpword[i]));
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
  if(blank)
  {
    for(unsigned int i = 0; i != limit - 1; i++)
    {
      delete blank[i];
    }
    delete[] blank;
  }
  word = NULL;
  blank = NULL;
  tmpword.clear();
  tmpblank.clear();
  ms.init(me->getInitial());
}

void
Interchunk::applyWord(wstring const &word_str)
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
	for(unsigned int j = i+1; j != limit; j++)
	{
	  if(word_str[j] == L'>')
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
