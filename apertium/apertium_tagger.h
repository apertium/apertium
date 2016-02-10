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

#include <getopt.h>
#include <string>

namespace Apertium {
class apertium_tagger : private constructor_eq_delete {
public:
  apertium_tagger(int &argc, char **&argv);

private:
  enum FunctionTypeType { Unigram, SlidingWindow };
  enum UnigramType { Stream_5_3_1, Stream_5_3_2, Stream_5_3_3 };
  enum FunctionType { Tagger, Retrain, Supervised, Train };
  static void help();

#if HAVE_GETOPT_LONG

  static std::string option_string(const int &indexptr_);
  static std::string option_string(const struct option &option_);

#else

  static std::string option_string(const int &val_);

#endif // HAVE_GETOPT_LONG

  static void locale_global_();

#if HAVE_GETOPT_LONG

  static const struct option longopts[];

#endif // HAVE_GETOPT_LONG

#if HAVE_GETOPT_LONG

  void set_indexptr();

#endif // HAVE_GETOPT_LONG

  void flagOptionCase(bool (basic_Tagger::Flags::*GetFlag)() const,
                      void (basic_Tagger::Flags::*SetFlag)(const bool &));
  std::string option_string();
  void functionTypeTypeOptionCase(const FunctionTypeType &FunctionTypeType_);
  void functionTypeOptionCase(const FunctionType &FunctionType_);
  void getIterationsArgument();
  unsigned long optarg_unsigned_long() const;
  void g_StreamTagger(basic_StreamTagger &StreamTagger_);
  void s_StreamTaggerTrainer(basic_StreamTaggerTrainer &StreamTaggerTrainer_);
  void g_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void r_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void s_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  void t_FILE_Tagger(FILE_Tagger &FILE_Tagger_);
  int &argc;
  char **&argv;
  int The_val;

#if HAVE_GETOPT_LONG

  int The_indexptr;
  Optional<int> FunctionTypeTypeOption_indexptr;
  Optional<int> FunctionTypeOption_indexptr;

#else

  Optional<int> FunctionTypeTypeOption_val;
  Optional<int> FunctionTypeOptiona_val;

#endif // HAVE_GETOPT_LONG

  Optional<FunctionTypeType> TheFunctionTypeType;
  Optional<UnigramType> TheUnigramType;
  Optional<FunctionType> TheFunctionType;
  unsigned long TheFunctionTypeOptionArgument;
  basic_Tagger::Flags TheFlags;
};
}

#endif // APERTIUM_TAGGER_H
