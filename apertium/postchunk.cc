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

#include <lttoolbox/xml_walk_util.h>
#include <lttoolbox/string_utils.h>

#include <iostream>

using namespace std;

Postchunk::Postchunk()
  : word(0), in_wblank(false), inword(false)
{}

bool
Postchunk::checkIndex(xmlNode *element, int index, int limit)
{
  if(index > limit) // Note: Unlike transfer/interchunk, we allow index==limit!
  {
    cerr << "Error in " << (char *) doc->URL << ": line " << element->line << ": index > limit" << endl;
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
Postchunk::evalCachedString(xmlNode* element)
{
  TransferInstr& ti = evalStringCache[element];
  switch (ti.getType()) {
  case ti_clip_tl:
    if (checkIndex(element, ti.getPos(), lword)) {
      if (gettingLemmaFromWord(ti.getContent()) && lword > 1) {
        if (in_lu) {
          out_wblank = combineWblanks(out_wblank, word[ti.getPos()]->getWblank());
        } else if (in_let_var) {
          var_out_wblank[var_val] = combineWblanks(var_out_wblank[var_val],
                                                   word[ti.getPos()]->getWblank());
        }
      }
      return word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
    }
    break;

  case ti_lu_count:
    return StringUtils::itoa(tmpword.size());

  case ti_var:
    if (lword > 1) {
      out_wblank = combineWblanks(out_wblank, var_out_wblank[ti.getContent()]);
    }
    return variables[ti.getContent()];

  case ti_lit_tag:
  case ti_lit:
    return ti.getContent();

  case ti_b:
    if (!blank_queue.empty()) {
      UString retblank = blank_queue.front();
      if (in_out) {
        blank_queue.pop();
      }
      return retblank;
    } else {
      return " "_u;
    }
    break;

  case ti_get_case_from:
    if (checkIndex(element, ti.getPos(), lword)) {
      return StringUtils::copycase(word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]),
                                   evalString((xmlNode*) ti.getPointer()));
    }
    break;

  case ti_case_of_tl:
    if (checkIndex(element, ti.getPos(), lword)) {
      return StringUtils::getcase(word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]));
    }
    break;

  default:
    return ""_u;
  }
  return ""_u;
}

void
Postchunk::processClip(xmlNode* element)
{
  int pos = 0;
  UString part;
  for(xmlAttr* i = element->properties; i != NULL; i = i->next) {
    if (!xmlStrcmp(i->name, (const xmlChar*) "part")) {
      part = to_ustring((const char*) i->children->content);
    } else if (!xmlStrcmp(i->name, (const xmlChar*) "pos")) {
      pos = atoi((const char *)i->children->content);
    }
  }
  evalStringCache[element] = TransferInstr(ti_clip_tl, part, pos, NULL);
}

void
Postchunk::processBlank(xmlNode* element)
{
  if (element->properties == NULL) {
    evalStringCache[element] = TransferInstr(ti_b, " "_u, -1);
  } else {
    int pos = atoi((const char *) element->properties->children->content) - 1;
    evalStringCache[element] = TransferInstr(ti_b, ""_u, pos);
  }
}

void
Postchunk::processLuCount(xmlNode* element)
{
  evalStringCache[element] = TransferInstr(ti_lu_count, ""_u, 0);
}

void
Postchunk::processCaseOf(xmlNode* element)
{
  int pos = 0;
  UString part;
  for (xmlAttr* i = element->properties; i != NULL; i = i->next) {
    if (!xmlStrcmp(i->name, (const xmlChar*) "part")) {
      part = to_ustring((const char*) i->children->content);
    } else if(!xmlStrcmp(i->name, (const xmlChar*) "pos")) {
      pos = atoi((const char *) i->children->content);
    }
  }
  evalStringCache[element] = TransferInstr(ti_case_of_tl, part, pos);
}

UString
Postchunk::processLu(xmlNode* element)
{
  in_lu = true;
  out_wblank.clear();

  UString myword;
  for (auto i : children(element)) {
    myword.append(evalString(i));
  }
  in_lu = false;

  if (lword == 1) {
    out_wblank = word[1]->getWblank();
  }

  if (myword.empty()) {
    return ""_u;
  } else {
    return out_wblank+"^"_u+myword+"$"_u;
  }
}

UString
Postchunk::processMlu(xmlNode* element)
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
      if(!myword.empty() && myword[0] != '#') {  //'+#' problem
        value += '+';
      }
    } else {
      if (!myword.empty()) {
        first_time = false;
      }
    }

	value.append(myword);
  }

  in_lu = false;

  if (lword == 1) {
    out_wblank = word[1]->getWblank();
  }

  if (value.empty()) {
    return ""_u;
  } else {
    return out_wblank+"^"_u+value+"$"_u;
  }
}

UString
Postchunk::processChunk(xmlNode* element)
{
  cerr << "Error: unexpected expression: '" << element->name << "'" << endl;
  exit(EXIT_FAILURE);
  return ""_u; // make the type checker happy
}

void
Postchunk::processOut(xmlNode *localroot)
{
  in_out = true;

  for (auto i : children(localroot)) {
    if(!xmlStrcmp(i->name, (const xmlChar *) "lu")) {
      write(processLu(i), output);
    } else if(!xmlStrcmp(i->name, (const xmlChar *) "mlu")) {
      write(processMlu(i), output);
    } else { // 'b'
      write(evalString(i), output);
    }
  }

  in_out = false;
}

void
Postchunk::processTags(xmlNode *localroot)
{
  for (auto i : children(localroot)) {
    if(!xmlStrcmp(i->name, (xmlChar const *) "tag")) {
      for (auto j : children(i)) {
        write(evalString(j), output);
      }
    }
  }
}

void
Postchunk::processLet(xmlNode *localroot)
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
    UString part;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((const char*)i->children->content);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
	pos = atoi((const char *) i->children->content);
      }
    }


    bool match = word[pos]->setChunkPart(attr_items[part],
					 evalString(rightSide));
    if(!match && trace)
    {
      cerr << "apertium-postchunk warning: <let> on line " << localroot->line << " sometimes discards its value." << endl;
    }
    evalStringCache[leftSide] = TransferInstr(ti_clip_tl, part, pos, NULL);
  }
}

void
Postchunk::processModifyCase(xmlNode *localroot)
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

  if(!xmlStrcmp(leftSide->name, (const xmlChar *) "clip"))
  {
    int pos = 0;
    UString part;

    for(xmlAttr *i = leftSide->properties; i != NULL; i = i->next)
    {
      if(!xmlStrcmp(i->name, (const xmlChar *) "part"))
      {
        part = to_ustring((const char*)i->children->content);
      }
      else if(!xmlStrcmp(i->name, (const xmlChar *) "pos"))
      {
        pos = atoi((const char *) i->children->content);
      }
    }

    UString const result = StringUtils::copycase(evalString(rightSide),
				   word[pos]->chunkPart(attr_items[part]));
    bool match = word[pos]->setChunkPart(attr_items[part], result);

    if(!match && trace)
    {
      cerr << "apertium-postchunk warning: <modify-case> on line " << localroot->line << " sometimes discards its value." << endl;
    }
  }
  else if(!xmlStrcmp(leftSide->name, (const xmlChar *) "var"))
  {
    UString const val = to_ustring((const char *) leftSide->properties->children->content);
    variables[val] = StringUtils::copycase(evalString(rightSide), variables[val]);
  }
}

void
Postchunk::processCallMacro(xmlNode *localroot)
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
  for (auto i : children(localroot)) {
    int pos = atoi((const char *) i->properties->children->content);
    if(!checkIndex(localroot, pos, lword)) {
      indexesOK = false;      // avoid segfaulting on empty chunks, e.g. ^x<x>{}$
      pos = 1;
    }
    myword[idx] = word[pos];
    idx++;
  }

  swap(myword, word);
  swap(npar, lword);

  if(indexesOK) {
    for (auto i : children(macro)) {
      processInstruction(i);
    }
  }
  else {
    cerr << "Warning: Not calling macro \"" << n << "\" from line " << localroot->line << " (empty word?)" << endl;
  }

  swap(myword, word);
  swap(npar, lword);

  delete[] myword;
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
    UChar32 val = in.get();
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
    else if(inword && val == '{')
    {
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
      content += wchar_t(val);
    }
  }
}

void
Postchunk::postchunk_wrapper_null_flush(InputFile& in, UFILE* out)
{
  null_flush = false;
  internal_null_flush = true;

  while(!in.eof())
  {
    postchunk(in, out);
    u_fputc('\0', out);
    u_fflush(out);
    variables = variable_defaults;
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
  unsigned int prev_last = last;
  int lastrule_id = -1;
  set<int> banned_rules;

  output = out;
  ms.init(me->getInitial());

  while(true)
  {
    if(ms.size() == 0)
    {
      if(lastrule != NULL)
      {
        int words_to_consume = applyRule();
        if (words_to_consume == -1) {
          banned_rules.clear();
          input_buffer.setPos(last);
        } else if (words_to_consume == 1) {
          banned_rules.clear();
          if (prev_last >= input_buffer.getSize()) {
            input_buffer.setPos(0);
          } else {
            input_buffer.setPos(prev_last+1);
          }
          while (true) {
            TransferToken& tt = input_buffer.next();
            if (tt.getType() == tt_word) {
              break;
            }
          }
        } else {
          banned_rules.insert(lastrule_id);
          input_buffer.setPos(prev_last);
          input_buffer.next();
          last = input_buffer.getPos();
        }
        lastrule_id = -1;
      }
      else
      {
        if(tmpword.size() != 0) {
          unchunk(*tmpword[0], output);
          tmpword.clear();
          input_buffer.setPos(last);
          input_buffer.next();
          prev_last = last;
          banned_rules.clear();
          last = input_buffer.getPos();
          ms.init(me->getInitial());
        }
        else if(tmpblank.size() != 0) {
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
      last = input_buffer.getPos();
      lastrule_id = val;

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
      if(tmpword.size() != 0) {
        tmpblank.push_back(&current.getContent());
        ms.clear();
      }
      else {
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
Postchunk::applyRule()
{
  UString const chunk = *tmpword[0];
  tmpword.clear();
  splitWordsAndBlanks(chunk, tmpword, tmpblank);

  word = new InterchunkWord *[tmpword.size()+1];
  lword = tmpword.size();
  word[0] = new InterchunkWord(wordzero(chunk));

  for(unsigned int i = 1, limit = tmpword.size()+1; i != limit; i++)
  {
    if(i != 1) {
      blank_queue.push(*tmpblank[i-1]);
    }

    word[i] = new InterchunkWord(*tmpword[i-1]);
  }

  int words_to_consume = processRule(lastrule);
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
  return words_to_consume;
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
      mytag += '>';
      vectags.push_back(mytag);
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

  return ""_u;
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

  return ""_u;
}

void
Postchunk::unchunk(UString const &chunk, UFILE* output)
{
  vector<UString> vectags = getVecTags(chunk);
  UString case_info = StringUtils::getcase(pseudolemma(chunk));
  bool uppercase_all = false;
  bool uppercase_first = false;

  if(case_info == "AA"_u)
  {
    uppercase_all = true;
  }
  else if(case_info == "Aa"_u)
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
          if(u_isdigit(chunk[i+1]))
          {
            int j = ++i;
            while (chunk[++i] != '>');
            unsigned long value = StringUtils::stoi(chunk.substr(j, i-j)) - 1;
            if(vectags.size() > value)
            {
              write(vectags[value], output);
            }
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
            // TODO
            u_fputc(u_toupper(chunk[i]), output);
          }
          else if(uppercase_first)
          {
            if(u_isalnum(chunk[i])) {
              // TODO
              u_fputc(u_toupper(chunk[i]), output);
              uppercase_first = false;
            } else {
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
  UString case_info = StringUtils::getcase(pseudolemma(chunk));
  bool uppercase_all = false;
  bool uppercase_first = false;
  bool lastblank = true;

  if(case_info == "AA"_u)
  {
    uppercase_all = true;
  }
  else if(case_info == "Aa"_u)
  {
    uppercase_first = true;
  }

  for(int i = beginChunk(chunk), limit = endChunk(chunk); i < limit; i++)
  {
    if(chunk[i] == '^')
    {
      if(!lastblank)
      {
        blanks.push_back(new UString(""_u));
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
            unsigned long value = StringUtils::stoi(chunk.c_str()+i+1) - 1;
            // TODO
            //unsigned long value = wcstoul(chunk.c_str()+i+1,
            //                              NULL, 0) - 1;
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
            // TODO
            ref += u_toupper(chunk[i]);
          }
          else if(uppercase_first)
          {
            if(iswalnum(chunk[i]))
            {
              // TODO
              ref += u_toupper(chunk[i]);
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
          blanks.push_back(new UString(""_u));
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
            i++; //i->"^"_u
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
              unsigned long value = StringUtils::stoi(chunk.c_str()+i+1) - 1;
              //unsigned long value = wcstoul(chunk.c_str()+i+1,
              //                              NULL, 0) - 1;
              // TODO: make sure this is equivalent
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
              // TODO
              ref += u_toupper(chunk[i]);
            }
            else if(uppercase_first)
            {
              if(u_isalnum(chunk[i])) // TODO
              {
                ref += u_toupper(chunk[i]); // TODO
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
        UString *myblank = new UString(""_u);
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
