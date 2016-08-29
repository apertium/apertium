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

#include "file_tagger.h"

#include <apertium/tsx_reader.h>
#include <apertium/tagger_utils.h>
#include <apertium/file_morpho_stream.h>
#include <apertium/utils.h>

#include <algorithm>
#include <cstdio>

namespace Apertium {
FILE_Tagger::FILE_Tagger() : debug(false), show_sf(false), null_flush(false),
                             allow_similar_ambg_class(false) {}

FILE_Tagger::~FILE_Tagger() {}

void FILE_Tagger::set_debug(const bool &Debug) { debug = Debug; }

void FILE_Tagger::set_show_sf(const bool &ShowSuperficial) {
  show_sf = ShowSuperficial;
}

void FILE_Tagger::setNullFlush(const bool &NullFlush) {
  null_flush = NullFlush;
}

void FILE_Tagger::set_allow_similar_ambg_class(const bool &asac) {
  allow_similar_ambg_class = asac;
}

void FILE_Tagger::tagger(FILE *Input, FILE *Output, const bool &First) {
  FileMorphoStream morpho_stream(Input, debug, &get_tagger_data());

  tagger(morpho_stream, Output, First);
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

void FILE_Tagger::scan_and_count_ambg_classes(FILE *crp,
                                              Collection &all_ambg_classes,
                                              vector<unsigned int> &all_acc) {
  TaggerData &td = get_tagger_data();

  // Get all ambiguity classes
  FileMorphoStream morpho_stream(crp, true, &td);
  tagger_utils::scan_for_ambg_classes(all_ambg_classes, morpho_stream);
  morpho_stream.rewind();

  // Count all of their frequencies
  all_acc.assign(all_ambg_classes.size(), 0);
  tagger_utils::count_ambg_class_freq(all_ambg_classes, all_acc, morpho_stream);
}

void FILE_Tagger::use_pop_ambg_class(Collection &all_ambg_classes,
                                     vector<unsigned int> &all_acc,
                                     unsigned int n) {
  // Sort and insert most popular into TaggerData
  TaggerData &td = get_tagger_data();
  Collection &output = td.getOutput();
  vector<unsigned int> &acc = td.getAmbgClassCounts();
  vector<unsigned int> indices(all_ambg_classes.size(), 0);
  for (int i = 0; i < all_ambg_classes.size(); i++) {
    indices[i] = i;
  }
  SortByComparer<unsigned int, int> sbc(all_acc);
  std::sort(indices.begin(), indices.end(), sbc);
  acc.assign(n, 0);
  for (size_t i = 0, num_ambg_classes = 0; num_ambg_classes < n; i++) {
    if (i >= indices.size()) {
      int avail_ambg_casses = (all_ambg_classes.size()
                               - (td.getTagIndex()).size() - 1);
      wcerr << "You asked for " << n
            << " ambiguity classes, but there are only " << avail_ambg_casses
            << ". Continuing with " << avail_ambg_casses << ".";
      break;
    }
    int all_ambg_classes_idx = indices[i];
    const set<TTag> &ambg_class = all_ambg_classes[all_ambg_classes_idx];
    if (ambg_class == td.getOpenClass() || ambg_class.size() == 1) {
      continue;
    }
    output[all_ambg_classes[all_ambg_classes_idx]];
    acc[num_ambg_classes] = all_acc[all_ambg_classes_idx];
    num_ambg_classes++;
  }
}

void FILE_Tagger::read_dictionary_and_counts_poptrim(FILE *crp, unsigned int n) {
  Collection all_ambg_classes;
  vector<unsigned int> all_acc;
  scan_and_count_ambg_classes(crp, all_ambg_classes, all_acc);
  use_pop_ambg_class(all_ambg_classes, all_acc, n);
  tagger_utils::add_neccesary_ambg_classes(get_tagger_data());
  post_ambg_class_scan();
}

void FILE_Tagger::init_ambg_class_freq() {
  tagger_utils::init_ambg_class_freq(get_tagger_data());
}

void FILE_Tagger::count_ambg_class_freq(FILE *crp) {
  tagger_utils::count_ambg_class_freq(crp, get_tagger_data());
}

void FILE_Tagger::read_dictionary_and_counts(FILE *crp) {
  // could be sped up by doing a single scan
  read_dictionary(crp);
  fseek(crp, 0, SEEK_SET);
  init_ambg_class_freq();
  tagger_utils::count_ambg_class_freq(crp, get_tagger_data());
}

}
