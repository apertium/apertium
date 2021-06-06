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
 * along with this program; if not, see <https://www.gnu.org/licenses/>.
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
  map<string, ApertiumRE> attr_items;
  map<string, string> variables;
  map<string, int> macros;
  map<string, set<string>> lists;
  map<string, set<string>> listslow;
  TransferWord **word;
  string **blank;
  Buffer<TransferToken> input_buffer;
  vector<UString *> tmpword;
  vector<UString *> tmpblank;
  UString output_string;

  FSTProcessor fstp;
  UFILE* output;
  int any_char;
  int any_tag;
  bool isRule;
  unsigned int numwords;

  unsigned int nwords;

  enum OutputType{lu,chunk};

  OutputType defaultAttrs;

  void destroy();
  void readData(InputFile& input);
  void readBil(string const &filename);
  string caseOf(string const &str);
  string copycase(string const &source_word, string const &target_word);

  bool beginsWith(string const &str1, string const &str2) const;
  bool endsWith(string const &str1, string const &str2) const;
  string tolower(string const &str) const;
  string tags(string const &str) const;
  UString readWord(InputFile& in);
  UString readBlank(InputFile& in);
  UString readUntil(InputFile& in, int const symbol) const;
  void applyWord(UString const &word_str);
  void applyRule();
  TransferToken & readToken(InputFile& in);
  void writeMultiple(list<vector<UString> >::iterator itwords,
                     list<UString>::iterator itblanks,
                     list<vector<UString> >::const_iterator limitwords,
                     UString acum = "", bool multiple = false);
  vector<UString> acceptions(UString str);
  bool isDefaultWord(UString const &str);
public:
  TransferMult();
  ~TransferMult();

  void read(string const &datafile, string const &fstfile);
  void transfer(InputFile& in, UFILE* out);
};

#endif
