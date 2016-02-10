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

#include "apertium_tagger.h"

#include "apertium_config.h"

#include "align.h"
#include "basic_exception_type.h"
#include "basic_stream_tagger.h"
#include "basic_stream_tagger_trainer.h"
#include "basic_tagger.h"
#include "exception.h"
#include "file_tagger.h"
#include "err_exception.h"
#include <apertium/hmm.cc>
#include "linebreak.h"
#include <apertium/lswpost.h>
#include "stream_5_3_1_tagger.h"
#include "stream_5_3_1_tagger_trainer.h"
#include "stream_5_3_2_tagger.h"
#include "stream_5_3_2_tagger_trainer.h"
#include "stream_5_3_3_tagger.h"
#include "stream_5_3_3_tagger_trainer.h"
#include <apertium/tagger_word.h>

#include <lttoolbox/lt_locale.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <getopt.h>
#include <iomanip>
#include <ios>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <unistd.h>

#ifdef _MSC_VER
#include <fcntl.h>
#include <io.h>
#endif // _MSC_VER

namespace Apertium {
apertium_tagger::apertium_tagger(int &argc, char **&argv)
    : argc(argc), argv(argv), The_val(),

#if HAVE_GETOPT_LONG

      The_indexptr(), FunctionTypeTypeOption_indexptr(),
      FunctionTypeOption_indexptr(),

#else

      FunctionTypeTypeOption_val(), FunctionTypeOptiona_val(),

#endif // HAVE_GETOPT_LONG

      TheFunctionTypeType(), TheUnigramType(), TheFunctionType(),
      TheFunctionTypeOptionArgument(0), TheFlags() {
  try {
    while (true) {
      The_val =

#if HAVE_GETOPT_LONG

          getopt_long(argc, argv, "dfgmpr:s:t:u:wz", longopts, &The_indexptr);

#else

          getopt(argc, argv, "dfgmpr:s:t:u:wz");

#endif // HAVE_GETOPT_LONG

      if (The_val == -1)
        break;

#if HAVE_GETOPT_LONG

      set_indexptr();

#endif // HAVE_GETOPT_LONG

      switch (The_val) {
      case 'd':
        flagOptionCase(&basic_Tagger::Flags::getDebug,
                       &basic_Tagger::Flags::setDebug);
        break;
      case 'f':
        flagOptionCase(&basic_Tagger::Flags::getFirst,
                       &basic_Tagger::Flags::setFirst);
        break;
      case 'm':
        flagOptionCase(&basic_Tagger::Flags::getMark,
                       &basic_Tagger::Flags::setMark);
        break;
      case 'p':
        flagOptionCase(&basic_Tagger::Flags::getShowSuperficial,
                       &basic_Tagger::Flags::setShowSuperficial);
        break;
      case 'z':
        flagOptionCase(&basic_Tagger::Flags::getNullFlush,
                       &basic_Tagger::Flags::setNullFlush);
        break;
      case 'u':
        functionTypeTypeOptionCase(Unigram);

        if (std::strncmp(optarg, "1", sizeof "1" - 1) == 0) {
          TheUnigramType = Stream_5_3_1;
          break;
        }

        if (std::strncmp(optarg, "2", sizeof "2" - 1) == 0) {
          TheUnigramType = Stream_5_3_2;
          break;
        }

        if (std::strncmp(optarg, "3", sizeof "3" - 1) == 0) {
          TheUnigramType = Stream_5_3_3;
          break;
        }

        {
          std::stringstream what_;
          what_ << "invalid argument '" << optarg << "' for '--unigram'\n"
"Valid arguments are:\n"
"  - '1'\n"
"  - '2'\n"
"  - '3'";
          throw Exception::apertium_tagger::InvalidArgument(what_);
        }
        break;
      case 'w':
        functionTypeTypeOptionCase(SlidingWindow);
        break;
      case 'g':
        functionTypeOptionCase(Tagger);
        break;
      case 'r':
        functionTypeOptionCase(Retrain);
        getIterationsArgument();
        break;
      case 's':
        functionTypeOptionCase(Supervised);
        getIterationsArgument();
        break;
      case 't':
        functionTypeOptionCase(Train);
        getIterationsArgument();
        break;
      case 'h':
        help();
        return;
      default:
        throw err_Exception();
      }
    }

    if (!TheFunctionType) {
      help();
      return;
    }

    switch (*TheFunctionType) {
    case Tagger:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        g_FILE_Tagger(HiddenMarkovModelTagger_);
      }

      switch (*TheFunctionTypeType) {
      case Unigram: {
        switch (*TheUnigramType) {
        case Stream_5_3_1: {
          Stream_5_3_1_Tagger Stream_5_3_1_Tagger_(TheFlags);
          g_StreamTagger(Stream_5_3_1_Tagger_);
        } break;
        case Stream_5_3_2: {
          Stream_5_3_2_Tagger Stream_5_3_2_Tagger_(TheFlags);
          g_StreamTagger(Stream_5_3_2_Tagger_);
        } break;
        case Stream_5_3_3: {
          Stream_5_3_3_Tagger Stream_5_3_3_Tagger_(TheFlags);
          g_StreamTagger(Stream_5_3_3_Tagger_);
        } break;
        default:
          std::abort();
        }
      } break;
      case SlidingWindow: {
        LSWPoST SlidingWindowTagger_;
        g_FILE_Tagger(SlidingWindowTagger_);
      } break;
      default:
        std::abort();
      }

      break;
    case Retrain:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        r_FILE_Tagger(HiddenMarkovModelTagger_);
      }
      switch (*TheFunctionTypeType) {
      case Unigram: {
        std::stringstream what_;
        what_ << "invalid option -- 'u'";
        throw Exception::apertium_tagger::InvalidOption(what_);
      }
      case SlidingWindow: {
        LSWPoST SlidingWindowTagger_;
        r_FILE_Tagger(SlidingWindowTagger_);
      } break;
      default:
        std::abort();
      }

      break;
    case Supervised:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        s_FILE_Tagger(HiddenMarkovModelTagger_);
      }
      switch (*TheFunctionTypeType) {
      case Unigram: {
        switch (*TheUnigramType) {
        case Stream_5_3_1: {
          Stream_5_3_1_TaggerTrainer Stream_5_3_1_TaggerTrainer_(TheFlags);
          s_StreamTaggerTrainer(Stream_5_3_1_TaggerTrainer_);
        } break;
        case Stream_5_3_2: {
          Stream_5_3_2_TaggerTrainer Stream_5_3_2_TaggerTrainer_(TheFlags);
          s_StreamTaggerTrainer(Stream_5_3_2_TaggerTrainer_);
        } break;
        case Stream_5_3_3: {
          Stream_5_3_3_TaggerTrainer Stream_5_3_3_TaggerTrainer_(TheFlags);
          s_StreamTaggerTrainer(Stream_5_3_3_TaggerTrainer_);
        } break;
        default:
          std::abort();
        }
      } break;
      case SlidingWindow: {
        std::stringstream what_;
        what_ << "invalid option -- 'w'";
        throw Exception::apertium_tagger::InvalidOption(what_);
      }
      default:
        std::abort();
      }

      break;
    case Train:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        t_FILE_Tagger(HiddenMarkovModelTagger_);
      }
      switch (*TheFunctionTypeType) {
      case Unigram: {
        std::stringstream what_;
        what_ << "invalid option -- 'u'";
        throw Exception::apertium_tagger::InvalidOption(what_);
      }
      case SlidingWindow: {
        LSWPoST SlidingWindowTagger_;
        t_FILE_Tagger(SlidingWindowTagger_);
      } break;
      default:
        std::abort();
      }

      break;
    default:
      std::abort();
    }
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::cerr << "apertium-tagger: " << basic_ExceptionType_.what() << '\n';
    throw err_Exception();
  }
}

void apertium_tagger::help() {

#if HAVE_GETOPT_LONG
#define NAME(NAME_) ", --" #NAME_
#define ARGUMENT(ARGUMENT_) "=" #ARGUMENT_
#else
#define NAME(NAME_)
#define ARGUMENT(ARGUMENT_) " " #ARGUMENT_
#endif // HAVE_GETOPT_LONG

  std::cerr <<
"Usage: apertium-tagger [OPTION]... -g SERIALISED_BASIC_TAGGER                  \\\n"
"                                      [INPUT                                   \\\n"
"                                      [OUTPUT]]\n"
"\n"
"  or:  apertium-tagger [OPTION]... -r ITERATIONS                               \\\n"
"                                      CORPUS                                   \\\n"
"                                      SERIALISED_BASIC_TAGGER\n"
"\n"
"  or:  apertium-tagger [OPTION]... -s ITERATIONS                               \\\n"
"                                      DICTIONARY                               \\\n"
"                                      CORPUS                                   \\\n"
"                                      TAGGER_SPECIFICATION                     \\\n"
"                                      SERIALISED_BASIC_TAGGER                  \\\n"
"                                      TAGGED_CORPUS                            \\\n"
"                                      UNTAGGED_CORPUS\n"
"\n"
"  or:  apertium-tagger [OPTION]... -s 0                                        \\\n"
"                                   -u MODEL                                    \\\n"
"                                      SERIALISED_BASIC_TAGGER                  \\\n"
"                                      TAGGED_CORPUS\n"
"\n"
"  or:  apertium-tagger [OPTION]... -t ITERATIONS                               \\\n"
"                                      DICTIONARY                               \\\n"
"                                      CORPUS                                   \\\n"
"                                      TAGGER_SPECIFICATION                     \\\n"
"                                      SERIALISED_BASIC_TAGGER\n"
"\n"
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n"
"\n";

  std::vector<std::pair<std::string, std::string> > options_description_;
  options_description_.push_back(std::make_pair("-d" NAME(debug),            "with -g, print error messages about the input"));
  options_description_.push_back(std::make_pair("-f" NAME(first),            "with -g, reorder each lexical unit's analyses so that the chosen one is first"));
  options_description_.push_back(std::make_pair("-m" NAME(mark),             "with -g, mark disambiguated lexical units"));
  options_description_.push_back(std::make_pair("-p" NAME(show-superficial), "with -g, output each lexical unit's surface form"));
  options_description_.push_back(std::make_pair("-z" NAME(null-flush),       "with -g, flush the output after getting each null character"));
  align::align_(options_description_);
  std::cerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-u" NAME(unigram) ARGUMENT(MODEL), "use unigram algorithm MODEL from <http://coltekin.net/cagri/papers/trmorph-tools.pdf>"));
  align::align_(options_description_);
  std::cerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-w" NAME(sliding-window), "use the Light Sliding Window algorithm"));
  align::align_(options_description_);
  std::cerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-g" NAME(tagger), "disambiguate the input"));
  align::align_(options_description_);
  std::cerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-r" NAME(retrain) ARGUMENT(ITERATIONS), "with -u: exit;\notherwise: retrain the tagger with ITERATIONS unsupervised iterations"));
  options_description_.push_back(std::make_pair("-s" NAME(supervised) ARGUMENT(ITERATIONS), "with -u: train the tagger with a hand-tagged corpus;\nwith -w: exit;\notherwise: initialise the tagger with a hand-tagged corpus and retrain it with ITERATIONS unsupervised iterations"));
  options_description_.push_back(std::make_pair("-t" NAME(train) ARGUMENT(ITERATIONS), "with -u: exit;\notherwise: train the tagger with ITERATIONS unsupervised iterations"));
  align::align_(options_description_);
  std::cerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-h" NAME(help), "display this help and exit"));
  align::align_(options_description_);
}

#if HAVE_GETOPT_LONG

std::string apertium_tagger::option_string(const int &indexptr_) {
  return option_string(longopts[indexptr_]);
}

std::string apertium_tagger::option_string(const struct option &option_) {
  std::stringstream option_string_;
  option_string_ << "--" << option_.name;
  return option_string_.str();
}

#else

std::string apertium_tagger::option_string(const int &val_) {
  std::stringstream option_string_;
  option_string_ << '-' << static_cast<char>(val_);
  return option_string_.str();
}

#endif // HAVE_GETOPT_LONG

void apertium_tagger::locale_global_() {

#if defined __clang__

  std::locale::global(std::locale(""));

#else
#if defined __APPLE__

  LtLocale::tryToSetLocale();

#else

  std::locale::global(std::locale(""));

#endif // defined __APPLE__
#endif // defined __clang__
}

#if HAVE_GETOPT_LONG

const struct option apertium_tagger::longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"debug", no_argument, 0, 'd'},
    {"first", no_argument, 0, 'f'},
    {"mark", no_argument, 0, 'm'},
    {"show-superficial", no_argument, 0, 'p'},
    {"null-flush", no_argument, 0, 'z'},
    {"unigram", no_argument, 0, 'u'},
    {"sliding-window", no_argument, 0, 'w'},
    {"tagger", no_argument, 0, 'g'},
    {"retrain", required_argument, 0, 'r'},
    {"supervised", required_argument, 0, 's'},
    {"train", required_argument, 0, 't'},
    {0, 0, 0, 0}};

#endif // HAVE_GETOPT_LONG

#if HAVE_GETOPT_LONG

void apertium_tagger::set_indexptr() {
  if (The_val == longopts[The_indexptr].val)
    return;

  for (std::size_t longopts_Index = 0; longopts[longopts_Index].val != 0;
       ++longopts_Index) {
    if (The_val == longopts[longopts_Index].val) {
      The_indexptr = longopts_Index;
      return;
    }
  }
}

#endif // HAVE_GETOPT_LONG

void apertium_tagger::flagOptionCase(
    bool (basic_Tagger::Flags::*GetFlag)() const,
    void (basic_Tagger::Flags::*SetFlag)(const bool &)) {
  if ((TheFlags.*GetFlag)()) {
    std::stringstream what_;
    what_ << "unexpected '" << option_string() << "' following '"
          << option_string() << '\'';
    throw Exception::apertium_tagger::UnexpectedFlagOption(what_);
  }

  (TheFlags.*SetFlag)(true);
}

std::string apertium_tagger::option_string() {

#if HAVE_GETOPT_LONG

  return option_string(The_indexptr);

#else

  return option_string(The_val);

#endif // HAVE_GETOPT_LONG
}

void apertium_tagger::functionTypeTypeOptionCase(
    const FunctionTypeType &FunctionTypeType_) {
  if (

#if HAVE_GETOPT_LONG

      FunctionTypeTypeOption_indexptr

#else

      FunctionTypeTypeOption_val

#endif // HAVE_GETOPT_LONG

      ) {
    std::stringstream what_;
    what_ << "unexpected '" << option_string() << "' following '"
          << option_string(

#if HAVE_GETOPT_LONG

                 *FunctionTypeTypeOption_indexptr

#else
                 *FunctionTypeTypeOption_val

#endif // HAVE_GETOPT_LONG

                 ) << '\'';
    throw Exception::apertium_tagger::UnexpectedFunctionTypeTypeOption(what_);
  }

  TheFunctionTypeType = FunctionTypeType_;

#if HAVE_GETOPT_LONG

  FunctionTypeTypeOption_indexptr = The_indexptr;

#else

  FunctionTypeTypeOption_val = The_val;

#endif // HAVE_GETOPT_LONG
}

void
apertium_tagger::functionTypeOptionCase(const FunctionType &FunctionType_) {
  if (

#if HAVE_GETOPT_LONG

      FunctionTypeOption_indexptr

#else

      FunctionTypeOptiona_val

#endif // HAVE_GETOPT_LONG

      ) {
    std::stringstream what_;
    what_ << "unexpected '" << option_string() << "' following '"
          << option_string(

#if HAVE_GETOPT_LONG

                 *FunctionTypeOption_indexptr

#else

                 *FunctionTypeOptiona_val

#endif // HAVE_GETOPT_LONG

                 ) << '\'';
    throw Exception::apertium_tagger::UnexpectedFunctionTypeOption(what_);
  }

  TheFunctionType = FunctionType_;

#if HAVE_GETOPT_LONG

  FunctionTypeOption_indexptr = The_indexptr;

#else

  FunctionTypeOptiona_val = The_val;

#endif // HAVE_GETOPT_LONG
}

void apertium_tagger::getIterationsArgument() {
  try {
    TheFunctionTypeOptionArgument = optarg_unsigned_long();
  } catch (const ExceptionType &ExceptionType_) {
    std::stringstream what_;
    what_ << "invalid argument '" << optarg << "' for '" << option_string()
          << '\'';
    throw Exception::apertium_tagger::InvalidArgument(what_);
  }
}

unsigned long apertium_tagger::optarg_unsigned_long() const {
  char *str_end;
  errno = 0;
  unsigned long N_0 = std::strtoul(optarg, &str_end, 10);

  if (*str_end != '\0') {
    std::stringstream what_;
    what_ << "can't convert char *optarg \"" << optarg << "\" to unsigned long";
    throw Exception::apertium_tagger::str_end_not_eq_NULL(what_);
  }

  if (*optarg == '\0') {
    std::stringstream what_;
    what_ << "can't convert char *optarg of size 1 \"\" to unsigned long";
    throw Exception::apertium_tagger::optarg_eq_NULL(what_);
  }

  if (errno == ERANGE) {
    std::stringstream what_;
    what_ << "can't convert char *optarg \"" << optarg
          << "\" to unsigned long, not in unsigned long range";
    throw Exception::apertium_tagger::ERANGE_(what_);
  }

  return N_0;
}

void apertium_tagger::g_StreamTagger(basic_StreamTagger &StreamTagger_) {
  locale_global_();

  if (argc - optind < 1 || !(argc - optind < 4)) {
    std::stringstream what_;
    what_ << "expected 1, 2, or 3 file arguments, got " << argc - optind;
    throw Exception::apertium_tagger::UnexpectedFileArgumentCount(what_);
  }

  std::ifstream SerialisedAnalysisFrequencies(argv[optind]);

  if (SerialisedAnalysisFrequencies.fail()) {
    std::stringstream what_;
    what_ << "can't open SERIALISED_BASIC_TAGGER file \"" << argv[optind]
          << "\"";
    throw Exception::apertium_tagger::ifstream_fail(what_);
  }

  try {
    StreamTagger_.deserialise(SerialisedAnalysisFrequencies);
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::stringstream what_;
    what_ << "can't deserialise SERIALISED_BASIC_TAGGER file \"" << argv[optind]
          << "\"";
    throw Exception::apertium_tagger::deserialise(what_);
  }

  if (argc - optind < 2) {
    Stream Input(TheFlags);
    StreamTagger_.tag(Input, std::wcout);
    return;
  }

  std::wifstream Input_stream(argv[optind + 1]);

  if (Input_stream.fail()) {
    std::stringstream what_;
    what_ << "can't open INPUT file \"" << argv[optind + 1] << "\"";
    throw Exception::apertium_tagger::wifstream_fail(what_);
  }

  if (argc - optind < 3) {
    Stream Input(TheFlags, Input_stream, argv[optind + 1]);
    StreamTagger_.tag(Input, std::wcout);
    return;
  }

  std::wofstream Output_stream(argv[optind + 2]);

  if (Output_stream.fail()) {
    std::stringstream what_;
    what_ << "can't open OUTPUT file \"" << argv[optind + 2] << "\"";
    throw Exception::apertium_tagger::wofstream_fail(what_);
  }

  Stream Input(TheFlags, Input_stream, argv[optind + 1]);
  StreamTagger_.tag(Input, Output_stream);
}

void apertium_tagger::s_StreamTaggerTrainer(
    basic_StreamTaggerTrainer &StreamTaggerTrainer_) {
  locale_global_();

  if (TheFunctionTypeOptionArgument != 0) {
    std::stringstream what_;
    what_ << "invalid argument '" << TheFunctionTypeOptionArgument
          << "' for '--supervised'";
    throw Exception::apertium_tagger::InvalidArgument(what_);
  }

  if (argc - optind < 2 || !(argc - optind < 3)) {
    std::stringstream what_;
    what_ << "expected 2 file arguments, got " << argc - optind;
    throw Exception::apertium_tagger::UnexpectedFileArgumentCount(what_);
  }

  std::wifstream TaggedCorpus_stream(argv[optind + 1]);

  if (TaggedCorpus_stream.fail()) {
    std::stringstream what_;
    what_ << "can't open TAGGED_CORPUS file \"" << argv[optind + 1] << "\"";
    throw Exception::apertium_tagger::wifstream_fail(what_);
  }

  Stream TaggedCorpus(TheFlags, TaggedCorpus_stream, argv[optind]);
  StreamTaggerTrainer_.train(TaggedCorpus);

  std::ofstream Serialised_basic_Tagger(argv[optind]);

  if (Serialised_basic_Tagger.fail()) {
    std::stringstream what_;
    what_ << "can't open SERIALISED_BASIC_TAGGER file \"" << argv[optind]
          << "\"";
    throw Exception::apertium_tagger::ofstream_fail(what_);
  }

  StreamTaggerTrainer_.serialise(Serialised_basic_Tagger);
}

void apertium_tagger::g_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  if (argc - optind < 1 || !(argc - optind < 4)) {
    std::stringstream what_;
    what_ << "expected 1, 2, or 3 file arguments, got " << argc - optind;
    throw Exception::apertium_tagger::UnexpectedFileArgumentCount(what_);
  }

  FILE *Serialised_FILE_Tagger = std::fopen(argv[optind], "rb");

  if (Serialised_FILE_Tagger == NULL) {
    std::stringstream what_;
    what_ << "can't open SERIALISED_BASIC_TAGGER file \"" << argv[optind]
          << "\" for reading in binary mode";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.deserialise(Serialised_FILE_Tagger);

  if (std::fclose(Serialised_FILE_Tagger) != 0) {
    std::stringstream what_;
    what_ << "can't close SERIALISED_BASIC_TAGGER file \"" << argv[optind]
          << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::generate_marks = TheFlags.getMark();
  FILE_Tagger_.set_show_sf(TheFlags.getShowSuperficial());
  FILE_Tagger_.setNullFlush(TheFlags.getNullFlush());

  if (argc - optind < 2)
    FILE_Tagger_.tagger(stdin, stdout, TheFlags.getFirst());
  else {
    FILE *Input = std::fopen(argv[optind + 1], "r");

    if (Input == NULL) {
      std::stringstream what_;
      what_ << "can't open INPUT file \"" << argv[optind + 1]
            << "\" for reading";
      throw Exception::apertium_tagger::fopen(what_);
    }

#ifdef _MSC_VER

    _setmode(_fileno(Input), _O_U8TEXT);

#endif // _MSC_VER

    if (argc - optind < 3)
      FILE_Tagger_.tagger(Input, stdout, TheFlags.getFirst());
    else {
      FILE *Output = std::fopen(argv[optind + 2], "w");

      if (Output == NULL) {
        std::stringstream what_;
        what_ << "can't open OUTPUT file \"" << argv[optind + 2]
              << "\" for writing";
        throw Exception::apertium_tagger::fopen(what_);
      }

#ifdef _MSC_VER

      _setmode(_fileno(Output), _O_U8TEXT);

#endif // _MSC_VER

      FILE_Tagger_.tagger(Input, Output, TheFlags.getFirst());

      if (std::fclose(Output) != 0) {
        std::stringstream what_;
        what_ << "can't close OUTPUT file \"" << argv[optind + 2] << "\"";
        throw Exception::apertium_tagger::fclose(what_);
      }
    }

    if (std::fclose(Input) != 0) {
      std::stringstream what_;
      what_ << "can't close INPUT file \"" << argv[optind + 1] << "\"";
      throw Exception::apertium_tagger::fclose(what_);
    }
  }
}

void apertium_tagger::r_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  if (argc - optind < 2 || !(argc - optind < 3)) {
    std::stringstream what_;
    what_ << "expected 2 file arguments, got " << argc - optind;
    throw Exception::apertium_tagger::UnexpectedFileArgumentCount(what_);
  }

  FILE *Serialised_FILE_Tagger = std::fopen(argv[optind + 1], "rb");

  if (Serialised_FILE_Tagger == NULL) {
    std::stringstream what_;
    what_ << "can't open SERIALISED_BASIC_TAGGER file \"" << argv[optind + 1]
          << "\" for reading in binary mode";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.deserialise(Serialised_FILE_Tagger);

  if (std::fclose(Serialised_FILE_Tagger) != 0) {
    std::stringstream what_;
    what_ << "can't close SERIALISED_BASIC_TAGGER file \"" << argv[optind + 1]
          << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::setArrayTags(FILE_Tagger_.getArrayTags());

  FILE *Corpus = std::fopen(argv[optind], "r");

  if (Corpus == NULL) {
    std::stringstream what_;
    what_ << "can't open CORPUS file \"" << argv[optind] << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

#ifdef _MSC_VER

  _setmode(_fileno(Corpus), _O_U8TEXT);

#endif // _MSC_VER

  FILE_Tagger_.train(Corpus, TheFunctionTypeOptionArgument);

  if (std::fclose(Corpus) != 0) {
    std::stringstream what_;
    what_ << "can't close CORPUS file \"" << argv[optind] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  Serialised_FILE_Tagger = std::fopen(argv[optind + 1], "wb");

  if (Serialised_FILE_Tagger == NULL) {
    std::stringstream what_;
    what_ << "can't open SERIALISED_BASIC_TAGGER file \"" << argv[optind + 1]
          << "\" for writing in binary mode";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.serialise(Serialised_FILE_Tagger);

  if (std::fclose(Serialised_FILE_Tagger) != 0) {
    std::stringstream what_;
    what_ << "can't close SERIALISED_BASIC_TAGGER file \"" << argv[optind + 1]
          << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }
}

void apertium_tagger::s_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  if (argc - optind < 6 || !(argc - optind < 7)) {
    std::stringstream what_;
    what_ << "expected 6 file arguments, got " << argc - optind;
    throw Exception::apertium_tagger::UnexpectedFileArgumentCount(what_);
  }

  FILE_Tagger_.deserialise(argv[optind + 2]);
  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::setArrayTags(FILE_Tagger_.getArrayTags());

  FILE *Dictionary = std::fopen(argv[optind], "r");

  if (Dictionary == NULL) {
    std::stringstream what_;
    what_ << "can't open DICTIONARY file \"" << argv[optind]
          << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.read_dictionary(Dictionary);

  if (std::fclose(Dictionary) != 0) {
    std::stringstream what_;
    what_ << "can't close DICTIONARY file \"" << argv[optind] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE *TaggedCorpus = std::fopen(argv[optind + 4], "r");

  if (TaggedCorpus == NULL) {
    std::stringstream what_;
    what_ << "can't open TAGGED_CORPUS file \"" << argv[optind + 4]
          << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE *UntaggedCorpus = std::fopen(argv[optind + 5], "r");

  if (UntaggedCorpus == NULL) {
    std::stringstream what_;
    what_ << "can't open UNTAGGED_CORPUS file \"" << argv[optind + 5]
          << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

#ifdef _MSC_VER

  _setmode(_fileno(TaggedCorpus), _O_U8TEXT);
  _setmode(_fileno(UntaggedCorpus), _O_U8TEXT);

#endif // _MSC_VER

  FILE_Tagger_.init_probabilities_from_tagged_text_(TaggedCorpus,
                                                    UntaggedCorpus);

  if (std::fclose(TaggedCorpus) != 0) {
    std::stringstream what_;
    what_ << "can't close TAGGED_CORPUS file \"" << argv[optind + 4] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  if (std::fclose(UntaggedCorpus) != 0) {
    std::stringstream what_;
    what_ << "can't close UNTAGGED_CORPUS file \"" << argv[optind + 5] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE *Corpus = std::fopen(argv[optind + 1], "r");

  if (Corpus == NULL) {
    std::stringstream what_;
    what_ << "can't open CORPUS file \"" << argv[optind + 1]
          << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

#ifdef _MSC_VER

  _setmode(_fileno(Corpus), _O_U8TEXT);

#endif // _MSC_VER

  FILE_Tagger_.train(Corpus, TheFunctionTypeOptionArgument);

  if (std::fclose(Corpus) != 0) {
    std::stringstream what_;
    what_ << "can't close CORPUS file \"" << argv[optind + 1] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE *Stream_ = std::fopen(argv[optind + 3], "wb");

  if (Stream_ == NULL) {
    std::stringstream what_;
    what_ << "can't open STREAM file \"" << argv[optind + 3]
          << "\" for writing in binary mode";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.serialise(Stream_);

  if (std::fclose(Stream_) != 0) {
    std::stringstream what_;
    what_ << "can't close STREAM file \"" << argv[optind + 3] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }
}

void apertium_tagger::t_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  if (argc - optind < 4 || !(argc - optind < 5)) {
    std::stringstream what_;
    what_ << "expected 4 file arguments, got " << argc - optind;
    throw Exception::apertium_tagger::UnexpectedFileArgumentCount(what_);
  }

  FILE_Tagger_.deserialise(argv[optind + 2]);
  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::setArrayTags(FILE_Tagger_.getArrayTags());

  FILE *Dictionary = std::fopen(argv[optind], "r");

  if (Dictionary == NULL) {
    std::stringstream what_;
    what_ << "can't open DICTIONARY file \"" << argv[optind]
          << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.read_dictionary(Dictionary);

  if (std::fclose(Dictionary) != 0) {
    std::stringstream what_;
    what_ << "can't close DICTIONARY file \"" << argv[optind] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE *Corpus = std::fopen(argv[optind + 1], "r");

  if (Corpus == NULL) {
    std::stringstream what_;
    what_ << "can't open CORPUS file \"" << argv[optind + 1]
          << "\" for reading";
    throw Exception::apertium_tagger::fopen(what_);
  }

#ifdef _MSC_VER

  _setmode(_fileno(Corpus), _O_U8TEXT);

#endif // _MSC_VER

  FILE_Tagger_.init_probabilities_kupiec_(Corpus);
  FILE_Tagger_.train_(Corpus, TheFunctionTypeOptionArgument);

  if (std::fclose(Corpus) != 0) {
    std::stringstream what_;
    what_ << "can't close CORPUS file \"" << argv[optind + 1] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }

  FILE *Stream_ = std::fopen(argv[optind + 3], "wb");

  if (Stream_ == NULL) {
    std::stringstream what_;
    what_ << "can't open STREAM file \"" << argv[optind + 3]
          << "\" for writing in binary mode";
    throw Exception::apertium_tagger::fopen(what_);
  }

  FILE_Tagger_.serialise(Stream_);

  if (std::fclose(Stream_) != 0) {
    std::stringstream what_;
    what_ << "can't close STREAM file \"" << argv[optind + 3] << "\"";
    throw Exception::apertium_tagger::fclose(what_);
  }
}
}

int main(int argc, char **argv) {
  try {
    apertium_tagger(argc, argv);
  } catch (const err_Exception &err_Exception_) {
    std::cerr << "Try 'apertium-tagger --help' for more information.\n";
    return 1;
  } catch (...) {
    throw;
  }
}
