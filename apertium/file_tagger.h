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

#include <apertium/tagger_flags.h>
#include <apertium/tagger_data.h>
#include <apertium/morpho_stream.h>

#include <cstdio>
#include <string>
#include <vector>

namespace Apertium {
class FILE_Tagger {
public:
  FILE_Tagger();
  FILE_Tagger(TaggerFlags& Flags_);
  virtual ~FILE_Tagger();
  virtual void deserialise(FILE *Serialised_FILE_Tagger) = 0;
  void set_debug(const bool &Debug);
  void set_show_sf(const bool &ShowSuperficial);
  void setNullFlush(const bool &NullFlush);
  virtual void tagger(FILE *Input, FILE *Output);
  virtual void tagger(MorphoStream &morpho_stream, FILE *Output) = 0;
  virtual std::vector<std::wstring> &getArrayTags() = 0;
  void init_and_train(MorphoStream &lexmorfo, unsigned long Count);
  void init_and_train(FILE *Corpus, unsigned long Count);
  virtual void train(FILE *Corpus, unsigned long Count);
  virtual void train(MorphoStream &lexmorpho, unsigned long count) = 0;
  virtual void train(MorphoStream &lexmorpho) = 0;
  virtual void serialise(FILE *Stream_) = 0;
  void deserialise(string const &TaggerSpecificationFilename);
  virtual void init_probabilities_from_tagged_text_(
      FILE *TaggedCorpus, FILE *Corpus);
  virtual void init_probabilities_from_tagged_text_(
      MorphoStream &stream_tagged,
      MorphoStream &stream_untagged) = 0;
  virtual void init_probabilities_kupiec_(FILE *Corpus);
  virtual void init_probabilities_kupiec_(MorphoStream &lexmorfo) = 0;

  /** It reads the expanded dictionary received as a parameter and calculates
   *  the set of ambiguity classes that the tagger will manage.
   *  @param is the input stream with the expanded dictionary to read
   */
  void read_dictionary(FILE *is);

  virtual TaggerData& get_tagger_data() = 0;

protected:
  virtual void deserialise(const TaggerData &Deserialised_FILE_Tagger) = 0;
  virtual void post_ambg_class_scan() = 0;
  TaggerFlags TheFlags;
};
}

#endif // FILE_TAGGER_H
