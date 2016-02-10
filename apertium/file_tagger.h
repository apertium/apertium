// Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.

#ifndef FILE_TAGGER_H
#define FILE_TAGGER_H

#include <apertium/tagger_data.h>

#include <cstdio>
#include <string>
#include <vector>

namespace Apertium {
class FILE_Tagger {
public:
  FILE_Tagger();
  virtual ~FILE_Tagger();
  virtual void deserialise(FILE *Serialised_FILE_Tagger) = 0;
  void set_debug(const bool &Debug);
  void set_show_sf(const bool &ShowSuperficial);
  void setNullFlush(const bool &NullFlush);
  virtual void tagger(FILE *Input, FILE *Output, const bool &First = false) = 0;
  virtual std::vector<std::wstring> &getArrayTags() = 0;
  virtual void train(FILE *Corpus, unsigned long Count) = 0;
  virtual void serialise(FILE *Stream_) = 0;
  void deserialise(char *const TaggerSpecificationFilename);
  virtual void read_dictionary(FILE *Dictionary) = 0;
  virtual void init_probabilities_from_tagged_text_(FILE *TaggedCorpus,
                                                    FILE *Corpus) = 0;
  virtual void init_probabilities_kupiec_(FILE *Corpus) = 0;
  virtual void train_(FILE *Corpus, unsigned long Count) = 0;

protected:
  virtual void deserialise(const TaggerData &Deserialised_FILE_Tagger) = 0;
  bool debug;
  bool show_sf;
  bool null_flush;
};
}

#endif // FILE_TAGGER_H
