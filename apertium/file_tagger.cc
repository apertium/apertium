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
// along with this program; if not, see <https://www.gnu.org/licenses/>.

#include "file_tagger.h"

#include <apertium/tsx_reader.h>
#include <apertium/tagger_utils.h>
#include <apertium/file_morpho_stream.h>
#include <apertium/utils.h>

#include <algorithm>
#include <cstdio>

namespace Apertium {
FILE_Tagger::FILE_Tagger() : TheFlags() {}

FILE_Tagger::FILE_Tagger(TaggerFlags& Flags_) : TheFlags(Flags_) {}

FILE_Tagger::~FILE_Tagger() {}

void FILE_Tagger::set_debug(const bool &Debug) { TheFlags.setDebug(Debug); }

void FILE_Tagger::set_show_sf(const bool &ShowSuperficial) {
  TheFlags.setShowSuperficial(ShowSuperficial);
}

void FILE_Tagger::setNullFlush(const bool &NullFlush) {
  TheFlags.setNullFlush(NullFlush);
}

void FILE_Tagger::tagger(FILE *Input, FILE *Output) {
  FileMorphoStream morpho_stream(Input, TheFlags.getDebug(), &get_tagger_data());

  tagger(morpho_stream, Output);
}

void FILE_Tagger::init_and_train(MorphoStream &lexmorfo, unsigned long count) {
  init_probabilities_kupiec_(lexmorfo);
  train(lexmorfo, count);
}

void FILE_Tagger::init_and_train(FILE *corpus, unsigned long count) {
  init_probabilities_kupiec_(corpus);
  train(corpus, count);
}

void FILE_Tagger::train(FILE *corpus, unsigned long count) {
  FileMorphoStream lexmorfo(corpus, true, &get_tagger_data());
  train(lexmorfo, count);
}

void FILE_Tagger::deserialise(string const &TaggerSpecificationFilename) {
  TSXReader TaggerSpecificationReader_;
  TaggerSpecificationReader_.read(TaggerSpecificationFilename);
  deserialise(TaggerSpecificationReader_.getTaggerData());
}

void FILE_Tagger::init_probabilities_from_tagged_text_(FILE *TaggedCorpus,
                                                       FILE *Corpus) {
  FileMorphoStream stream_tagged(TaggedCorpus, true, &get_tagger_data());
  FileMorphoStream stream_untagged(Corpus, true, &get_tagger_data());
  init_probabilities_from_tagged_text_(stream_tagged, stream_untagged);
}

void FILE_Tagger::init_probabilities_kupiec_(FILE *Corpus) {
  FileMorphoStream lexmorfo(Corpus, true, &get_tagger_data());
  init_probabilities_kupiec_(lexmorfo);
}

void FILE_Tagger::read_dictionary(FILE *fdic) {
  tagger_utils::scan_for_ambg_classes(fdic, get_tagger_data());
  tagger_utils::add_neccesary_ambg_classes(get_tagger_data());
  post_ambg_class_scan();
}

}
