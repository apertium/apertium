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

#include <apertium/xml_walk_util.h>
#include <apertium/string_utils.h>

#include <iostream>

using namespace Apertium;
using namespace std;

Interchunk::Interchunk()
  : word(0), last_lword(0), inword(false)
{}

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
Interchunk::evalCachedString(xmlNode* element)
{
  TransferInstr& ti = evalStringCache[element];
  switch (ti.getType()) {
  case ti_clip_tl:
    if (checkIndex(element, ti.getPos(), lword)) {
      if (ti.getContent() == "content"_u) {
        UString wf = word[ti.getPos()]->chunkPart(attr_items[ti.getContent()]);
        return wf.substr(1, wf.length()-2); // trim { and }
      } else {
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
Interchunk::processClip(xmlNode* element)
{
  int pos = 0;
  UString part;
  for (xmlAttr* i = element->properties; i != NULL; i = i->next) {
    if (!xmlStrcmp(i->name, (const xmlChar*) "part")) {
      part = to_ustring((const char*) i->children->content);
    } else if (!xmlStrcmp(i->name, (const xmlChar*) "pos")) {
      pos = atoi((const char*) i->children->content) - 1;
    }
  }
  evalStringCache[element] = TransferInstr(ti_clip_tl, part, pos, NULL);
}

void
Interchunk::processBlank(xmlNode* element)
{
  if (element->properties == NULL) {
    evalStringCache[element] = TransferInstr(ti_b, " "_u, -1);
  } else {
    int pos = atoi((const char*) element->properties->children->content) - 1;
    evalStringCache[element] = TransferInstr(ti_b, ""_u, pos);
  }
}

void
Interchunk::processLuCount(xmlNode* element)
{
  cerr << "Error: unexpected expression: '" << element->name << "'" << endl;
  exit(EXIT_FAILURE);
}

UString
Interchunk::processLu(xmlNode* element)
{
  cerr << "Error: unexpected expression: '" << element->name << "'" << endl;
  exit(EXIT_FAILURE);
  return ""_u; // make the type checker happy
}

UString
Interchunk::processMlu(xmlNode* element)
{
  cerr << "Error: unexpected expression: '" << element->name << "'" << endl;
  exit(EXIT_FAILURE);
  return ""_u; // make the type checker happy
}

void
Interchunk::processCaseOf(xmlNode* element)
{
  int pos = 0;
  UString part;
  for (xmlAttr* i = element->properties; i != NULL; i = i->next) {
    if (!xmlStrcmp(i->name, (const xmlChar*) "part")) {
      part = to_ustring((char*) i->children->content);
    } else if (!xmlStrcmp(i->name, (const xmlChar*) "pos")) {
      pos = atoi((const char*) i->children->content) - 1;
    }
  }
  evalStringCache[element] = TransferInstr(ti_case_of_tl, part, pos);
}

void
Interchunk::processOut(xmlNode *localroot)
{
  in_out = true;

  for (auto i : children(localroot)) {
    if(!xmlStrcmp(i->name, (const xmlChar *) "chunk")) {
      write(processChunk(i), output);
    } else { // 'b'
      write(evalString(i), output);
    }
  }

  in_out = false;
}

UString
Interchunk::processChunk(xmlNode *localroot)
{
  UString result;
  result.append("^"_u);

  for (auto i : children(localroot)) {
    result.append(evalString(i));
  }

  result.append("$"_u);
  return result;
}

void
Interchunk::processLet(xmlNode *localroot)
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
Interchunk::processModifyCase(xmlNode *localroot)
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

    UString const result = StringUtils::copycase(evalString(rightSide),
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
    variables[val] = StringUtils::copycase(evalString(rightSide), variables[val]);
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
  int idx = 0;
  if(npar > 0)
  {
    myword = new InterchunkWord *[npar];
    for (auto i : children(localroot)) {
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
        if(tmpword.size() != 0) {
          u_fprintf(output, "^%S$", tmpword[0]->c_str());
          tmpword.clear();
          input_buffer.setPos(last);
          input_buffer.next();
          last = input_buffer.getPos();
          ms.init(me->getInitial());
        }
        else if(tmpblank.size() != 0) {
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
