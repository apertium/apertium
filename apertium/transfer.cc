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
  // Read transfer rules data from .t*x.bin file
  cerr << "readData" << endl; // di

  alphabet.read(in);
  cerr << "Alphabet size: " << alphabet.size() << endl; // di

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
  cerr << "readBil" << endl; // di
  FILE *in = fopen(fstfile.c_str(), "rb");
  if(!in)
  {
    cerr << "Error: Could not open file '" << fstfile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  fstp.load(in);
  fstp.initBiltrans();
  fclose(in);
}

void
Transfer::setExtendedDictionary(string const &fstfile)
{ 
  cerr << "setExtendedDictionary" << endl; // di
  FILE *in = fopen(fstfile.c_str(), "rb");
  if(!in)
  {
    cerr << "Error: Could not open extended dictionary file '" << fstfile << "'." << endl;
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
  // read and parse .t*x transfer file
  readTransfer(transferfile);

  // open precompiled .t*x.bin file and read data from it
  cerr << "Reading data from " << datafile.c_str() << endl;
  FILE *in = fopen(datafile.c_str(), "rb");
  if(!in)
  {
    cerr << "Error: Could not open file '" << datafile << "'." << endl;
    exit(EXIT_FAILURE);
  }
  readData(in);
  fclose(in);

  // read data from fstfile if specified
  if(fstfile != "")
  {
    cerr << "Reading fst data from " << fstfile << endl; // di
    readBil(fstfile);
  }
}

void
Transfer::readTransfer(string const &in)
{ 
  // Read transfer rules from .t*x file.
  // In fact, here we collect only default attribute value,
  // macros, and actions specified in rules.
  cerr << "Reading transfer rules from " << in.c_str() << endl; // di
  doc = xmlReadFile(in.c_str(), NULL, 0);
  if(doc == NULL)
  {
    cerr << "Error: Could not parse file '" << in << "'." << endl;
    exit(EXIT_FAILURE);
  }

  root_element = xmlDocGetRootElement(doc);
  //cerr << root_element->properties << endl; // di

  // search through attributes of root element 
  for(xmlAttr *i = root_element->properties; i != NULL; i = i->next)
  { 
    // only check for 'default' attribute
    if(!xmlStrcmp(i->name, (const xmlChar *) "default"))
    {
      // assuming either default="chunk" or something else
      if(!xmlStrcmp(i->children->content, (const xmlChar *) "chunk"))
      {
        // if default="chunk", set it to chunk
        defaultAttrs = chunk; // default value for 'chunk'
      }
      else
      { 
        // if not default="chunk", set it to default
        defaultAttrs = lu; // default value for 'default'
      }
    }
  }

  // search through root's children nodes for macroses & rules
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
  // go through subelements of 'section-rules'
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    { 
      // normally looking at a 'rule' node now
      //cerr << "Looking at " << i->name << endl; // di
      for(xmlAttr *j = i->properties; j != NULL; j = j->next) // di
      { // di
        if(!xmlStrcmp(j->name, (const xmlChar *) "comment")) // di
        { // di
          cerr << "Collecting rule " << xmlNodeListGetString(i->doc, j->children, 1) << endl;   // di           
        } // di
      } // di
      // di
      // go through subelements of this 'rule' node
      for(xmlNode *j = i->children; ; j = j->next)
      {
        // check if subelement is an 'action' node
        if(j->type == XML_ELEMENT_NODE && !xmlStrcmp(j->name, (const xmlChar *) "action"))
        {
          // if so, add it at the end of the rule map
          //cerr << "Collected '" << i->name << "' part '" << j->name << "'" << endl; // di
          rule_map.push_back(j);
          break;
        }
      }
    }
  }
}

void
Transfer::collectMacros(xmlNode *localroot)
{ 
  // go through subelements of 'section-macros'
  // and add all subelements, normally 'def-macros' nodes
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
    wcerr << L"Error in " << UtfConverter::fromUtf8((char *) doc->URL) <<L": line " << element->line << endl;
    return false;
  }
  return true;
}

void
Transfer::evalStringClip(xmlNode *element, string &lemma, int &pos)
{
  // This is a corrupted baby clone of evalString 
  // that only gets lemma and its position from source part
  
  map<xmlNode *, TransferInstr>::iterator it;
  it = evalStringCache.find(element);
  TransferInstr &ti = it->second;

  lemma = "";
  pos = -1;

  if (ti.getType() == ti_clip_tl)
  {
    if (checkIndex(element, ti.getPos(), lword))
    { 
      if (ti.getContent() == "lem") 
      { 
        pos = ti.getPos();
        lemma = word[pos]->source(attr_items["lem"], ti.getCondition());
      }
    }
  }
}

string
Transfer::evalString(xmlNode *element)
{
  // Contrary to its name, this function basically evaluates
  // an xml element and executes appropriate instruction.

  // I believe it is used to evaluate lowest-level action elements, 
  // such as 'clip' or 'lit-tag'.

  // If TransferInstr object corresponding to the element is already
  // in evalStringCache, execute that instruction,
  // if not, first add the instruction to evalStringCache,
  // then call evalString again, and execute that instruction.
  
  // First, let's see what we've got. // di
  if (element->type == XML_ELEMENT_NODE)  // di
  {  // di
    cerr << "Evaluating " << element->name << " "; // di
    for(xmlAttr *prop = element->properties; prop != NULL; prop = prop->next) // di
    {  // di
      cerr << prop->name << "='" << xmlNodeListGetString(element->doc, prop->children, 1) << "' "; // di
    } // di
    cerr << endl; // di
  } // di

  map<xmlNode *, TransferInstr>::iterator it;
  it = evalStringCache.find(element); 

  // Check if the TransferInstr object corresponding to the element
  // is already in evalStringCache...
  if(it != evalStringCache.end())
  { 
    // ...if it is, execute the corresponding instruction...
    TransferInstr &ti = it->second;

    // ...depending on its type 
    switch(ti.getType())
    {
      case ti_clip_sl: // <clip ... side="sl" ...>
        if(checkIndex(element, ti.getPos(), lword))
        {
          return word[ti.getPos()]->source(attr_items[ti.getContent()], ti.getCondition());
        }
        break;

      case ti_clip_tl: // <clip ... side="tl" ...>
        if(checkIndex(element, ti.getPos(), lword))
        {
          return word[ti.getPos()]->target(attr_items[ti.getContent()], ti.getCondition());
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

      case ti_var:
        return variables[ti.getContent()];

      case ti_lit_tag: // <lit-tag .../>
      case ti_lit: // <lit .../>
        return ti.getContent(); // just output what's specified in 'v'

      case ti_b: // <b/>
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

      default:
        return "";
    }
    return "";
  } // end of if(it != evalStringCache.end()) clause

  // The following code is executed if TransferInstr object 
  // corresponding to the element is not in evalStringCache yet.
  // It parses lowest-level element, makes TransferInstr object out of it,
  // and pushes it into evalStringCache.
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
      else
      {
        evalStringCache[element] = TransferInstr(ti_linkto_tl, (const char *) part, pos, (void *) as, queue);
      }
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "sl"))
    {
      evalStringCache[element] = TransferInstr(ti_clip_sl, (const char *) part, pos, NULL, queue);
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
    cerr << "Error: unexpected rvalue expression '" << element->name << "'" << endl;
    exit(EXIT_FAILURE);
  }

  return evalString(element);
} // end of evalString

void
Transfer::processOut(xmlNode *localroot)
{ 
  // apply 'out' subelement of a rule, one subelement at a time,
  // depending on subelement type
  cerr << "Applying 'out' element" << endl; // di
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
              //cerr << myword << endl; // di
            }
	  }
	  if(myword != "")
	  {
  	    fputwc_unlocked(L'^', output);
   	    fputws_unlocked(UtfConverter::fromUtf8(myword).c_str(), output);
	    fputwc_unlocked(L'$', output);
            //cerr << UtfConverter::fromUtf8(myword).c_str() << endl; // di
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
                  cerr << myword << endl;
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
          fputws_unlocked(UtfConverter::fromUtf8(evalString(i)).c_str(), output);
        }
      }
      else
      {
        if(!xmlStrcmp(i->name, (const xmlChar *) "chunk"))
        {
          string processed = processChunk(i);
          fputws_unlocked(UtfConverter::fromUtf8(processed).c_str(), output);
          cerr << "Hey, I just made a chunk: " << processed << endl; // di
        }
        else // 'b'
        {
          fputws_unlocked(UtfConverter::fromUtf8(evalString(i)).c_str(), output);
        }
      }
    }
  }
} // end of processOut

string
Transfer::processChunk(xmlNode *localroot)
{ 
  // apply 'chunk' subelement of 'out' element of a rule,
  // one subelement at a time, depending on subelement type

  cerr << "Applying 'chunk' element" << endl; // di
  string name, namefrom;
  string caseofchunk = "aa";
  string result;

  // this will be the cache of source language lemmas found in chunk
  unsigned int limit = tmpword.size();
  string* wordcache;
  wordcache = new string [limit];

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

  // starting to build the chunk
  result.append("^");
  cerr << result << endl; // di

  // adding chunk name
  if(caseofchunk != "")
  {
    if(name != "")
    {
      result.append(copycase(variables[caseofchunk], name));
      cerr << result << endl; // di
    }
    else if(namefrom != "")
    {
      result.append(copycase(variables[caseofchunk], variables[namefrom]));
      cerr << result << endl; // di
    }
    else
    {
      cerr << "Error: you must specify either 'name' or 'namefrom' for the 'chunk' element" << endl;
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    if(name != "")
    {
      result.append(name);
      cerr << result << endl; // di
    }
    else if(namefrom != "")
    {
      result.append(variables[namefrom]);
      cerr << result << endl; // di
    }
    else
    {
      cerr << "Error: you must specify either 'name' or 'namefrom' for the 'chunk' element" << endl;
      exit(EXIT_FAILURE);
    }
  }

  // processing and adding chunk subelements one element at a time
  int count = 0; // di
  for(xmlNode *i = localroot->children; i != NULL; i = i->next)
  {
    if(i->type == XML_ELEMENT_NODE)
    { 
      cerr << "chunk element # " << count << ": " << i->name << endl; // di
      count++; // di
      if(!xmlStrcmp(i->name, (const xmlChar *) "tags"))
      {
        // add chunk tags
        result.append(processTags(i));
        result.append("{");
        cerr << result << endl; // di
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "lu"))
      {
        // process and add one 'lu' element
        string myword;
        string untouched;
        int untouched_pos;

        for(xmlNode *j = i->children; j != NULL; j = j->next)
        {
          if(j->type == XML_ELEMENT_NODE)
          {
            cerr << "Executing " << j->name << endl; // di
            myword.append(evalString(j));

            evalStringClip(j, untouched, untouched_pos); // black magic
            if(untouched_pos != -1)
            {
              //cerr << "Got untouched: " << untouched_pos << ", " << untouched << endl; // di
              wordcache[untouched_pos].append(untouched);
            }
          }
        }
        if(myword != "")
        {
          //cerr << myword << endl;
          result.append("^");
          result.append(myword);
          result.append("$");
          //cerr << result << endl;
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
          cerr << myword << endl; // di
          result.append("^");
          result.append(myword);
          result.append("$");
          cerr << result << endl; // di
        }
      }
      else // 'b'
      {
        result.append(evalString(i));
        cerr << result << endl; // di
      }
    }
  }

  // finishing the chunk
  result.append("}$");

  // now it's time to check if there was a magic word in the chunk
  bool stopword = false;
  for (int k = 0; k < limit && !stopword; k++)
  {
    if (wordcache[k] == "море")
    {
      stopword = true;
    }
  }
  if (stopword)
  {
    result = "^untouchable<SN>{";
    for (int k = 0; k < limit-1; k++)
    {
      result.append("^");
      result.append(wordcache[k]);
      result.append("$ ");
    }
    result.append("^");
    result.append(wordcache[limit-1]);
    result.append("$}$");
  }
  return result;
} // end of processChunk

string
Transfer::processTags(xmlNode *localroot)
{ 
  cerr << "processTags" << endl; // di
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
  // process instruction specified in rule action based on its name
  cerr << "Processing instruction '" << localroot->name << "'" << endl; // di

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
  cerr << "processRejectCurrentRule" << endl; // di
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
  cerr << "processLet" << endl; // di
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
        word[ti.getPos()]->setSource(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
        return;

      case ti_clip_tl:
        word[ti.getPos()]->setTarget(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
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
        as = i->children->content;
      }
    }

    if(!xmlStrcmp(side, (const xmlChar *) "tl"))
    {
      word[pos]->setTarget(attr_items[(const char *) part], evalString(rightSide), queue);
      evalStringCache[leftSide] = TransferInstr(ti_clip_tl, (const char *) part, pos, NULL, queue);
    }
    else
    {
      word[pos]->setSource(attr_items[(const char *) part], evalString(rightSide), queue);
      evalStringCache[leftSide] = TransferInstr(ti_clip_sl, (const char *) part, pos, NULL, queue);
    }
  }
}

void
Transfer::processAppend(xmlNode *localroot)
{ 
  cerr << "processAppend" << endl; // di
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
  cerr << "processModifyCase" << endl; // di
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
      word[pos]->setSource(attr_items[(const char *) part], result);
    }
    else
    {
      string const result = copycase(evalString(rightSide),
				     word[pos]->target(attr_items[(const char *) part], queue));
      word[pos]->setTarget(attr_items[(const char *) part], result);
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
  cerr << "processCallMacro" << endl; // di
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
  
  delete[] myword;
  delete[] myblank;
}

int
Transfer::processChoose(xmlNode *localroot)
{ 
  cerr << "processChoose" << endl; // di
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
  cerr << "processLogical" << endl; // di
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
  cerr << "processIn" << endl; // di
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
  cerr << "processTest" << endl; // di
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
  cerr << "processAnd" << endl; // di
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
  cerr << "processOr" << endl; // di
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
  cerr << "processNot" << endl; // di
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
  cerr << "processEqual" << endl; // di
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
  cerr << "beginsWith" << endl; // di
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
  cerr << "endsWith" << endl; // di
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
  cerr << "processBeginsWith" << endl; // di
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
  cerr << "processEndsWith" << endl; // di
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
  cerr << "processBeginsWithList" << endl; // di
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
  cerr << "processEndsWithList" << endl; // di
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
  cerr << "processContainsSubstring" << endl; // di
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
  cerr << "copycase" << endl; // di
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
  cerr << "caseOf" << endl; // di
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
  cerr << "tolower" << endl; // di
  return UtfConverter::toUtf8(StringUtils::tolower(UtfConverter::fromUtf8(str)));
}

string
Transfer::tags(string const &str) const
{ 
  //cerr << "tags" << endl; // di
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
  cerr << "processRule" << endl; // di
  int instruction_return, words_to_consume = -1;
  // localroot is supposed to be an 'action' tag
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
  cerr << "readToken" << endl; // di
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  wstring content;
  while(true)
  {
    int val = fgetwc_unlocked(in);
    //wcerr << UtfConverter::toUtf8(wchar_t(val)) << endl; // di

    if(feof(in) || (val == 0 && internal_null_flush))
    {
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(val == '\\')
    {
      content += L'\\';
      content += (wchar_t) fgetwc_unlocked(in);
      //wcerr << content << endl; // di
    }
    else if(val == L'[')
    {
      content += L'[';
      //wcerr << content << endl; // di
      while(true)
      {
	int val2 = fgetwc_unlocked(in);
	if(val2 == L'\\')
	{
	  content += L'\\';
	  content += wchar_t(fgetwc_unlocked(in));
          //wcerr << content << endl; // di
	}
	else if(val2 == L']')
	{
	  content += L']';
          //wcerr << content << endl; // di
	  break;
	}
	else
	{
	  content += wchar_t(val2);
          //cerr << UtfConverter::toUtf8(content) << endl; // di
	}
      }
    }
    else if(val == L'$')
    {
      cerr << UtfConverter::toUtf8(content) << endl;
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == L'^')
    {
      cerr << UtfConverter::toUtf8(content) << endl;
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
} // end of readToken

bool
Transfer::getNullFlush(void)
{ 
  cerr << "getNullFlush" << endl; // di
  return null_flush;
}

void
Transfer::setNullFlush(bool null_flush)
{ 
  cerr << "setNullFlush" << endl; // di
  this->null_flush = null_flush;
}

void
Transfer::setTrace(bool trace)
{ 
  cerr << "setTrace" << endl; // di
  this->trace = trace;
}

void
Transfer::setTraceATT(bool trace)
{ 
  cerr << "setTraceATT" << endl; // di
  this->trace_att = trace;
}

void
Transfer::transfer_wrapper_null_flush(FILE *in, FILE *out)
{ 
  cerr << "transfer_wrapper_null_flush" << endl; // di
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
  cerr << endl << "transfer starts" << endl << endl; // di

  if(getNullFlush())
  {
    transfer_wrapper_null_flush(in, out);
  }

  int last = 0;
  int prev_last = 0;
  int lastrule_id = -1;
  set<int> banned_rules;

  output = out;
  ms.init(me->getInitial());

  int counter = 0; // di
  while(true)
  { 
    cerr << endl << "Transfer iteration # " << counter << endl; // di
    cerr << "last: " << last << endl; // di
    cerr << "prev_last: " << prev_last << endl; // di
    cerr << "lastrule_id: " << lastrule_id << endl; // di
    cerr << "ms.size(): " << ms.size() << endl; // di
    // Let's look at input_buffer contents // di
    int initbuffpos = input_buffer.getPos(); // di
    cerr << "input_buffer position: " << initbuffpos << endl << endl; // di
    input_buffer.setPos(0); // di
    int currbuffpos, prevbuffpos = input_buffer.getPos(); // di
    TransferToken currbufftok, prevbufftok = input_buffer.next(); // di
    bool run = true; // di
    while (run) { // di
      currbuffpos = input_buffer.getPos(); // di
      currbufftok = input_buffer.next(); // di
      cerr << "input_buffer.buf[" << prevbuffpos << "]: " << UtfConverter::toUtf8(prevbufftok.getContent()) << endl; // di
      if (currbuffpos == prevbuffpos) { // di
        run = false; // di
      } else { // di
        prevbuffpos = currbuffpos; // di
        prevbufftok = currbufftok; // di
      } // di
    } // di
    cerr << endl; // di
    // Return input_buffer to its initial position // di
    input_buffer.setPos(initbuffpos); // di
    // List banned_rules //di
    //cerr << "banned_rules:" << endl;
    //for(set<int>::iterator iter=banned_rules.begin(); iter != banned_rules.end(); iter++) { // di
    //    cerr << *iter << ", "; // di
    //} // di
    //cerr << endl; // di

    if(trace_att)
    {
      cerr << "Loop start " << endl;
      cerr << "ms.size: " << ms.size() << endl;

      cerr << "tmpword.size(): " << tmpword.size() << endl;

      for (unsigned int ind = 0; ind < tmpword.size(); ind++)
      {
        if(ind != 0)
        {
          wcerr << L" ";
        }
        wcerr << *tmpword[ind];
      }
      wcerr << endl;

      cerr << "tmpblank.size(): " << tmpblank.size() << endl;
      for (unsigned int ind = 0; ind < tmpblank.size(); ind++)
      {
        wcerr << L"'";
        wcerr << *tmpblank[ind];
        wcerr << L"' ";
      }
      wcerr << endl;

      cerr << "last: " << last << endl;
      cerr << "prev_last: " << prev_last << endl << endl;
    } // if(trace_att) ends here 

    if (ms.size() == 0)
    { 
      cerr << "(ms.size() == 0)" << endl; // di
      if(lastrule != NULL)
      {
        // this is the branch where a rule specified by lastrule_id is applied
        cerr << "lastrule != NULL" << endl; // di
        int num_words_to_consume = applyRule();

        if(trace_att)
        {
          cerr << "num_words_to_consume: " << num_words_to_consume << endl;
        }

        //Consume all the words from the input which matched the rule.
        //This piece of code is executed unless the rule contains a "reject-current-rule" instruction
        if(num_words_to_consume < 0)
        {
          cerr << "num_words_to_consume < 0" << endl; // di
          banned_rules.clear();
          input_buffer.setPos(last);
        }
        else if(num_words_to_consume > 0)
        {
          cerr << "num_words_to_consume > 0" << endl; // di
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
          cerr << "num_words_to_consume == 0" << endl; // di
          //Add rule to banned rules
          banned_rules.insert(lastrule_id);
          input_buffer.setPos(prev_last);
          input_buffer.next();
          last = input_buffer.getPos();
        } // thy words consumed
        lastrule_id = -1;
      }
      else // lastrule == NULL
      {
        if(tmpword.size() != 0)
        {
          if(trace_att)
          {
            cerr << "printing tmpword[0]" <<endl;
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
                else
                {
                  tl.push_back(*it);
                  it++;
                  tl.push_back(*it);
                }
                continue;
              }
              else if(*it == L'/')
              {
                seenSlash++;
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
              else if(seenSlash > 1)
              {
                break;
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
          cerr << "tmpblank.size() != 0" << endl;
          if(trace_att)
          {
            cerr << "printing tmpblank[0]" <<endl;
          }
          fputws_unlocked(tmpblank[0]->c_str(), output);
          tmpblank.clear();
          prev_last = last;
          last = input_buffer.getPos();
          ms.init(me->getInitial());
	}
      }
    } // if(ms.size() == 0) ends here

    int val = ms.classifyFinals(me->getFinals(), banned_rules);
    if(val != -1)
    {
      lastrule = rule_map[val-1];
      lastrule_id = val;
      last = input_buffer.getPos();

      if(trace)
      {
        wcerr << endl << L"apertium-transfer: Rule " << val << L" ";
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
          cerr << endl << "transfer ends" << endl << endl;
	  return;
	}
	break;

      default:
	cerr << "Error: Unknown input token." << endl;
	return;
    }
    counter++;
  }
} // end of transfer

int
Transfer::applyRule()
{ 
  cerr << "applyRule" << endl; // di
  cerr << "limit " << tmpword.size() << endl; // di
  //wcerr << UtfConverter::toUtf8(*tmpword[0]) << endl; // di
  
  int words_to_consume;
  unsigned int limit = tmpword.size();

  for(unsigned int i = 0; i != limit; i++)
  { 
    cerr << "applyRule iteration # " << i << endl; // di
    if(i == 0)
    { 
      cerr << "i == 0" << endl; // di
      word = new TransferWord *[limit];
      lword = limit;
      if(limit != 1)
      { 
        cerr << "limit != 1" << endl; // di
        blank = new string *[limit - 1];
        lblank = limit - 1;
      }
      else
      { 
        cerr << "not limit != 1" << endl; // di
        blank = NULL;
        lblank = 0;
      }
    }
    else
    { 
      cerr << "not i == 0" << endl; // di
      blank[i-1] = new string(UtfConverter::toUtf8(*tmpblank[i-1]));
    }

    pair<wstring, int> tr;
    if(useBilingual && preBilingual == false)
    { 
      cerr << "useBilingual && preBilingual == false" << endl; // di
      tr = fstp.biltransWithQueue(*tmpword[i], false);
      cerr << i << " ";
      wcerr << tr.first << " ";
      cerr << tr.second << endl;
    }
    else if(preBilingual)
    { 
      // this part is dedicated to splitting token by slash, e.g.
      // if tmpword[i] was word_in_lang1<its><tags>/word_in_lang2<its><tags> 
      // then
      // sl = word_in_lang1<its><tags>
      // tl = word_in_lang2<its><tags>
      cerr << "preBilingual" << endl; // di
      wstring sl;
      wstring tl;
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
          else
          {
            tl.push_back(*it);
            it++;
            tl.push_back(*it);
          }
          continue;
        }

        if(*it == L'/')
        {
          seenSlash++;
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
        else if(seenSlash > 1)
        {
          break;
        }
      }
      cerr << UtfConverter::toUtf8(sl) << endl; // di
      wcerr << tl << endl; // di
      //tmpword[i]->assign(sl);
      tr = pair<wstring, int>(tl, false);
    }
    else
    { 
      // here we don't need to split anything
      cerr << "else" << endl; // di
      tr = pair<wstring, int>(*tmpword[i], false);
    }

    //wcerr << tr.first << endl; // di
    word[i] = new TransferWord(UtfConverter::toUtf8(*tmpword[i]),
			       UtfConverter::toUtf8(tr.first), tr.second);
    //cerr << i << " "; // di
    //wcerr << UtfConverter::fromUtf8(word[i]) << endl; // di
  }

  words_to_consume = processRule(lastrule);

  // some cleanup ?
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
  cerr << "wtc: " << words_to_consume << endl; // di
  return words_to_consume;
} // end of applyRule

/* HERE */
void
Transfer::applyWord(wstring const &word_str)
{
  // Here, the token contained in word_str is fed 
  // to the fst by stepping with ms

  cerr << "applyWord: applying to " << UtfConverter::toUtf8(word_str) << endl; // di
  ms.step(L'^');
  for(unsigned int i = 0, limit = word_str.size(); i < limit; i++)
  {
    switch(word_str[i])
    {
      case L'\\':
        i++;
	ms.step(towlower(word_str[i]), any_char);
	break;

      case L'/': // got to the end of left side part (source token)
        i = limit;
        break;

      case L'<': // got to the start of a tag
	for(unsigned int j = i+1; j != limit; j++)
	{
	  if(word_str[j] == L'>') // got to the end of the tag
	  {
            // try to get the symbol corresponding to the tag
	    int symbol = alphabet(word_str.substr(i, j-i+1));
	    if(symbol) // there is such symbol in alphabet
	    {
	      ms.step(symbol, any_tag);
	    }
	    else // there is no such symbol in alphabet
	    {
	      ms.step(any_tag);
	    }
	    i = j;
	    break;
	  }
	}
	break;

      default: // default is applying lemma's symbols one by one
	ms.step(towlower(word_str[i]), any_char);
	break;
    }
  }
  ms.step(L'$'); // push the end of token
  //cerr << UtfConverter::toUtf8(word_str) << endl; // di
} // end of applyWord

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
