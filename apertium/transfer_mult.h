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
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
#ifndef _TRANSFER_MULT_
#define _TRANSFER_MULT_

#include <apertium/transfer_instr.h>
#include <apertium/transfer_token.h>
#include <apertium/transfer_word.h>
#include <lttoolbox/alphabet.h>
#include <lttoolbox/buffer.h>
#include <lttoolbox/fst_processor.h>
#include <lttoolbox/match_exe.h>
#include <lttoolbox/match_state.h>

#include <cstdio>
#include <map>
#include <set>
#include <vector>

using namespace std;

class TransferMult
{
private:
  
  Alphabet alphabet;
  MatchExe *me;
  MatchState ms;
  map<string, ApertiumRE, Ltstr> attr_items;
  map<string, string, Ltstr> variables;
  map<string, int, Ltstr> macros;
  map<string, set<string, Ltstr>, Ltstr> lists;
  map<string, set<string, Ltstr>, Ltstr> listslow;
  TransferWord **word;
  string **blank;
  Buffer<TransferToken> input_buffer;
  vector<wstring *> tmpword;
  vector<wstring *> tmpblank;
  wstring output_string;  

  FSTProcessor fstp;
  FILE *output;
  int any_char;
  int any_tag;
  bool isRule;
  unsigned int numwords;
  
  unsigned int nwords;
  
  enum OutputType{lu,chunk};
  
  OutputType defaultAttrs;
  
  void destroy();
  void readData(FILE *input);
  void readBil(string const &filename);
  string caseOf(string const &str);
  string copycase(string const &source_word, string const &target_word);

  bool beginsWith(string const &str1, string const &str2) const;
  bool endsWith(string const &str1, string const &str2) const;
  string tolower(string const &str) const;
  string tags(string const &str) const;
  wstring readWord(FILE *in);
  wstring readBlank(FILE *in);
  wstring readUntil(FILE *in, int const symbol) const;
  void applyWord(wstring const &word_str);
  void applyRule();
  TransferToken & readToken(FILE *in);
  void writeMultiple(list<vector<wstring> >::iterator itwords,
                     list<wstring>::iterator itblanks, 
                     list<vector<wstring> >::const_iterator limitwords, 
                     wstring acum = L"", bool multiple = false);
  vector<wstring> acceptions(wstring str);
  bool isDefaultWord(wstring const &str);
public:
  TransferMult();
  ~TransferMult();
  
  void read(string const &datafile, string const &fstfile);
  void transfer(FILE *in, FILE *out);
};

#endif
