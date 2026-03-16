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
#include <apertium/transfer.h>

#include <lttoolbox/string_utils.h>
#include <lttoolbox/xml_walk_util.h>

#include <iostream>

using namespace std;

Transfer::Transfer()
{}

void
Transfer::readBil(string const &fstfile)
{
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
  TransferBase::read(transferfile.c_str(), datafile.c_str());
  if (getattr(root_element, "default") == "chunk"_u) {
    defaultAttrs = chunk;
  } else {
    defaultAttrs = lu;
  }
  if (!fstfile.empty()) {
    readBil(fstfile);
  }
}

bool
Transfer::checkIndex(xmlNode *element, int index, int limit)
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
Transfer::evalCachedString(xmlNode *element)
{
  TransferInstr& ti = evalStringCache[element];
  switch (ti.getType()) {
  case ti_clip_sl:
    if (checkIndex(element, ti.getPos(), lword)) {
      if (gettingLemmaFromWord(ti.getContent()) && last_lword > 1) {
        if(in_lu) {
          out_wblank = combineWblanks(out_wblank, word[ti.getPos()]->getWblank());
        } else if (in_let_var) {
          var_out_wblank[var_val] = combineWblanks(var_out_wblank[var_val], word[ti.getPos()]->getWblank());
        }
      }

      return word[ti.getPos()]->source(attr_items[ti.getContent()], ti.getCondition());
    }
    break;

  case ti_clip_tl:
    if(checkIndex(element, ti.getPos(), lword)) {
      if(gettingLemmaFromWord(ti.getContent()) && last_lword > 1) {
        if(in_lu) {
          out_wblank = combineWblanks(out_wblank, word[ti.getPos()]->getWblank());
        } else if(in_let_var) {
          var_out_wblank[var_val] = combineWblanks(var_out_wblank[var_val], word[ti.getPos()]->getWblank());
        }
      }

      return word[ti.getPos()]->target(attr_items[ti.getContent()], ti.getCondition());
    }
    break;

  case ti_clip_ref:
    if(checkIndex(element, ti.getPos(), lword)) {
      return word[ti.getPos()]->reference(attr_items[ti.getContent()], ti.getCondition());
    }
    break;

  case ti_linkto_sl:
    if(checkIndex(element, ti.getPos(), lword)) {
      if(!word[ti.getPos()]->source(attr_items[ti.getContent()], ti.getCondition()).empty()) {
        UString ret;
        ret += '<';
        ret += ti.getStrval();
        ret += '>';
        return ret;
      } else {
        return ""_u;
      }
    }
    break;

  case ti_linkto_tl:
    if(checkIndex(element, ti.getPos(), lword)) {
      if(!word[ti.getPos()]->target(attr_items[ti.getContent()], ti.getCondition()).empty()) {
        UString ret;
        ret += '<';
        ret += ti.getStrval();
        ret += '>';
        return ret;
      } else {
        return ""_u;
      }
    }
    break;

  case ti_linkto_ref:
    if(checkIndex(element, ti.getPos(), lword)) {
      if(!word[ti.getPos()]->reference(attr_items[ti.getContent()], ti.getCondition()).empty()) {
        UString ret;
        ret += '<';
        ret += ti.getStrval();
        ret += '>';
        return ret;
      } else {
        return ""_u;
      }
    }
    break;

  case ti_var:
    if(last_lword > 1) {
      out_wblank = combineWblanks(out_wblank, var_out_wblank[ti.getContent()]);
    }
    return variables[ti.getContent()];

  case ti_lit_tag:
  case ti_lit:
    return ti.getContent();

  case ti_b:
    if(!blank_queue.empty()) {
      UString retblank = blank_queue.front();
      if(in_out) {
        blank_queue.pop();
      }

      return retblank;
    } else {
      return " "_u;
    }
    break;

  case ti_get_case_from:
    if(checkIndex(element, ti.getPos(), lword)) {
      return copycase(word[ti.getPos()]->source(attr_items[ti.getContent()]),
                      evalString(ti.getPointer()));
    }
    break;

  case ti_case_of_sl:
    if(checkIndex(element, ti.getPos(), lword)) {
      return StringUtils::getcase(word[ti.getPos()]->source(attr_items[ti.getContent()]));
    }
    break;

  case ti_case_of_tl:
    if(checkIndex(element, ti.getPos(), lword)) {
      return StringUtils::getcase(word[ti.getPos()]->target(attr_items[ti.getContent()]));
    }
    break;

  case ti_case_of_ref:
    if(checkIndex(element, ti.getPos(), lword)) {
      return StringUtils::getcase(word[ti.getPos()]->reference(attr_items[ti.getContent()]));
    }
    break;

  default:
    return ""_u;
  }
  return ""_u;
}

void
Transfer::processClip(xmlNode* element)
{
  int pos = 0;
  xmlChar *side = NULL;
  UString as;
  UString part;
  bool queue = true;

  for(xmlAttr *i = element->properties; i != NULL; i = i->next) {
    if(!xmlStrcmp(i->name, (const xmlChar *) "side")) {
      side = i->children->content;
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "part")) {
      part = to_ustring((const char*) i->children->content);
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "pos")) {
      pos = atoi((const char *)i->children->content) - 1;
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "queue")) {
      if(!xmlStrcmp(i->children->content, (const xmlChar *) "no")) {
        queue = false;
      }
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "link-to")) {
      as = to_ustring((const char*)i->children->content);
    }
  }

  if(!as.empty()) {
    if(!xmlStrcmp(side, (const xmlChar *) "sl")) {
      evalStringCache[element] = TransferInstr(ti_linkto_sl, part, pos, NULL, queue, as);
    } else if(!xmlStrcmp(side, (const xmlChar *) "ref")) {
      evalStringCache[element] = TransferInstr(ti_linkto_ref, part, pos, NULL, queue, as);
    } else {
      evalStringCache[element] = TransferInstr(ti_linkto_tl, part, pos, NULL, queue, as);
    }
  } else if(!xmlStrcmp(side, (const xmlChar *) "sl")) {
    evalStringCache[element] = TransferInstr(ti_clip_sl, part, pos, NULL, queue);
  } else if(!xmlStrcmp(side, (const xmlChar *) "ref")) {
    evalStringCache[element] = TransferInstr(ti_clip_ref, part, pos, NULL, queue);
  } else {
    evalStringCache[element] = TransferInstr(ti_clip_tl, part, pos, NULL, queue);
  }
}

void
Transfer::processBlank(xmlNode* element)
{
  evalStringCache[element] = TransferInstr(ti_b, " "_u, -1);
}

void
Transfer::processCaseOf(xmlNode* element)
{
  int pos = 0;
  xmlChar *side = NULL;
  UString part;

  for(xmlAttr *i = element->properties; i != NULL; i = i->next) {
    if(!xmlStrcmp(i->name, (const xmlChar *) "side")) {
      side = i->children->content;
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "part")) {
      part = to_ustring((const char*) i->children->content);
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "pos")) {
      pos = atoi((const char *) i->children->content) - 1;
    }
  }

  if(!xmlStrcmp(side, (const xmlChar *) "sl")) {
    evalStringCache[element] = TransferInstr(ti_case_of_sl, part, pos);
  } else if(!xmlStrcmp(side, (const xmlChar *) "ref")) {
    evalStringCache[element] = TransferInstr(ti_case_of_ref, part, pos);
  } else {
    evalStringCache[element] = TransferInstr(ti_case_of_tl, part, pos);
  }
}

UString
Transfer::processLu(xmlNode* element)
{
  in_lu = true;
  out_wblank.clear();

  UString myword;
  for (auto i : children(element)) {
    myword.append(evalString(i));
  }

  in_lu = false;

  // if word == nullptr then we're outputting a word
  // inside an npar=0 macro and there's not much we can do
  if(last_lword == 1 && word != nullptr) {
    out_wblank = word[0]->getWblank();
  }

  if(!myword.empty()) {
    if(myword[0] != '[' || myword[1] != '[') {
      UString ret = out_wblank;
      ret += '^';
      ret += myword;
      ret += '$';
      return ret;
    } else {
      myword += '$';
      return myword;
    }
  } else {
    return ""_u;
  }
}

UString
Transfer::processMlu(xmlNode* element)
{
  UString value;

  bool first_time = true;
  out_wblank.clear();

  in_lu = true;
  for (auto i : children(element)) {
    UString myword;
    for (auto j : children(i)) {
      myword.append(evalString(j));
    }

	if (!first_time) {
	  if(!myword.empty() && myword[0] != '#') { //'+#' problem
        value += '+';
      }
	} else {
      if (!myword.empty()) {
	    first_time = false;
      }
	}

	value.append(myword);
  }

  if(last_lword == 1) {
    out_wblank = word[0]->getWblank();
  }

  if(!value.empty()) {
    UString ret = out_wblank;
    ret += '^';
    ret += value;
    ret += '$';
    return ret;
  } else {
    return ""_u;
  }
}

void
Transfer::processLuCount(xmlNode* element)
{
  cerr << "Error: unexpected expression: '" << element->name << "'" << endl;
  exit(EXIT_FAILURE);
}

void
Transfer::processOut(xmlNode *localroot)
{
  in_out = true;

  for (auto i : children(localroot)) {
    if(defaultAttrs == lu) {
      if(!xmlStrcmp(i->name, (const xmlChar *) "lu")) {
        write(processLu(i), output);
      } else if(!xmlStrcmp(i->name, (const xmlChar *) "mlu")) {
        write(processMlu(i), output);
      } else { // 'b'
        write(evalString(i), output);
      }
    } else {
      if(!xmlStrcmp(i->name, (const xmlChar *) "chunk")) {
        write(processChunk(i), output);
      } else { // 'b'
        write(evalString(i), output);
      }
    }
  }
  in_out = false;
}

UString
Transfer::processChunk(xmlNode *localroot)
{
  UString name, namefrom;
  UString caseofchunk;
  UString result;

  for(xmlAttr *i = localroot->properties; i != NULL; i = i->next)
  {
    if(!xmlStrcmp(i->name, (const xmlChar *) "name"))
    {
      name = to_ustring((const char *) i->children->content);
    }
    else if(!xmlStrcmp(i->name, (const xmlChar *) "namefrom"))
    {
      namefrom = to_ustring((const char *) i->children->content);
    }
    else if(!xmlStrcmp(i->name, (const xmlChar *) "case"))
    {
      caseofchunk = to_ustring((const char *) i->children->content);
    }
  }

  result += '^';
  if(!caseofchunk.empty() && !dictionary_case)
  {
    if(!name.empty())
    {
      result.append(StringUtils::copycase(variables[caseofchunk], name));
    }
    else if(!namefrom.empty())
    {
      result.append(StringUtils::copycase(variables[caseofchunk], variables[namefrom]));
    }
    else
    {
      cerr << "Error: you must specify either 'name' or 'namefrom' for the 'chunk' element" << endl;
      exit(EXIT_FAILURE);
    }
  }
  else
  {
    if(!name.empty())
    {
      result.append(name);
    }
    else if(!namefrom.empty())
    {
      result.append(variables[namefrom]);
    }
    else
    {
      cerr << "Error: you must specify either 'name' or 'namefrom' for the 'chunk' element" << endl;
      exit(EXIT_FAILURE);
    }
  }

  for (auto i : children(localroot)) {
    if(!xmlStrcmp(i->name, (const xmlChar *) "tags")) {
      result.append(processTags(i));
      result += '{';
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "lu")) {
      result.append(processLu(i));
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "mlu")) {
      result.append(processMlu(i));
    } else { // 'b'
      result.append(evalString(i));
    }
  }
  result += '}';
  result += '$';
  return result;
}

UString
Transfer::processTags(xmlNode *localroot)
{
  UString result;
  for (auto i : children(localroot)) {
    if (!xmlStrcmp(i->name, (const xmlChar*) "tag")) {
      for (auto j : children(i)) {
        result.append(evalString(j));
      }
    }
  }
  return result;
}

void
Transfer::processLet(xmlNode *localroot)
{
  xmlNode *leftSide = NULL, *rightSide = NULL;

  for (auto i : children(localroot)) {
    if(leftSide == NULL) {
      leftSide = i;
    } else {
      rightSide = i;
      break;
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

      case ti_clip_sl:
        if (checkIndex(leftSide, ti.getPos(), lword)) {
          bool match = word[ti.getPos()]->setSource(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
          if (!match && trace)
          {
            cerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
          }
        }
        return;

      case ti_clip_tl:
        if (checkIndex(leftSide, ti.getPos(), lword)) {
          bool match = word[ti.getPos()]->setTarget(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
          if (!match && trace)
          {
            cerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
          }
        }
        return;

      case ti_clip_ref:
        if (checkIndex(leftSide, ti.getPos(), lword)) {
          bool match = word[ti.getPos()]->setReference(attr_items[ti.getContent()], evalString(rightSide), ti.getCondition());
          if (!match && trace)
          {
            cerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
          }
        }
        return;

      default:
        return;
    }
  }
  if(leftSide->name != NULL && !xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    in_let_var = true;

    UString const val = to_ustring((const char *) leftSide->properties->children->content);

    var_val = val;
    var_out_wblank[var_val].clear();

    variables[val] = evalString(rightSide);

    in_let_var = false;
    evalStringCache[leftSide] = TransferInstr(ti_var, val, 0);
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    xmlChar *side = NULL, *as = NULL;
    UString part;
    bool queue = true;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "side"))
      {
	side = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((const char*) i->children->content);
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
      cerr << "Error: Transfer::processLet() bad access on pos >= lword" << endl;
      return;
    }
    if (word[pos] == 0) {
      cerr << "Error: Transfer::processLet() null access on word[pos]" << endl;
      return;
    }

    if(!xmlStrcmp(side, (const xmlChar *) "tl"))
    {
      bool match = word[pos]->setTarget(attr_items[part], evalString(rightSide), queue);
      if(!match && trace)
      {
        cerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
      }
      evalStringCache[leftSide] = TransferInstr(ti_clip_tl, part, pos, NULL, queue);
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
    {
      bool match = word[pos]->setReference(attr_items[part], evalString(rightSide), queue);
      if(!match && trace)
      {
        cerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
      }
      evalStringCache[leftSide] = TransferInstr(ti_clip_ref, part, pos, NULL, queue);
    }
    else
    {
      bool match = word[pos]->setSource(attr_items[part], evalString(rightSide), queue);
      if(!match && trace)
      {
        cerr << "apertium-transfer warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
      }
      evalStringCache[leftSide] = TransferInstr(ti_clip_sl, part, pos, NULL, queue);
    }
  }
}

void
Transfer::processModifyCase(xmlNode *localroot)
{
  if (dictionary_case) return;
  xmlNode *leftSide = NULL, *rightSide = NULL;

  for (auto i : children(localroot)) {
    if(leftSide == NULL) {
      leftSide = i;
    } else {
      rightSide = i;
      break;
    }
  }

  if(leftSide->name != NULL && !xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    xmlChar *side = NULL, *as = NULL;
    UString part;
    bool queue = true;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "side"))
      {
	side = i->children->content;
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((const char*)i->children->content);
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
      UString const result = StringUtils::copycase(evalString(rightSide),
				      word[pos]->source(attr_items[part], queue));
      bool match = word[pos]->setSource(attr_items[part], result);
      if(!match && trace)
      {
        cerr << "apertium-transfer warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
      }
    }
    else if(!xmlStrcmp(side, (const xmlChar *) "ref"))
    {
      UString const result = StringUtils::copycase(evalString(rightSide),
              word[pos]->reference(attr_items[part], queue));
      bool match = word[pos]->setReference(attr_items[part], result);
      if(!match && trace)
      {
        cerr << "apertium-transfer warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
      }
    }
    else
    {
      UString const result = StringUtils::copycase(evalString(rightSide),
				     word[pos]->target(attr_items[part], queue));
      bool match = word[pos]->setTarget(attr_items[part], result);
      if(!match && trace)
      {
        cerr << "apertium-transfer warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
      }
    }
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    UString const val = to_ustring((const char *) leftSide->properties->children->content);
    variables[val] = StringUtils::copycase(evalString(rightSide), variables[val]);
  }
}

void
Transfer::processCallMacro(xmlNode *localroot)
{
  UString const n = to_ustring((const char *) localroot->properties->children->content);
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
  int idx = 0;
  if(npar > 0)
  {
    myword = new TransferWord *[npar];
    std::fill(myword, myword+npar, (TransferWord *)(0));
    for (auto i : children(localroot)) {
      if (idx >= npar) {
        cerr << "Error: processCallMacro() number of arguments >= npar at line " << i->line << endl;
        return;
      }
      int pos = atoi((const char *) i->properties->children->content)-1;
      myword[idx] = word[pos];

      idx++;
    }
  }

  swap(myword, word);
  swap(npar, lword);

  for (auto i : children(macro)) {
    processInstruction(i);
  }

  swap(myword, word);
  swap(npar, lword);

  delete[] myword;
}

TransferToken &
Transfer::readToken(InputFile& in)
{
  if(!input_buffer.isEmpty())
  {
    return input_buffer.next();
  }

  UString content;
  while(true)
  {
    UChar32 val = in.get();
    if(in.eof() || (val == 0 && internal_null_flush))
    {
      in_wblank = false;
      return input_buffer.add(TransferToken(content, tt_eof));
    }
    if(in_wblank)
    {
      content = "[["_u;
      content += val;

      while(true)
      {
        UChar32 val3 = in.get();
        if(val3 == '\\')
        {
          content += '\\';
          content += in.get();
        }
        else if(val3 == '$') //[[..]]^..$ is the LU
        {
          in_wblank = false;
          return input_buffer.add(TransferToken(content, tt_word));
        }
        else if(val3 == '\0' && null_flush)
        {
          in_wblank = false;
          u_fflush(output);
        }
        else
        {
          content += val3;
        }
      }
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
        if(val2 == '\\')
        {
          content += '\\';
          content += in.get();
        }
        else if(val2 == '[')
        { //wordbound blank
          in_wblank = true;
          content.pop_back();

          return input_buffer.add(TransferToken(content, tt_blank));
        }
        else if(val2 == ']')
        {
          content += ']';
          break;
        }
        else if(val2 == U_EOF)
        {
          break;                // should not happen
        }
        else if(val2 == '\0')
        {
          in.unget(val2);
          break;
        }
        else
        {
          content += val2;
        }
      }
    }
    else if(val == '$')
    {
      return input_buffer.add(TransferToken(content, tt_word));
    }
    else if(val == '^')
    {
      return input_buffer.add(TransferToken(content, tt_blank));
    }
    else if(val == '\0' && null_flush)
    {
      in_wblank = false;
      u_fflush(output);
    }
    else
    {
      content += val;
    }
  }
}

void
Transfer::setTraceATT(bool trace)
{
  this->trace_att = trace;
}

void
Transfer::tmp_clear()
{
  tmpblank.clear();
  tmpword.clear();
  variables = variable_defaults;
}

void
Transfer::transfer_wrapper_null_flush(InputFile& in, UFILE* out)
{
  null_flush = false;
  internal_null_flush = true;

  while(!in.eof())
  {
    tmp_clear();
    transfer(in, out);
    u_fputc('\0', out);
    u_fflush(out);
  }

  internal_null_flush = false;
  null_flush = true;
}

void
Transfer::transfer(InputFile& in, UFILE* out)
{
  if(getNullFlush())
  {
    transfer_wrapper_null_flush(in, out);
  }

  unsigned int last = input_buffer.getPos();
  unsigned int prev_last = last;
  int lastrule_id = -1;
  set<int> banned_rules;
  in_wblank = false;

  output = out;
  ms.init(me->getInitial());

  while(true)
  {
    if(trace_att)
    {
      cerr << "Loop start " << endl;
      cerr << "ms.size: " << ms.size() << endl;

      cerr << "tmpword.size(): " << tmpword.size() << endl;
      for (unsigned int ind = 0; ind < tmpword.size(); ind++)
      {
        if(ind != 0)
        {
          cerr << " ";
        }
        cerr << *tmpword[ind];
      }
      cerr << endl;

      cerr << "tmpblank.size(): " << tmpblank.size() << endl;
      for (unsigned int ind = 0; ind < tmpblank.size(); ind++)
      {
        cerr << "'";
        cerr << *tmpblank[ind];
        cerr << "' ";
      }
      cerr << endl;

      cerr << "last: " << last << endl;
      cerr << "prev_last: " << prev_last << endl << endl;
    }

    if(ms.size() == 0)
    {
      if(lastrule != NULL)
      {
        if (trace) {
          cerr << endl << "apertium-transfer: Applied rule " << lastrule_id << " line " << rule_lines[lastrule_id-1];
          for (auto& it : tmpword) {
            cerr << " " << *it;
          }
          cerr << endl;
        }

        int num_words_to_consume = applyRule();

        if(trace_att)
        {
          cerr << "num_words_to_consume: " << num_words_to_consume << endl;
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
          while(num_consumed_words < num_words_to_consume && !input_buffer.isEmpty())
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
            cerr << "printing tmpword[0]" <<endl;
          }

          pair<UString, int> tr;
          UString tr_wblank;
          if(useBilingual && preBilingual == false)
          {
            if(isExtended && (*tmpword[0])[0] == '*') {
              tr = extended.biltransWithQueue((*tmpword[0]).substr(1), false);
              if(tr.first[0] == '@') {
                tr.first[0] = '*';
              } else {
                UString temp;
                temp += '%';
                temp.append(tr.first);
                temp.swap(tr.first);
              }
            } else {
              tr = fstp.biltransWithQueue(*tmpword[0], false);
            }
          }
          else if(preBilingual)
          {
            UString sl;
            UString tl;
            UString ref;
            UString wblank;
            UString cur;

            int seenSlash = 0;
            for(UString::const_iterator it = tmpword[0]->begin(); it != tmpword[0]->end(); it++)
            {
              if(*it == '\\') {
                cur.push_back(*it);
                it++;
                cur.push_back(*it);
                continue;
              }
              else if(*it == '[')
              {
                if(*(it+1) == '[') //wordbound blank
                {
                  while(true)
                  {
                    if(*it == '\\')
                    {
                      wblank.push_back(*it);
                      it++;
                      wblank.push_back(*it);
                    }
                    else if(*it == '^' && *(it-1) == ']' && *(it-2) == ']')
                    {
                      break;
                    }
                    else
                    {
                      wblank.push_back(*it);
                    }

                    it++;
                  }
                }
                else
                {
                  cur.push_back(*it);
                }
                continue;
              }
              else if(*it == '/')
              {
                ref.clear();
                switch (seenSlash) {
                case 0: cur.swap(sl); break;
                case 1: cur.swap(tl); break;
                default: cur.swap(ref);
                }
                seenSlash++;
                continue;
              }
              else if (*it == '<') {
                while (*it != '>') {
                  cur.push_back(*it);
                  if (*it == '\\') {
                    it++;
                    cur.push_back(*it);
                  }
                  it++;
                }
              }
              cur.push_back(*it);
            }
            if (!cur.empty()) {
              ref.clear();
              switch (seenSlash) {
              case 0: cur.swap(sl); break;
              case 1: cur.swap(tl); break;
              default: cur.swap(ref);
              }
            }
            //tmpword[0]->assign(sl);
            tr = pair<UString, int>(tl, false);
            tr_wblank = wblank;
            //cerr << "pb: " << *tmpword[0] << " :: " << sl << " >> " << tl << endl ;
          }
          else
          {
            tr = pair<UString, int>(*tmpword[0], 0);
          }

	  if(tr.first.size() != 0) {
	    if(defaultAttrs == lu) {
          if(tr.first[0] != '[' || tr.first[1] != '[') {
            u_fprintf(output, "%S^", tr_wblank.c_str());
          }
          u_fprintf(output, "%S$", tr.first.c_str());
        } else {
          if(tr.first[0] == '*') {
            u_fprintf(output, "^unknown<unknown>{%S^", tr_wblank.c_str());
          } else {
            u_fprintf(output, "^default<default>{%S^", tr_wblank.c_str());
          }
          u_fprintf(output, "%S$}$", tr.first.c_str());
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
          if(trace_att) {
            cerr << "printing tmpblank[0]" <<endl;
          }
          write(*tmpblank[0], output);
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
      last_lword = tmpword.size();

      if(trace) {
        cerr << endl << "apertium-transfer: Matched rule " << val << " line " << lastrule_line;
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

int
Transfer::applyRule()
{
  int words_to_consume;
  unsigned int limit = tmpword.size();
  //cerr << "applyRule: " << tmpword.size() << endl;

  for(unsigned int i = 0; i != limit; i++)
  {
    if(i == 0)
    {
      word = new TransferWord *[limit];
      std::fill(word, word+limit, (TransferWord *)(0));
      lword = limit;
    }
    else
    {
      if(int(blank_queue.size()) < last_lword - 1)
      {
        blank_queue.push(*tmpblank[i-1]);
      }
    }

    if(useBilingual && preBilingual == false) {
      auto tr = fstp.biltransWithQueue(*tmpword[i], false);
      word[i] = new TransferWord(*tmpword[i], tr.first, ""_u, ""_u, tr.second);
    }
    else if(preBilingual) {
      UString sl;
      UString tl;
      UString ref;
      UString wblank;

      int seenSlash = 0;
      bool inTag = false;
      for(UString::const_iterator it = tmpword[i]->begin(); it != tmpword[i]->end(); it++)
      {
        if(*it == '\\')
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
        else if(*it == '[')
        {
          if(*(it+1) == '[') //wordbound blank
          {
            while(true) {
              if(*it == '\\') {
                wblank.push_back(*it);
                it++;
                wblank.push_back(*it);
              }
              else if(*it == '^' && *(it-1) == ']' && *(it-2) == ']') {
                break;
              }
              else {
                wblank.push_back(*it);
              }

              it++;
            }
          }
          else {
            if(seenSlash == 0)      sl.push_back(*it);
            else if(seenSlash == 1) tl.push_back(*it);
            else                   ref.push_back(*it);
          }
          continue;
        }

        if(*it == '/' && !inTag) {
          seenSlash++;

          ref.clear(); //word after last slash is ref
          continue;
        }
        // slashes in tags are accepted elsewhere in the pipe,
        // so don't split readings on them here
        else if (*it == '<') {
          inTag = true;
        }
        else if (*it == '>') {
          inTag = false;
        }
        if(seenSlash == 0)      sl.push_back(*it);
        else if(seenSlash == 1) tl.push_back(*it);
        else                   ref.push_back(*it);
      }
      word[i] = new TransferWord(sl, tl, ref, wblank, false);
    }
    else // neither useBilingual nor preBilingual (sl==tl)
    {
      word[i] = new TransferWord(*tmpword[i], *tmpword[i], ""_u, ""_u, false);
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
  word = NULL;
  tmpword.clear();
  tmpblank.clear();
  ms.init(me->getInitial());
  return words_to_consume;
}

/* HERE */
void
Transfer::applyWord(UString const &word_str)
{
  ms.step('^');

  for(unsigned int i = 0, limit = word_str.size(); i < limit; i++)
  {
    switch(word_str[i])
    {
      case '\\':
        i++;
        ms.step(u_tolower(word_str[i]), any_char);
        break;

      case '[':
        if(word_str[i+1] == '[')
        {
          while(true)
          {
            if(word_str[i] == '\\')
            {
              i++;
            }
            else if(i >= 4)
            {
              if(word_str[i] == '^' && word_str[i-1] == ']' && word_str[i-2] == ']')
              {
                break;
              }
            }

            i++;
          }
        }
        else
        {
          ms.step(u_tolower(word_str[i]), any_char);
        }
        break;

      case '/':
        i = limit;
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

      default:
	ms.step(u_tolower(word_str[i]), any_char);
	break;
    }
  }
  ms.step('$');
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
