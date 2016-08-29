#include <apertium/tagger_word.h>
#include <apertium/morpho_stream.h>
#include <apertium/dual_morpho_stream.h>
#include "exception.h"

using namespace std;

DualMorphoStream::DualMorphoStream(FILE *fcg, FILE *funtagged, bool d, TaggerData *t)
  : cg_stream(fcg, d, t), untagged_stream(funtagged, d, t){}

TaggerWord* DualMorphoStream::get_next_word() {
  TaggerWord* cg_word = cg_stream.get_next_word();
  TaggerWord* untagged_word = untagged_stream.get_next_word();
  if (!cg_word || !untagged_word) {
    if (cg_word || untagged_word) {
      std::stringstream what_;
      what_ << "One stream has ended prematurely. "
            << "Please check if they are aligned.\n";
      throw Apertium::Exception::UnalignedStreams(what_);
    }
    return NULL;
  }
  if (cg_word->get_tags().size() == 1) {
    delete untagged_word;
    return cg_word;
  } else {
    delete cg_word;
    return untagged_word;
  }
}

void DualMorphoStream::setNullFlush(bool nf) {
  cg_stream.setNullFlush(nf);
  untagged_stream.setNullFlush(nf);
}

bool DualMorphoStream::getEndOfFile() {
  return cg_stream.getEndOfFile() && untagged_stream.getEndOfFile();
}

void DualMorphoStream::setEndOfFile(bool eof) {
  cg_stream.setEndOfFile(eof);
  untagged_stream.setEndOfFile(eof);
}

void DualMorphoStream::rewind() {
  cg_stream.rewind();
  untagged_stream.rewind();
}
