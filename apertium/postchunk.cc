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
lword(0),
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
  in_lu = false;
  in_out = false;
  in_let_var = false;
  in_wblank = false;
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
  bool recompile_attrs = Compression::string_read(in) != pcre_version_endian();
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::string_read(in));
    attr_items[cad_k].read(in);
    UString fallback = Compression::string_read(in);
    if(recompile_attrs) {
      attr_items[cad_k].compile(UtfConverter::toUtf8(fallback));
    }
  }

  // variables
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::string_read(in));
    variables[cad_k] = UtfConverter::toUtf8(Compression::string_read(in));
  }

  // macros
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::string_read(in));
    macros[cad_k] = Compression::multibyte_read(in);
  }

  // lists
  for(int i = 0, limit = Compression::multibyte_read(in); i != limit; i++)
  {
    string const cad_k = UtfConverter::toUtf8(Compression::string_read(in));

    for(int j = 0, limit2 = Compression::multibyte_read(in); j != limit2; j++)
    {
      UString const cad_v = Compression::string_read(in);
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
    cerr << "Error: Could not open file '" << datafile << "'." << endl;
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
    cerr << "Error in " << UtfConverter::fromUtf8((char *) doc->URL) << ": line " << element->line << ": index > limit" << endl;
    return false;
  }
  if(index < 0) {
    cerr << "Error in " << UtfConverter::fromUtf8((char *) doc->URL) << ": line " << element->line << ": index < 0" << endl;
    return false;
  }
  if(word[index] == 0)
  {
    cerr << "Error in " << UtfConverter::fromUtf8((char *) doc->URL) << ": line " << element->line << ": Null access at word[index]" << endl;
    return false;
  }
  return true;
}

bool
Postchunk::gettingLemmaFromWord(string attr)
{
    return (attr.compare("lem") == 0 || attr.compare("lemh") == 0 || attr.compare("whole") == 0);
}

string
Postchunk::combineWblanks(string wblank_current, string wblank_to_add)
{
  if(wblank_current.empty() && wblank_to_add.empty())
  {
    return wblank_current;
  }
  else if(wblank_current.empty())
  {
    return wblank_to_add;
  }
  else if(wblank_to_add.empty())
  {
    return wblank_current;
  }
  
  string new_out_wblank;
  for(string::const_iterator it = wblank_current.begin(); it != wblank_current.end(); it++)
  {
    if(*it == '\\')
    {
      new_out_wblank += *it;
      it++;
      new_out_wblank += *it;
    }
    else if(*it == ']')
    {
      if(*(it+1) == ']')
      {
        new_out_wblank += ';';
        break;
      }
    }
    else
    {
      new_out_wblank += *it;
    }
  }
  
  for(string::const_iterator it = wblank_to_add.begin(); it != wblank_to_add.end(); it++)
  {
    if(*it == '\\')
    {
      new_out_wblank += *it;
      it++;
      new_out_wblank += *it;
    }
    else if(*it == '[')
    {
      if(*(it+1) == '[')
      {
        new_out_wblank += ' ';
        it++;
      }
    }
    else
    {
      new_out_wblank += *it;
    }
  }
  
  return new_out_wblank;
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
          if(gettingLemmaFromWord(ti.getContent()) && lword > 1)
          {
            if(in_lu)
            {
              out_wblank = combineWblanks(out_wblank, word[ti.getPos()]->getWblank());
            }
            else if(in_let_var)
            {
              var_out_wblank[var_val] = combineWblanks(var_out_wblank[var_val], word[ti.getPos()]->getWblank());
            }
          }
          
          return word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
        }
        break;

      case ti_lu_count:
        return StringUtils::itoa_string(tmpword.size());

      case ti_var:
        if(lword > 1)
        {
        out_wblank = combineWblanks(out_wblank, var_out_wblank[ti.getContent()]);
        }
        
        return variables[ti.getContent()];

      case ti_lit_tag:
      case ti_lit:
        return ti.getContent();

      case ti_b:
        if(!blank_queue.empty())
        {
          string retblank = blank_queue.front();
          if(in_out)
          {
            blank_queue.pop();
          }
          
          return retblank;
        }
        else
        {
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
    in_lu = true;
    out_wblank.clear();
    
    string myword;
    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
       if(i->type == XML_ELEMENT_NODE)
       {
         myword.append(evalString(i));
       }
    }
    
    in_lu = false;
    
    if(lword == 1)
    {
      out_wblank = word[1]->getWblank();
    }

    if(myword != "")
    {
      return out_wblank+"^"+myword+"$";
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
    out_wblank.clear();

    for(xmlNode *i = element->children; i != NULL; i = i->next)
    {
      if(i->type == XML_ELEMENT_NODE)
      {
        in_lu = true;
        
        string myword;

        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            myword.append(evalString(j));
          }
        }
        
        in_lu = false;

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
    
    if(lword == 1)
    {
      out_wblank = word[1]->getWblank();
    }

    if(value != "")
    {
      return out_wblank+"^"+value+"$";
    }
    else
    {
      return "";
    }
  }

  else
  {
    cerr << "Error: unexpected rvalue expression '" << element->name << "'" << endl;
    exit(EXIT_FAILURE);
  }

  return evalString(element);
}

void
Postchunk::processOut(xmlNode *localroot)
{
  in_out = true;
  
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "lu"))
      {
        in_lu = true;
        out_wblank.clear();
        
        string myword;
        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            myword.append(evalString(j));
          }
        }
        
        in_lu = false;
        
        if(lword == 1)
        {
          out_wblank = word[1]->getWblank();
        }

        if (!myword.empty()) {
          u_fprintf(output, "%S^%S$", out_wblank.c_str(), myword.c_str());
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "mlu"))
      {
        string myword;
        bool first_time = true;
        out_wblank.clear();
        
        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            in_lu = true;
            
            string mylocalword;
            for(xmlNode *k = j->children; k != NULL; k = k->next)
            {
              if(k->type == XML_ELEMENT_NODE)
              {
                mylocalword.append(evalString(k));
              }
            }
            
            in_lu = false;

            if(!first_time)
            {
              if(mylocalword != "")
              {
                myword += '+';
              }
            }
            else
            {
              if(mylocalword != "")
              {
                first_time = false;
              }
            }
            
            myword.append(mylocalword);
          }
        }
        
        if(lword == 1)
        {
          out_wblank = word[1]->getWblank();
        }

        u_fprintf(output, "%S^%S$", out_wblank.c_str(), myword.c_str());
      }
      else // 'b'
      {
        write(evalString(i), output);
      }
    }
  }
  
  in_out = false;
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
            write(evalString(j), output);
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
        in_let_var = true;
        var_val = ti.getContent();
        var_out_wblank[var_val].clear();
        
        variables[ti.getContent()] = evalString(rightSide);
        
        in_let_var = false;
        return;

      case ti_clip_tl:
      {
        bool match = word[ti.getPos()]->setChunkPart(attr_items[ti.getContent()], evalString(rightSide));
        if(!match && trace)
        {
          cerr << "apertium-postchunk warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
        }
      }
        return;

      default:
        return;
    }
  }
  if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    in_let_var = true;
    
    string const val = (const char *) leftSide->properties->children->content;
    
    var_val = val;
    var_out_wblank[var_val].clear();
    
    variables[val] = evalString(rightSide);
    
    in_let_var = false;
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


    bool match = word[pos]->setChunkPart(attr_items[(const char *) part],
					 evalString(rightSide));
    if(!match && trace)
    {
      cerr << "apertium-postchunk warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
    }
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
      in_let_var = true;
      var_val = name;
      variables[name].append(evalString(i));
      in_let_var = false;
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
    bool match = word[pos]->setChunkPart(attr_items[(const char *) part], result);

    if(!match && trace)
    {
      cerr << "apertium-postchunk warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
    }
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

  myword[0] = word[0];

  bool indexesOK = true;
  int idx = 1;
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
      idx++;
    }
  }

  swap(myword, word);
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
    cerr << "Warning: Not calling macro \"" << n << "\" from line " << localroot->line << " (empty word?)" << endl;
  }

  swap(myword, word);
  swap(npar, lword);

  delete[] myword;
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
      set<string> &myset = listslow[(const char *) idlist];
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

  set<string> &myset = lists[(const char *) idlist];
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
  set<string>::iterator it, limit;

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
  set<string>::iterator it, limit;

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
  UString result;
  UString const s_word = UtfConverter::fromUtf8(source_word);
  UString const t_word = UtfConverter::fromUtf8(target_word);

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
  UString const s = UtfConverter::fromUtf8(str);

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

UString
Postchunk::caseOf(UString const &str)
{
  if(str.size() > 1)
  {
    if(!iswupper(str[0]))
    {
      return "aa";
    }
    else if(!iswupper(str[str.size()-1]))
    {
      return "Aa";
    }
    else
    {
      return "AA";
    }
  }
  else if(str.size() == 1)
  {
    if(!iswupper(str[0]))
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
  
  while(!blank_queue.empty()) //flush remaining blanks that are not spaces
  {
    if(blank_queue.front().compare(" ") != 0)
    {
      write(blank_queue.front(), output);
    }
    blank_queue.pop();
  }
}

TransferToken &
Postchunk::readToken(InputFile& in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  UString content;
  while(true)
  {
    int val = fgetwc_unlocked(in);
    if(feof(in) || (internal_null_flush && val == 0))
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == '\\')
    {
      content += '\\';
      content += wchar_t(fgetwc_unlocked(in));
    }
    else if(val == '[')
    {
      content += '[';
      while(true)
      {
	int val2 = fgetwc_unlocked(in);
	if(val2 == '\\')
	{
	  content += '\\';
	  content += wchar_t(fgetwc_unlocked(in));
	}
	else if(val2 == ']')
	{
	  content += ']';
	  break;
	}
	else
	{
	  content += wchar_t(val2);
	}
      }
    }
    else if(inword && val == '{')
    {
      content += '{';
      while(true)
      {
	int val2 = fgetwc_unlocked(in);
	if(val2 == '\\')
	{
	  content += '\\';
	  content += wchar_t(fgetwc_unlocked(in));
	}
	else if(val2 == '}')
	{
	  int val3 = wchar_t(fgetwc_unlocked(in));
	  ungetwc(val3, in);

	  content += '}';
	  if(val3 == '$')
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
Postchunk::postchunk_wrapper_null_flush(InputFile& in, UFILE* out)
{
  null_flush = false;
  internal_null_flush = true;

  while(!feof(in))
  {
    postchunk(in, out);
    u_fputc('\0', out);
    int code = fflush(out);
    if(code != 0)
    {
      cerr << "Could not flush output " << errno << endl;
    }
  }

  internal_null_flush = false;
  null_flush = true;
}

void
Postchunk::postchunk(InputFile& in, UFILE* out)
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

      if(trace)
      {
        cerr << endl << "apertium-postchunk: Rule " << val << " line " << lastrule_line;
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
Postchunk::applyRule()
{
  UString const chunk = *tmpword[0];
  tmpword.clear();
  splitWordsAndBlanks(chunk, tmpword, tmpblank);

  word = new InterchunkWord *[tmpword.size()+1];
  lword = tmpword.size();
  word[0] = new InterchunkWord(UtfConverter::toUtf8(wordzero(chunk)));

  for(unsigned int i = 1, limit = tmpword.size()+1; i != limit; i++)
  {
    if(i != 1)
    {
      string blank_to_add = string(UtfConverter::toUtf8(*tmpblank[i-1]));
      blank_queue.push(blank_to_add);
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
  word = NULL;

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
Postchunk::applyWord(UString const &word_str)
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

vector<UString>
Postchunk::getVecTags(UString const &chunk)
{
  vector<UString> vectags;

  for(int i = 0, limit = chunk.size(); i != limit; i++)
  {
    if(chunk[i] == '\\')
    {
      i++;
    }
    else if(chunk[i] == '<')
    {
      UString mytag;
      do
      {
        mytag += chunk[i++];
      }
      while(chunk[i] != '>');
      vectags.push_back(mytag + '>');
    }
    else if(chunk[i] == '{')
    {
      break;
    }
  }
  return vectags;
}

int
Postchunk::beginChunk(UString const &chunk)
{
  for(int i = 0, limit = chunk.size(); i != limit; i++)
  {
    if(chunk[i] == '\\')
    {
      i++;
    }
    else if(chunk[i] == '{')
    {
      return i + 1;
    }
  }
  return chunk.size();
}

int
Postchunk::endChunk(UString const &chunk)
{
  return chunk.size()-2;
}

UString
Postchunk::wordzero(UString const &chunk)
{
  for(unsigned int i = 0, limit = chunk.size(); i != limit ;i++)
  {
    if(chunk[i] == '\\')
    {
      i++;
    }
    else if(chunk[i] == '{')
    {
      return chunk.substr(0, i);
    }
  }

  return "";
}

UString
Postchunk::pseudolemma(UString const &chunk)
{
  for(unsigned int i = 0, limit = chunk.size(); i != limit ;i++)
  {
    if(chunk[i] == '\\')
    {
      i++;
    }
    else if(chunk[i] == '<' || chunk[i] == '{')
    {
      return chunk.substr(0, i);
    }
  }

  return "";
}

void
Postchunk::unchunk(UString const &chunk, UFILE* output)
{
  vector<UString> vectags = getVecTags(chunk);
  UString case_info = caseOf(pseudolemma(chunk));
  bool uppercase_all = false;
  bool uppercase_first = false;

  if(case_info == "AA")
  {
    uppercase_all = true;
  }
  else if(case_info == "Aa")
  {
    uppercase_first = true;
  }

  for(int i = beginChunk(chunk), limit = endChunk(chunk); i < limit; i++)
  {
    if(chunk[i] == '\\') {
      u_fputc('\\', output);
      u_fputc(chunk[++i], output);
    } else if(chunk[i] == '^') {
      u_fputc('^', output);
      while(chunk[++i] != '$')
      {
        if(chunk[i] == '\\')
        {
          u_fputc('\\', output);
          u_fputc(chunk[++i], output);
        }
        else if(chunk[i] == '<')
        {
          if(iswdigit(chunk[i+1]))
          {
            // replace tag
            unsigned long value = wcstoul(chunk.c_str()+i+1,
					  NULL, 0) - 1;
            //atoi(chunk.c_str()+i+1)-1;
            if(vectags.size() > value)
            {
              write(vectags[value], output);
            }
            while(chunk[++i] != '>');
          }
          else
          {
            u_fputc('<', output);
            while(chunk[++i] != '>') u_fputc(chunk[i], output);
            u_fputc('>', output);
          }
        }
        else
        {
          if(uppercase_all)
          {
            u_fputc(towupper(chunk[i]), output);
          }
          else if(uppercase_first)
          {
	    if(iswalnum(chunk[i]))
	    {
	      u_fputc(towupper(chunk[i]), output);
	      uppercase_first = false;
	    }
            else
	    {
	      u_fputc(chunk[i], output);
	    }
          }
          else
          {
            u_fputc(chunk[i], output);
          }
        }
      }
      u_fputc('$', output);
    }
    else if(chunk[i] == '[')
    {
      u_fputc('[', output);
      while(chunk[++i] != ']')
      {
        if(chunk[i] == '\\')
        {
          u_fputc('\\', output);
          u_fputc(chunk[++i], output);
        }
        else
        {
          u_fputc(chunk[i], output);
        }
      }
      u_fputc(']', output);
    }
    else
    {
      u_fputc(chunk[i], output);
    }
  }
}


void
Postchunk::splitWordsAndBlanks(UString const &chunk, vector<UString *> &words,
                               vector<UString *> &blanks)
{
  vector<UString> vectags = getVecTags(chunk);
  UString case_info = caseOf(pseudolemma(chunk));
  bool uppercase_all = false;
  bool uppercase_first = false;
  bool lastblank = true;

  if(case_info == "AA")
  {
    uppercase_all = true;
  }
  else if(case_info == "Aa")
  {
    uppercase_first = true;
  }

  for(int i = beginChunk(chunk), limit = endChunk(chunk); i < limit; i++)
  {
    if(chunk[i] == '^')
    {
      if(!lastblank)
      {
        blanks.push_back(new UString(""));
      }
      lastblank = false;
      UString *myword = new UString();
      UString &ref = *myword;

      while(chunk[++i] != '$')
      {
        if(chunk[i] == '\\')
        {
          ref += '\\';
          ref += chunk[++i];
        }
        else if(chunk[i] == '<')
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
            while(chunk[++i] != '>');
          }
          else
          {
            ref += '<';
            while(chunk[++i] != '>') ref += chunk[i];
            ref += '>';
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
    else if(chunk[i] == '[')
    {
      if(chunk[i+1] == '[') //wordbound blank
      {
        if(!lastblank)
        {
          blanks.push_back(new UString(""));
        }
        lastblank = false;
        UString *myword = new UString();
        UString &ref = *myword;

        while(true)
        {
          if(chunk[i] == '\\')
          {
            ref += '\\';
            ref += chunk[++i];
          }
          else if(chunk[i] == ']' && chunk[i-1] == ']')
          {
            ref += chunk[i];
            i++; //i->"^"
            break;
          }
          else
          {
            ref += chunk[i];
          }
          
          i++;
        }
        
        while(chunk[++i] != '$')
        {
          if(chunk[i] == '\\')
          {
            ref += '\\';
            ref += chunk[++i];
          }
          else if(chunk[i] == '<')
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
              while(chunk[++i] != '>');
            }
            else
            {
              ref += '<';
              while(chunk[++i] != '>') ref += chunk[i];
              ref += '>';
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
      else
      {
        if (!(lastblank && blanks.back()))
        {
          blanks.push_back(new UString());
        }
        UString &ref = *(blanks.back());
        ref += '[';
        while(chunk[++i] != ']')
        {
          if(chunk[i] == '\\')
          {
            ref += '\\';
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
    }
    else
    {
      if (!lastblank)
      {
        UString *myblank = new UString("");
        blanks.push_back(myblank);
      }
      UString &ref = *(blanks.back());
      if(chunk[i] == '\\')
      {
        ref += '\\';
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

