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

#ifndef APERTIUM_TAGGER_H
#define APERTIUM_TAGGER_H

#include "apertium_config.h"

#include "basic_stream_tagger.h"
#include "basic_stream_tagger_trainer.h"
#include "basic_tagger.h"
#include "constructor_eq_delete.h"
#include "file_tagger.h"
#include "optional.h"

#include "getopt_long.h"
#include <string>

namespace Apertium {
class apertium_tagger : private constructor_eq_delete {
public:
  apertium_tagger(int &argc, char **&argv);

private:
  enum FunctionTypeType { Unigram, SlidingWindow, Perceptron };
  enum UnigramType { Stream_5_3_1, Stream_5_3_2, Stream_5_3_3 };
  enum FunctionType { Tagger, Retrain, Supervised, Train };
  static void help();
  static const struct option longopts[];

  static std::string option_string(const int &indexptr_);
  static std::string option_string(const struct option &option_);
  static void locale_global_();
  void set_indexptr();
  void flagOptionCase(bool (basic_Tagger::Flags::*GetFlag)() const,
                      void (basic_Tagger::Flags::*SetFlag)(const bool &));
  std::string option_string();
  void functionTypeTypeOptionCase(const FunctionTypeType &FunctionTypeType_);
  void functionTypeOptionCase(const FunctionType &FunctionType_);
  void getCgAugmentedModeArgument();
  void getIterationsArgument();
  unsigned long optarg_unsigned_long(const char *metavar);
  void get_file_arguments(
    bool get_crp_fn,
    char **DicFn, char **CrpFn,
    char **TaggedFn, char **UntaggedFn,
    char **TsxFn, char **ProbFn);
  void init_FILE_Tagger(FILE_Tagger &FILE_Tagger_, string const &TsxFn);

  MorphoStream* setup_untagged_morpho_stream(
    FILE_Tagger &FILE_Tagger_,
    char *DicFn, char *UntaggedFn,
    FILE **Dictionary, FILE **UntaggedCorpus);
  void close_untagged_files(
    char *DicFn, char *UntaggedFn,
    FILE *Dictionary, FILE *UntaggedCorpus);

  void g_StreamTagger(StreamTagger &StreamTagger_);
  void s_StreamTaggerTrainer(StreamTaggerTrainer &StreamTaggerTrainer_);
  void g_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void r_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void s_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void t_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void c_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  int &argc;
  char **&argv;
  int The_val;
  int nonoptarg;


  int The_indexptr;
  Optional<int> FunctionTypeTypeOption_indexptr;
  Optional<int> FunctionTypeOption_indexptr;


  Optional<FunctionTypeType> TheFunctionTypeType;
  Optional<UnigramType> TheUnigramType;
  Optional<FunctionType> TheFunctionType;
  unsigned long TheFunctionTypeOptionArgument;
  unsigned long CgAugmentedMode;
  basic_Tagger::Flags TheFlags;
};
}

#endif // APERTIUM_TAGGER_H
