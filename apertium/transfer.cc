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
#include <apertium/transfer.h>
#include <apertium/trx_reader.h>
#include <apertium/utf_converter.h>
#include <apertium/string_utils.h>
#include <lttoolbox/compression.h>
#include <lttoolbox/xml_parse_util.h>
#include <pcre.h>

#include <cctype>
#include <iostream>
#include <stack>
#include <cerrno>

#ifdef _WIN32
#include <utf8_fwrap.h>
#endif

using namespace Apertium;
using namespace std;

void
Transfer::destroy()
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

Transfer::Transfer() :
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
  defaultAttrs = lu;
  useBilingual = true;
  preBilingual = false;
  isExtended = false;
  null_flush = false;
  internal_null_flush = false;
  trace = false;
  trace_att = false;
  emptyblank = "";
}

Transfer::~Transfer()
{
  destroy();
}

void
Transfer::readData(FILE *in)
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
Transfer::readBil(string const &fstfile)
{
  FILE *in = fopen(fstfile.c_str(), "rb");
  if(!in)
  {
    wcerr << "Error: Could not open file '" << fstfile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  fstp.load(in);
  fstp.initBiltrans();
  fclose(in);
}

void
Transfer::setExtendedDictionary(string const &fstfile)
{
  FILE *in = fopen(fstfile.c_str(), "rb");
  if(!in)
  {
    wcerr << "Error: Could not open extended dictionary file '" << fstfile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  extended.load(in);
  extended.initBiltrans();
  fclose(in);
  isExtended = true;
}

void
Transfer::read(string const &transferfile, string const &datafile,
	       string const &fstfile)
{
  readTransfer(transferfile);

  // datafile
  FILE *in = fopen(datafile.c_str(), "rb");
  if(!in)
  {
    wcerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

  if(fstfile != "")
  {
    readBil(fstfile);
  }
}

void
Transfer::readTransfer(string const &in)
{
  doc = xmlReadFile(in.c_str(), NULL, 0);

  if(doc == NULL)
  {
    wcerr << "Error: Could not parse file '" << in << "'." << endl;
    exit(EXIT_FAILURE);
  }

  root_element = xmlDocGetRootElement(doc);

  // search for root element attributes
  for(xmlAttr *i = root_element->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "default"))
    {
      if(!xmlStrcmp(i->children->content, (const xmlChar *) "chunk"))
      {
        defaultAttrs = chunk;
      }
      else
      {
        defaultAttrs = lu; // default value for 'default'
      }
    }
  }

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
Transfer::collectRules(xmlNode *localroot)
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
Transfer::collectMacros(xmlNode *localroot)
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
Transfer::checkIndex(xmlNode *element, int index, int limit)
{
  if(index >= limit)
  {
    wcerr << L"Error in " << UtfConverter::fromUtf8((char *) doc->URL) << L": line " << element->line << L": index >= limit" << endl;
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
Transfer::evalString(xmlNode *element)
{
  map<xmlNode *, TransferInstr>::iterator it;
  it = evalStringCache.find(element);
  if(it != evalStringCache.end())
  {
    TransferInstr &ti = it->second;
    switch(ti.getType())
    {
      case ti_clip_sl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return word[ti.getPos()]->source(attr_items[ti.getContent()], ti.getCondition());
        }
        break;

      case ti_clip_tl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return word[ti.getPos()]->target(attr_items[ti.getContent()], ti.getCondition());
        }
        break;

      case ti_clip_ref:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return word[ti.getPos()]->reference(attr_items[ti.getContent()], ti.getCondition());
        }
        break;

      case ti_linkto_sl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          if(word[ti.getPos()]->source(attr_items[ti.getContent()], ti.getCondition()) != "")
          {
            return "<" + string((char *) ti.getPointer()) + ">";
          }
          else
          {
            return "";
          }
        }
        break;

      case ti_linkto_tl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          if(word[ti.getPos()]->target(attr_items[ti.getContent()], ti.getCondition()) != "")
          {
            return "<" + string((char *) ti.getPointer()) + ">";
          }
          else
          {
            return "";
          }
        }
        break;

      case ti_linkto_ref:
        if(checkIndex(element, ti.getPos(), lword))
        {
          if(word[ti.getPos()]->reference(attr_items[ti.getContent()], ti.getCondition()) != "")
          {
            return "<" + string((char *) ti.getPointer()) + ">";
          }
          else
          {
            return "";
          }
        }
        break;

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
          return copycase(word[ti.getPos()]->source(attr_items[ti.getContent()]),
                  evalString((xmlNode *) ti.getPointer()));
        }
        break;

      case ti_case_of_sl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return caseOf(word[ti.getPos()]->source(attr_items[ti.getContent()]));
        }
        break;

      case ti_case_of_tl:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return caseOf(word[ti.getPos()]->target(attr_items[ti.getContent()]));
        }
        break;

      case ti_case_of_ref:
        if(checkIndex(element, ti.getPos(), lword))
        {
          return caseOf(word[ti.getPos()]->reference(attr_items[ti.getContent()]));
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
    xmlChar *part = NULL, *side = NULL, *as = NULL;
    bool queue = true;

    for(xmlAttr *i = element->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "side"))
      {
	side = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *)i->children->content) - 1;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "queue"))
      {
        if(!xmlStrcmp(i->children->content, (const xmlChar *) "no"))
        {
          queue = false;
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "link-to"))
      {
        as = i->children->content;
      }
    }

    if(as != NULL)
    {
      if(!xmlStrcmp(side, (const xmlChar *) "sl"))
      {
        evalStringCache[element] = TransferInstr(ti_linkto_sl, (const char *) part, pos, (void *) as, queue);
      }
      else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
      {
        evalStringCache[element] = TransferInstr(ti_linkto_ref, (const char *) part, pos, (void *) as, queue);
      }
      else
      {
        evalStringCache[element] = TransferInstr(ti_linkto_tl, (const char *) part, pos, (void *) as, queue);
      }
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "sl"))
    {
      evalStringCache[element] = TransferInstr(ti_clip_sl, (const char *) part, pos, NULL, queue);
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
    {
      evalStringCache[element] = TransferInstr(ti_clip_ref, (const char *) part, pos, NULL, queue);
    }
    else
    {
      evalStringCache[element] = TransferInstr(ti_clip_tl, (const char *) part, pos, NULL, queue);
    }
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
    xmlChar *part = NULL, *side = NULL;

    for(xmlAttr *i = element->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "side"))
      {
	side = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content) - 1;
      }
    }

    if(!xmlStrcmp(side, (const xmlChar *) "sl"))
    {
      evalStringCache[element] = TransferInstr(ti_case_of_sl, (const char *) part, pos);
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
    {
      evalStringCache[element] = TransferInstr(ti_case_of_ref, (const char *) part, pos);
    }
    else
    {
      evalStringCache[element] = TransferInstr(ti_case_of_tl, (const char *) part, pos);
    }
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
  else if(!xmlStrcmp(element->name, (const xmlChar *) "chunk"))
  {
    return processChunk(element);
  }
  else
  {
    wcerr << "Error: unexpected rvalue expression '" << element->name << "'" << endl;
    exit(EXIT_FAILURE);
  }

  return evalString(element);
}

void
Transfer::processOut(xmlNode *localroot)
{
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(defaultAttrs == lu)
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
	  fputwc_unlocked('^', output);
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
	        if(myword != "" && myword[0] != '#')  //'+#' problem
	        {
	          fputwc_unlocked(L'+', output);
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
          fputws_unlocked(UtfConverter::fromUtf8(evalString(i)).c_str(),
			  output);
        }
      }
      else
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
}

string
Transfer::processChunk(xmlNode *localroot)
{
  string name, namefrom;
  string caseofchunk = "aa";
  string result;

  for(xmlAttr *i = localroot->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "name"))
    {
      name = (const char *) i->children->content;
    }
    else if(!xmlStrcmp(i->name, (const xmlChar *) "namefrom"))
    {
      namefrom = (const char *) i->children->content;
    }
    else if(!xmlStrcmp(i->name, (const xmlChar *) "case"))
    {
      caseofchunk = (const char *) i->children->content;
    }
  }

  result.append("^");
  if(caseofchunk != "")
  {
    if(name != "")
    {
      result.append(copycase(variables[caseofchunk], name));
    }
    else if(namefrom != "")
    {
      result.append(copycase(variables[caseofchunk], variables[namefrom]));
    }
    else
    {
      wcerr << "Error: you must specify either 'name' or 'namefrom' for the 'chunk' element" << endl;
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    if(name != "")
    {
      result.append(name);
    }
    else if(namefrom != "")
    {
      result.append(variables[namefrom]);
    }
    else
    {
      wcerr << "Error: you must specify either 'name' or 'namefrom' for the 'chunk' element" << endl;
      exit(EXIT_FAILURE);
    }
  }

  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "tags"))
      {
        result.append(processTags(i));
        result.append("{");
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "lu"))
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
          result.append("^");
          result.append(myword);
          result.append("$");
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "mlu"))
      {
        bool first_time = true;
        string myword;
        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          string mylocalword;
          if(j->type == XML_ELEMENT_NODE)
          {
            for(xmlNode *k = j->children; k != NULL; k = k->next)
            {
              if(k->type == XML_ELEMENT_NODE)
              {
                mylocalword.append(evalString(k));
              }
            }

            if(!first_time)
            {
              if(mylocalword != "" && mylocalword[0] != '#')  // '+#' problem
              {
                myword += '+';
              }
            }
            else
            {
              first_time = false;
            }
          }
          myword.append(mylocalword);
        }
        if(myword != "")
        {
          result.append("^");
          result.append(myword);
          result.append("$");
        }
      }
      else // 'b'
      {
        result.append(evalString(i));
      }
    }
  }
  result.append("}$");
  return result;
}

string
Transfer::processTags(xmlNode *localroot)
{
  string result;
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
            result.append(evalString(j));
          }
        }
      }
    }
  }
  return result;
}

int
Transfer::processInstruction(xmlNode *localroot)
{
  int words_to_consume = -1;
  if(!xmlStrcmp(localroot->name, (const xmlChar *) "choose"))
  {
    words_to_consume = processChoose(localroot);
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
  else if(!xmlStrcmp(localroot->name, (const xmlChar *) "reject-current-rule"))
  {
    words_to_consume = processRejectCurrentRule(localroot);
  }
  return words_to_consume;
}

int
Transfer::processRejectCurrentRule(xmlNode *localroot)
{
  bool shifting = true;
  string value;
  for(xmlAttr *i = localroot->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "shifting"))
    {
      value = (char *) i->children->content;
      break;
    }
  }

  if(value == "no")
  {
    shifting = false;
  }

  return shifting ? 1 : 0;
}

void
Transfer::processLet(xmlNode *localroot)
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

      case ti_clip_sl:
        if (checkIndex(leftSide, ti.getPos(), lword)) {
          bool match = word[ti.getPos()]->setSource(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
          if (!match && trace)
          {
            wcerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
          }
        }
        return;

      case ti_clip_tl:
        if (checkIndex(leftSide, ti.getPos(), lword)) {
          bool match = word[ti.getPos()]->setTarget(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
          if (!match && trace)
          {
            wcerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
          }
        }
        return;

      case ti_clip_ref:
        if (checkIndex(leftSide, ti.getPos(), lword)) {
          bool match = word[ti.getPos()]->setReference(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
          if (!match && trace)
          {
            wcerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
          }
        }
        return;

      default:
        return;
    }
  }
  if(leftSide->name != NULL && !xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    string const val = (const char *) leftSide->properties->children->content;
    variables[val] = evalString(rightSide);
    evalStringCache[leftSide] = TransferInstr(ti_var, val, 0);
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    xmlChar *part = NULL, *side = NULL, *as = NULL;
    bool queue = true;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "side"))
      {
	side = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content) - 1;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "queue"))
      {
        if(!xmlStrcmp(i->children->content, (const xmlChar *) "no"))
        {
          queue = false;
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "link-to"))
      {
        as = i->children->content; // TODO: set but never read
      }
    }

    if (pos >= lword) {
      wcerr << L"Error: Transfer::processLet() bad access on pos >= lword" << endl;
      return;
    }
    if (word[pos] == 0) {
      wcerr << L"Error: Transfer::processLet() null access on word[pos]" << endl;
      return;
    }

    if(!xmlStrcmp(side, (const xmlChar *) "tl"))
    {
      bool match = word[pos]->setTarget(attr_items[(const char *) part], evalString(rightSide), queue);
      if(!match && trace)
      {
        wcerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
      }
      evalStringCache[leftSide] = TransferInstr(ti_clip_tl, (const char *) part, pos, NULL, queue);
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
    {
      bool match = word[pos]->setReference(attr_items[(const char *) part], evalString(rightSide), queue);
      if(!match && trace)
      {
        wcerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
      }
      evalStringCache[leftSide] = TransferInstr(ti_clip_ref, (const char *) part, pos, NULL, queue);
    }
    else
    {
      bool match = word[pos]->setSource(attr_items[(const char *) part], evalString(rightSide), queue);
      if(!match && trace)
      {
        wcerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
      }
      evalStringCache[leftSide] = TransferInstr(ti_clip_sl, (const char *) part, pos, NULL, queue);
    }
  }
}

void
Transfer::processAppend(xmlNode *localroot)
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
Transfer::processModifyCase(xmlNode *localroot)
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
    xmlChar *part = NULL, *side = NULL, *as = NULL;
    bool queue = true;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "side"))
      {
	side = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
	part = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content) - 1;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "queue"))
      {
        if(!xmlStrcmp(i->children->content, (xmlChar const *) "no"))
        {
          queue = false;
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "link-to"))
      {
        as = i->children->content;
        (void)as; // ToDo, remove "as" and the whole else?
      }
    }
    if(!xmlStrcmp(side, (const xmlChar *) "sl"))
    {
      string const result = copycase(evalString(rightSide),
				      word[pos]->source(attr_items[(const char *) part], queue));
      bool match = word[pos]->setSource(attr_items[(const char *) part], result);
      if(!match && trace)
      {
        wcerr << "apertium-transfer warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
      }
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
    {
      string const result = copycase(evalString(rightSide),
              word[pos]->reference(attr_items[(const char *) part], queue));
      bool match = word[pos]->setReference(attr_items[(const char *) part], result);
      if(!match && trace)
      {
        wcerr << "apertium-transfer warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
      }
    }
    else
    {
      string const result = copycase(evalString(rightSide),
				     word[pos]->target(attr_items[(const char *) part], queue));
      bool match = word[pos]->setTarget(attr_items[(const char *) part], result);
      if(!match && trace)
      {
        wcerr << "apertium-transfer warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
      }
    }
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    string const val = (const char *) leftSide->properties->children->content;
    variables[val] = copycase(evalString(rightSide), variables[val]);
  }
}

void
Transfer::processCallMacro(xmlNode *localroot)
{
  string const n = (const char *) localroot->properties->children->content;
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

  TransferWord **myword = NULL;
  if(npar > 0)
  {
    myword = new TransferWord *[npar];
    std::fill(myword, myword+npar, (TransferWord *)(0));
  }
  string **myblank = NULL;
  if(npar > 0)
  {
    myblank = new string *[npar];
    myblank[npar-1] = &emptyblank;
  }

  int idx = 0;
  int lastpos = 0;
  for(xmlNode *i = localroot->children; npar && i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      if (idx >= npar) {
      	  wcerr << L"Error: processCallMacro() number of arguments >= npar at line " << i->line << endl;
      	  return;
      }
      int pos = atoi((const char *) i->properties->children->content)-1;
      myword[idx] = word[pos];
      if(idx-1 >= 0)
      {
        if(lastpos + 1 > lblank) { // if a 1-pattern rule calls macro with same
          noblank = "";            // param twice the blank array will be empty
          myblank[idx-1] = &noblank;
        }
        else {
          myblank[idx-1] = blank[lastpos];
        }
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

  delete[] myword;
  delete[] myblank;
}

int
Transfer::processChoose(xmlNode *localroot)
{
  int words_to_consume = -1;
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
              words_to_consume = processInstruction(j);
              if(words_to_consume != -1)
              {
                return words_to_consume;
              }
	    }
	  }
	}
        if(picked_option)
        {
          return words_to_consume;
        }
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "otherwise"))
      {
	for(xmlNode *j = i->children; j != NULL; j = j->next)
	{
	  if(j->type == XML_ELEMENT_NODE)
	  {
            words_to_consume = processInstruction(j);
            if(words_to_consume != -1)
            {
              return words_to_consume;
            }
          }
        }
      }
    }
  }
  return words_to_consume;
}

bool
Transfer::processLogical(xmlNode *localroot)
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
Transfer::processIn(xmlNode *localroot)
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
Transfer::processTest(xmlNode *localroot)
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
Transfer::processAnd(xmlNode *localroot)
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
Transfer::processOr(xmlNode *localroot)
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
Transfer::processNot(xmlNode *localroot)
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
Transfer::processEqual(xmlNode *localroot)
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
Transfer::beginsWith(string const &s1, string const &s2) const
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
Transfer::endsWith(string const &s1, string const &s2) const
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
Transfer::processBeginsWith(xmlNode *localroot)
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
Transfer::processEndsWith(xmlNode *localroot)
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
Transfer::processBeginsWithList(xmlNode *localroot)
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
Transfer::processEndsWithList(xmlNode *localroot)
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
Transfer::processContainsSubstring(xmlNode *localroot)
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
Transfer::copycase(string const &source_word, string const &target_word)
{
  wstring result;
  wstring const s_word = UtfConverter::fromUtf8(source_word);
  wstring const t_word = UtfConverter::fromUtf8(target_word);

  bool firstupper = iswupper(s_word[0]);
  bool uppercase = firstupper && iswupper(s_word[s_word.size()-1]);
  bool sizeone = s_word.size() == 1;

  if(!uppercase || (sizeone && uppercase))
  {
    result = t_word;
    result[0] = towlower(result[0]);
    //result = StringUtils::tolower(t_word);
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
Transfer::caseOf(string const &str)
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
Transfer::tolower(string const &str) const
{
  return UtfConverter::toUtf8(StringUtils::tolower(UtfConverter::fromUtf8(str)));
}

string
Transfer::tags(string const &str) const
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

int
Transfer::processRule(xmlNode *localroot)
{
  int instruction_return, words_to_consume = -1;
  // localroot is suposed to be an 'action' tag
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    {
      instruction_return = processInstruction(i);
      // When an instruction which modifies the number of words to be consumed
      // from the input is found, execution of the rule is stopped
      if(instruction_return != -1)
      {
        words_to_consume = instruction_return;
        break;
      }
    }
  }
  return words_to_consume;
}

TransferToken &
Transfer::readToken(FILE *in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  wstring content;
  while(true)
  {
    int val = fgetwc_unlocked(in);
    if(feof(in) || (val == 0 && internal_null_flush))
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == '\\')
    {
      content += L'\\';
      content += (wchar_t) fgetwc_unlocked(in);
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
    else if(val == L'$')
    {
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == L'^')
    {
      return input_buffer.add(TransferToken(content, tt_blank));
    }
    else if(val == L'\0' && null_flush)
    {
      fflush(output);
    }
    else
    {
      content += wchar_t(val);
    }
  }
}

bool
Transfer::getNullFlush(void)
{
  return null_flush;
}

void
Transfer::setNullFlush(bool null_flush)
{
  this->null_flush = null_flush;
}

void
Transfer::setTrace(bool trace)
{
  this->trace = trace;
}

void
Transfer::setTraceATT(bool trace)
{
  this->trace_att = trace;
}

void
Transfer::transfer_wrapper_null_flush(FILE *in, FILE *out)
{
  null_flush = false;
  internal_null_flush = true;

  while(!feof(in))
  {
    transfer(in, out);
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
Transfer::transfer(FILE *in, FILE *out)
{
  if(getNullFlush())
  {
    transfer_wrapper_null_flush(in, out);
  }

  unsigned int last = input_buffer.getPos();
  unsigned int prev_last = last;
  int lastrule_id = -1;
  set<int> banned_rules;

  output = out;
  ms.init(me->getInitial());

  while(true)
  {
    if(trace_att)
    {
      wcerr << "Loop start " << endl;
      wcerr << "ms.size: " << ms.size() << endl;

      wcerr << "tmpword.size(): " << tmpword.size() << endl;
      for (unsigned int ind = 0; ind < tmpword.size(); ind++)
      {
        if(ind != 0)
        {
          wcerr << L" ";
        }
        wcerr << *tmpword[ind];
      }
      wcerr << endl;

      wcerr << "tmpblank.size(): " << tmpblank.size() << endl;
      for (unsigned int ind = 0; ind < tmpblank.size(); ind++)
      {
        wcerr << L"'";
        wcerr << *tmpblank[ind];
        wcerr << L"' ";
      }
      wcerr << endl;

      wcerr << "last: " << last << endl;
      wcerr << "prev_last: " << prev_last << endl << endl;
    }

    if(ms.size() == 0)
    {
      if(lastrule != NULL)
      {
        int num_words_to_consume = applyRule();

        if(trace_att)
        {
          wcerr << "num_words_to_consume: " << num_words_to_consume << endl;
        }

        //Consume all the words from the input which matched the rule.
        //This piece of code is executed unless the rule contains a "reject-current-rule" instruction
        if(num_words_to_consume < 0)
        {
          banned_rules.clear();
          input_buffer.setPos(last);
        }
        else if(num_words_to_consume > 0)
        {
          banned_rules.clear();
          if(prev_last >= input_buffer.getSize())
          {
            input_buffer.setPos(0);
          }
          else
          {
            input_buffer.setPos(prev_last+1);
          }
          int num_consumed_words = 0;
          while(num_consumed_words < num_words_to_consume)
          {
            TransferToken& local_tt = input_buffer.next();
            if (local_tt.getType() == tt_word)
            {
              num_consumed_words++;
            }
          }
        }
        else
        {
          //Add rule to banned rules
          banned_rules.insert(lastrule_id);
          input_buffer.setPos(prev_last);
          input_buffer.next();
          last = input_buffer.getPos();
        }
        lastrule_id = -1;
      }
      else
      {
        if(tmpword.size() != 0)
        {
          if(trace_att)
          {
            wcerr << "printing tmpword[0]" <<endl;
          }

          pair<wstring, int> tr;
          if(useBilingual && preBilingual == false)
          {
	    if(isExtended && (*tmpword[0])[0] == L'*')
	    {
	      tr = extended.biltransWithQueue((*tmpword[0]).substr(1), false);
              if(tr.first[0] == L'@')
              {
                tr.first[0] = L'*';
              }
              else
              {
                tr.first = L"%" + tr.first;
              }
            }
            else
            {
	      tr = fstp.biltransWithQueue(*tmpword[0], false);
            }
          }
          else if(preBilingual)
          {
            wstring sl;
            wstring tl;
            wstring ref;

            int seenSlash = 0;
            for(wstring::const_iterator it = tmpword[0]->begin(); it != tmpword[0]->end(); it++)
            {
              if(*it == L'\\')
              {
                if(seenSlash == 0)
                {
                  sl.push_back(*it);
                  it++;
                  sl.push_back(*it);
                }
                else if(seenSlash == 1)
                {
                  tl.push_back(*it);
                  it++;
                  tl.push_back(*it);
                }
                else
                {
                  ref.push_back(*it);
                  it++;
                  ref.push_back(*it);
                }
                continue;
              }
              else if(*it == L'/')
              {
                seenSlash++;

                ref.clear(); //the word after the last slash is the ref
                continue;
              }
              if(seenSlash == 0)
              {
                sl.push_back(*it);
              }
              else if(seenSlash == 1)
              {
                tl.push_back(*it);
              }
              else
              {
                ref.push_back(*it);
              }
            }
            //tmpword[0]->assign(sl);
            tr = pair<wstring, int>(tl, false);
            //wcerr << L"pb: " << *tmpword[0] << L" :: " << sl << L" >> " << tl << endl ;
          }
          else
          {
            tr = pair<wstring, int>(*tmpword[0], 0);
          }

	  if(tr.first.size() != 0)
	  {
	    if(defaultAttrs == lu)
	    {
	      fputwc_unlocked(L'^', output);
	      fputws_unlocked(tr.first.c_str(), output);
	      fputwc_unlocked(L'$', output);
            }
            else
            {
              if(tr.first[0] == '*')
              {
                fputws_unlocked(L"^unknown<unknown>{^", output);
              }
              else
              {
	        fputws_unlocked(L"^default<default>{^", output);
              }
	      fputws_unlocked(tr.first.c_str(), output);
	      fputws_unlocked(L"$}$", output);
            }
	  }
	  banned_rules.clear();
	  tmpword.clear();
	  input_buffer.setPos(last);
	  input_buffer.next();
	  prev_last = last;
	  last = input_buffer.getPos();
	  ms.init(me->getInitial());
	}
	else if(tmpblank.size() != 0)
	{
          if(trace_att)
          {
            wcerr << "printing tmpblank[0]" <<endl;
          }
          fputws_unlocked(tmpblank[0]->c_str(), output);
          tmpblank.clear();
          prev_last = last;
          last = input_buffer.getPos();
          ms.init(me->getInitial());
	}
      }
    }
    int val = ms.classifyFinals(me->getFinals(), banned_rules);
    if(val != -1)
    {
      size_t lastrule_line = rule_lines[val-1];
      lastrule = rule_map[val-1];
      lastrule_id = val;
      last = input_buffer.getPos();

      if(trace)
      {
        wcerr << endl << L"apertium-transfer: Rule " << val << L" line " << lastrule_line << L" ";
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

int
Transfer::applyRule()
{
  int words_to_consume;
  unsigned int limit = tmpword.size();
  //wcerr << L"applyRule: " << tmpword.size() << endl;

  for(unsigned int i = 0; i != limit; i++)
  {
    if(i == 0)
    {
      word = new TransferWord *[limit];
      std::fill(word, word+limit, (TransferWord *)(0));
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

    pair<wstring, int> tr;
    if(useBilingual && preBilingual == false)
    {
      tr = fstp.biltransWithQueue(*tmpword[i], false);
      wstring refx;
      word[i] = new TransferWord(UtfConverter::toUtf8(*tmpword[i]),
                                 UtfConverter::toUtf8(tr.first),
                                 UtfConverter::toUtf8(refx),
                                 tr.second);
    }
    else if(preBilingual)
    {
      wstring sl;
      wstring tl;
      wstring ref;

      int seenSlash = 0;
      for(wstring::const_iterator it = tmpword[i]->begin(); it != tmpword[i]->end(); it++)
      {
        if(*it == L'\\')
        {
          if(seenSlash == 0)
          {
            sl.push_back(*it);
            it++;
            sl.push_back(*it);
          }
          else if(seenSlash == 1)
          {
            tl.push_back(*it);
            it++;
            tl.push_back(*it);
          }
          else
          {
            ref.push_back(*it);
            it++;
            ref.push_back(*it);
          }
          continue;
        }

        if(*it == L'/')
        {
          seenSlash++;

          ref.clear(); //word after last slash is ref
          continue;
        }
        if(seenSlash == 0)
        {
          sl.push_back(*it);
        }
        else if(seenSlash == 1)
        {
          tl.push_back(*it);
        }
        else
        {
          ref.push_back(*it);
        }
      }
      tr = pair<wstring, int>(tl, false);
      word[i] = new TransferWord(UtfConverter::toUtf8(sl),
                                 UtfConverter::toUtf8(tr.first),
                                 UtfConverter::toUtf8(ref),
                                 tr.second);
    }
    else // neither useBilingual nor preBilingual (sl==tl)
    {
      tr = pair<wstring, int>(*tmpword[i], false);
      wstring refx;
      word[i] = new TransferWord(UtfConverter::toUtf8(*tmpword[i]),
                                 UtfConverter::toUtf8(tr.first),
                                 UtfConverter::toUtf8(refx), 
                                 tr.second);
    }
  }

  words_to_consume = processRule(lastrule);
  lastrule = NULL;

  if(word)
  {
    for(unsigned int i = 0; i != limit; i++)
    {
      delete word[i];
      word[i] = 0; // ToDo: That this changes things means there are much bigger problems elsewhere
    }
    delete[] word;
  }
  if(blank)
  {
    for(unsigned int i = 0; i != limit - 1; i++)
    {
      delete blank[i];
      blank[i] = 0;
    }
    delete[] blank;
  }
  word = NULL;
  blank = NULL;
  tmpword.clear();
  tmpblank.clear();
  ms.init(me->getInitial());
  return words_to_consume;
}

/* HERE */
void
Transfer::applyWord(wstring const &word_str)
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

      case L'/':
        i = limit;
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

      default:
	ms.step(towlower(word_str[i]), any_char);
	break;
    }
  }
  ms.step(L'$');
}

void
Transfer::setPreBilingual(bool value)
{
  preBilingual = value;
}

bool
Transfer::getPreBilingual(void) const
{
  return preBilingual;
}

void
Transfer::setUseBilingual(bool value)
{
  useBilingual = value;
}

bool
Transfer::getUseBilingual(void) const
{
  return useBilingual;
}

void
Transfer::setCaseSensitiveness(bool value)
{
  fstp.setCaseSensitiveMode(value);
}
