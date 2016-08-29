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
#include "err_exception.h"
#include "exception.h"
#include "file_tagger.h"
#include "linebreak.h"
#include "stream_5_3_1_tagger.h"
#include "stream_5_3_1_tagger_trainer.h"
#include "stream_5_3_2_tagger.h"
#include "stream_5_3_2_tagger_trainer.h"
#include "stream_5_3_3_tagger.h"
#include "stream_5_3_3_tagger_trainer.h"
#include <apertium/perceptron_tagger.h>
#include <apertium/hmm.cc>
#include <apertium/lswpost.h>
#include <apertium/tagger_word.h>
#include <apertium/shell_utils.h>

#include <lttoolbox/lt_locale.h>

#include "getopt_long.h"
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <locale>
#include <sstream>
#include <string>
#include <unistd.h>

namespace Apertium {
using namespace ShellUtils;

/** Top level argument parsing */

apertium_tagger::apertium_tagger(int &argc, char **&argv)
    : argc(argc), argv(argv), The_val(), nonoptarg(),

      The_indexptr(), FunctionTypeTypeOption_indexptr(),
      FunctionTypeOption_indexptr(),

      TheFunctionTypeType(), TheUnigramType(), TheFunctionType(),
      TheFunctionTypeOptionArgument(0), TheFlags() {
  try {
    while (true) {
      The_val = getopt_long(argc, argv, "bdfegmpr:s:t:u:wxz", longopts, &The_indexptr);

      if (The_val == -1)
        break;

      set_indexptr();

      switch (The_val) {
      case 'b':
        flagOptionCase(&basic_Tagger::Flags::getSentSeg,
                       &basic_Tagger::Flags::setSentSeg);
        break;
      case 'd':
        flagOptionCase(&basic_Tagger::Flags::getDebug,
                       &basic_Tagger::Flags::setDebug);
        break;
      case 'e':
        flagOptionCase(&basic_Tagger::Flags::getSkipErrors,
                       &basic_Tagger::Flags::setSkipErrors);
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
      case 'x':
        functionTypeTypeOptionCase(Perceptron);
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

    nonoptarg = argc - optind;

    switch (*TheFunctionType) {
    case Tagger:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        g_FILE_Tagger(HiddenMarkovModelTagger_);
        break;
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
      case Perceptron: {
        PerceptronTagger perceptron(TheFlags);
        g_StreamTagger(perceptron);
      } break;
      default:
        std::abort();
      }

      break;
    case Retrain:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        r_FILE_Tagger(HiddenMarkovModelTagger_);
        break;
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
        break;
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
      } break;
      case Perceptron: {
        PerceptronTagger perceptron(TheFlags);
        s_StreamTaggerTrainer(perceptron);
      } break;
      default:
        std::abort();
      }

      break;
    case Train:
      if (!TheFunctionTypeType) {
        HMM HiddenMarkovModelTagger_;
        t_FILE_Tagger(HiddenMarkovModelTagger_);
        break;
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
    std::wcerr << "apertium-tagger: " << basic_ExceptionType_.what() << std::endl;
    throw err_Exception();
  }
}

void apertium_tagger::help() {

  std::wcerr <<
"Usage: apertium-tagger [OPTION]... -g SERIALISED_TAGGER                        \\\n"
"                                      [INPUT                                   \\\n"
"                                      [OUTPUT]]\n"
"\n"
"  or:  apertium-tagger [OPTION]... -r ITERATIONS                               \\\n"
"                                      CORPUS                                   \\\n"
"                                      SERIALISED_TAGGER\n"
"\n"
"  or:  apertium-tagger [OPTION]... -s ITERATIONS                               \\\n"
"                                      DICTIONARY                               \\\n"
"                                      CORPUS                                   \\\n"
"                                      TAGGER_SPECIFICATION                     \\\n"
"                                      SERIALISED_TAGGER                        \\\n"
"                                      TAGGED_CORPUS                            \\\n"
"                                      UNTAGGED_CORPUS\n"
"\n"
"  or:  apertium-tagger [OPTION]... -s 0                                        \\\n"
"                                      DICTIONARY                               \\\n"
"                                      TAGGER_SPECIFICATION                     \\\n"
"                                      SERIALISED_TAGGER                        \\\n"
"                                      TAGGED_CORPUS                            \\\n"
"                                      UNTAGGED_CORPUS\n"
"\n"
"  or:  apertium-tagger [OPTION]... -s 0                                        \\\n"
"                                   -u MODEL                                    \\\n"
"                                      SERIALISED_TAGGER                        \\\n"
"                                      TAGGED_CORPUS\n"
"\n"
"  or:  apertium-tagger [OPTION]... -t ITERATIONS                               \\\n"
"                                      DICTIONARY                               \\\n"
"                                      CORPUS                                   \\\n"
"                                      TAGGER_SPECIFICATION                     \\\n"
"                                      SERIALISED_TAGGER\n"
"\n"
"Mandatory arguments to long options are mandatory for short options too.\n"
"\n";

  std::vector<std::pair<std::string, std::string> > options_description_;
  options_description_.push_back(std::make_pair("-d, --debug",            "with -g, print error messages about the input"));
  options_description_.push_back(std::make_pair("-f, --first",            "with -g, reorder each lexical unit's analyses so that the chosen one is first"));
  options_description_.push_back(std::make_pair("-m, --mark",             "with -g, mark disambiguated lexical units"));
  options_description_.push_back(std::make_pair("-p, --show-superficial", "with -g, output each lexical unit's surface form"));
  options_description_.push_back(std::make_pair("-z, --null-flush",       "with -g, flush the output after getting each null character"));
  align::align_(options_description_);
  std::wcerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-u, --unigram=MODEL", "use unigram algorithm MODEL from <http://coltekin.net/cagri/papers/trmorph-tools.pdf>"));
  align::align_(options_description_);
  std::wcerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-w, --sliding-window", "use the Light Sliding Window algorithm"));
  options_description_.push_back(std::make_pair("-x, --perceptron", "use the averaged perceptron algorithm"));
  options_description_.push_back(std::make_pair("-e, --skip-on-error", "with -xs, ignore certain types of errors with the training corpus"));
  align::align_(options_description_);
  std::wcerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-g, --tagger", "disambiguate the input"));
  align::align_(options_description_);
  std::wcerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-r, --retrain=ITERATIONS", "with -u: exit;\notherwise: retrain the tagger with ITERATIONS unsupervised iterations"));
  options_description_.push_back(std::make_pair("-s, --supervised=ITERATIONS", "with -u: train the tagger with a hand-tagged corpus;\nwith -w: exit;\notherwise: initialise the tagger with a hand-tagged corpus and retrain it with ITERATIONS unsupervised iterations"));
  options_description_.push_back(std::make_pair("-t, --train=ITERATIONS", "with -u: exit;\notherwise: train the tagger with ITERATIONS unsupervised iterations"));
  align::align_(options_description_);
  std::wcerr << '\n';
  options_description_.clear();
  options_description_.push_back(std::make_pair("-h, --help", "display this help and exit"));
  align::align_(options_description_);
}

const struct option apertium_tagger::longopts[] = {
    {"help", no_argument, 0, 'h'},
    {"sent-seg", no_argument, 0, 'b'},
    {"debug", no_argument, 0, 'd'},
    {"skip-on-error", no_argument, 0, 'e'},
    {"first", no_argument, 0, 'f'},
    {"mark", no_argument, 0, 'm'},
    {"show-superficial", no_argument, 0, 'p'},
    {"null-flush", no_argument, 0, 'z'},
    {"unigram", required_argument, 0, 'u'},
    {"sliding-window", no_argument, 0, 'w'},
    {"perceptron", no_argument, 0, 'x'},
    {"tagger", no_argument, 0, 'g'},
    {"retrain", required_argument, 0, 'r'},
    {"supervised", required_argument, 0, 's'},
    {"train", required_argument, 0, 't'},
    {0, 0, 0, 0}};

/** Utilities */

std::string apertium_tagger::option_string(const int &indexptr_) {
  return option_string(longopts[indexptr_]);
}

std::string apertium_tagger::option_string(const struct option &option_) {
  std::stringstream option_string_;
  option_string_ << "--" << option_.name;
  return option_string_.str();
}

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
  return option_string(The_indexptr);
}

void apertium_tagger::functionTypeTypeOptionCase(
    const FunctionTypeType &FunctionTypeType_) {
  if (FunctionTypeTypeOption_indexptr) {
    std::stringstream what_;
    what_ << "unexpected '" << option_string() << "' following '"
          << option_string(*FunctionTypeTypeOption_indexptr)
          << '\'';
    throw Exception::apertium_tagger::UnexpectedFunctionTypeTypeOption(what_);
  }

  TheFunctionTypeType = FunctionTypeType_;
  FunctionTypeTypeOption_indexptr = The_indexptr;
}

void apertium_tagger::functionTypeOptionCase(
    const FunctionType &FunctionType_) {
  if (FunctionTypeOption_indexptr) {
    std::stringstream what_;
    what_ << "unexpected '" << option_string() << "' following '"
          << option_string(*FunctionTypeOption_indexptr)
          << '\'';
    throw Exception::apertium_tagger::UnexpectedFunctionTypeOption(what_);
  }

  TheFunctionType = FunctionType_;
  FunctionTypeOption_indexptr = The_indexptr;
}

void apertium_tagger::getIterationsArgument() {
  try {
    TheFunctionTypeOptionArgument = optarg_unsigned_long("ITERATIONS");
  } catch (const ExceptionType &ExceptionType_) {
    std::stringstream what_;
    what_ << "invalid argument '" << optarg << "' for '" << option_string()
          << '\'';
    throw Exception::apertium_tagger::InvalidArgument(what_);
  }
}

static unsigned long parse_unsigned_long(const char *metavar, const char *val) {
  char *str_end;
  errno = 0;
  unsigned long N_0 = std::strtoul(val, &str_end, 10);

  if (*str_end != '\0') {
    std::stringstream what_;
    what_ << "can't convert " << metavar << " \"" << val << "\" to unsigned long";
    throw Exception::apertium_tagger::str_end_not_eq_NULL(what_);
  }

  if (*val == '\0') {
    std::stringstream what_;
    what_ << "can't convert " << metavar << " of size 1 \"\" to unsigned long";
    throw Exception::apertium_tagger::optarg_eq_NULL(what_);
  }

  if (errno == ERANGE) {
    std::stringstream what_;
    what_ << "can't convert " << metavar << " \"" << val
          << "\" to unsigned long, not in unsigned long range";
    throw Exception::apertium_tagger::ERANGE_(what_);
  }

  return N_0;
}

unsigned long apertium_tagger::optarg_unsigned_long(const char *metavar) {
  return parse_unsigned_long(metavar, optarg);
}

void apertium_tagger::get_file_arguments(
    bool get_crp_fn,
    char **DicFn, char **CrpFn,
    char **TaggedFn, char **UntaggedFn,
    char **TsxFn, char **ProbFn) {
  if (*TheFunctionType != Retrain) {
    *DicFn = argv[optind++];
  }
  if (get_crp_fn) {
    *CrpFn = argv[optind++];
  }
  if (*TheFunctionType == Supervised) {
    *TsxFn = argv[optind++];
    *ProbFn = argv[optind++];
    *TaggedFn = argv[optind++];
  }
  *UntaggedFn = argv[optind++];
  if (*TheFunctionType == Supervised && !get_crp_fn) {
    *CrpFn = *UntaggedFn;
  }
  if (*TheFunctionType != Supervised) {
    if (*TheFunctionType != Retrain) {
      *TsxFn = argv[optind++];
    }
    *ProbFn = argv[optind++];
  }
}

void apertium_tagger::init_FILE_Tagger(FILE_Tagger &FILE_Tagger_, string const &TsxFn) {
  FILE_Tagger_.deserialise(TsxFn);
  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::setArrayTags(FILE_Tagger_.getArrayTags());
}

MorphoStream* apertium_tagger::setup_untagged_morpho_stream(
    FILE_Tagger &FILE_Tagger_,
    char *DicFn, char *UntaggedFn,
    FILE **Dictionary, FILE **UntaggedCorpus) {
  if (*TheFunctionType != Retrain) {
    *Dictionary = try_open_file_utf8("DICTIONARY", DicFn, "r");
  }
  *UntaggedCorpus = try_open_file_utf8("UNTAGGED_CORPUS", UntaggedFn, "r");

  FILE_Tagger_.read_dictionary(*Dictionary);

  return new FileMorphoStream(*UntaggedCorpus, true, &FILE_Tagger_.get_tagger_data());
}

void apertium_tagger::close_untagged_files(
    char *DicFn, char *UntaggedFn,
    FILE *Dictionary, FILE *UntaggedCorpus) {
  if (*TheFunctionType == Supervised || *TheFunctionType == Train) {
    try_close_file("DICTIONARY", DicFn, Dictionary);
  }
  try_close_file("UNTAGGED_CORPUS", UntaggedFn, UntaggedCorpus);
}

/** Implementation of flags/subcommands */

void apertium_tagger::g_StreamTagger(StreamTagger &StreamTagger_) {
  locale_global_();

  expect_file_arguments(nonoptarg, 1, 4);

  std::ifstream SerialisedAnalysisFrequencies;
  try_open_fstream("SERIALISED_TAGGER", argv[optind],
                   SerialisedAnalysisFrequencies);

  try {
    StreamTagger_.deserialise(SerialisedAnalysisFrequencies);
  } catch (const basic_ExceptionType &basic_ExceptionType_) {
    std::stringstream what_;
    what_ << "can't deserialise SERIALISED_TAGGER file \"" << argv[optind]
          << "\" Reason: " << basic_ExceptionType_.what();
    throw Exception::apertium_tagger::deserialise(what_);
  }

  if (nonoptarg < 2) {
    Stream Input(TheFlags);
    StreamTagger_.tag(Input, std::wcout);
    return;
  }

  std::wifstream Input_stream;
  try_open_fstream("INPUT", argv[optind + 1], Input_stream);

  if (nonoptarg < 3) {
    Stream Input(TheFlags, Input_stream, argv[optind + 1]);
    StreamTagger_.tag(Input, std::wcout);
    return;
  }

  std::wofstream Output_stream;
  try_open_fstream("OUTPUT", argv[optind + 2], Input_stream);

  Stream Input(TheFlags, Input_stream, argv[optind + 1]);
  StreamTagger_.tag(Input, Output_stream);
}

void apertium_tagger::s_StreamTaggerTrainer(
    StreamTaggerTrainer &StreamTaggerTrainer_) {
  locale_global_();

  if (TheFunctionTypeOptionArgument != 0 && *TheFunctionTypeType != Perceptron) {
    std::stringstream what_;
    what_ << "invalid argument '" << TheFunctionTypeOptionArgument
          << "' for '--supervised'";
    throw Exception::apertium_tagger::InvalidArgument(what_);
  }

  if (*TheFunctionTypeType == Perceptron) {
    expect_file_arguments(nonoptarg, 4);
  } else {
    expect_file_arguments(nonoptarg, 2);
  }

  std::wifstream TaggedCorpus_stream;
  try_open_fstream("TAGGED_CORPUS", argv[optind + 1], TaggedCorpus_stream);
  Stream TaggedCorpus(TheFlags, TaggedCorpus_stream, argv[optind + 1]);

  if (*TheFunctionTypeType == Perceptron) {
    std::wifstream UntaggedCorpus_stream;
    try_open_fstream("UNTAGGED_CORPUS", argv[optind + 2], UntaggedCorpus_stream);
    Stream UntaggedCorpus(TheFlags, UntaggedCorpus_stream, argv[optind + 2]);

    PerceptronTagger &pt = dynamic_cast<PerceptronTagger&>(StreamTaggerTrainer_);
    pt.read_spec(argv[optind + 3]);
    pt.train(TaggedCorpus, UntaggedCorpus, TheFunctionTypeOptionArgument);
  } else {
    StreamTaggerTrainer_.train(TaggedCorpus);
  }

  std::ofstream Serialised_basic_Tagger;
  try_open_fstream("SERIALISED_TAGGER", argv[optind],
                   Serialised_basic_Tagger);

  StreamTaggerTrainer_.serialise(Serialised_basic_Tagger);
}

void apertium_tagger::g_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  expect_file_arguments(nonoptarg, 1, 4);

  FILE *Serialised_FILE_Tagger =
      try_open_file("SERIALISED_TAGGER", argv[optind], "rb");
  FILE_Tagger_.deserialise(Serialised_FILE_Tagger);
  try_close_file("SERIALISED_TAGGER", argv[optind], Serialised_FILE_Tagger);

  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::setArrayTags(FILE_Tagger_.getArrayTags());
  TaggerWord::generate_marks = TheFlags.getMark();
  FILE_Tagger_.set_show_sf(TheFlags.getShowSuperficial());
  FILE_Tagger_.setNullFlush(TheFlags.getNullFlush());

  if (nonoptarg < 2)
    FILE_Tagger_.tagger(stdin, stdout, TheFlags.getFirst());
  else {
    FILE *Input = try_open_file("INPUT", argv[optind + 1], "r");

    if (nonoptarg < 3)
      FILE_Tagger_.tagger(Input, stdout, TheFlags.getFirst());
    else {
      FILE *Output = try_open_file_utf8("OUTPUT", argv[optind + 2], "w");
      FILE_Tagger_.tagger(Input, Output, TheFlags.getFirst());
      try_close_file("OUTPUT", argv[optind + 2], Output);
    }

    try_close_file("INPUT", argv[optind + 1], Input);
  }
}

void apertium_tagger::r_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  expect_file_arguments(nonoptarg, 2);

  char *ProbFn, *UntaggedFn;

  get_file_arguments(
      false,
      NULL, NULL, NULL, &UntaggedFn,
      NULL, &ProbFn);

  FILE *Serialised_FILE_Tagger =
      try_open_file("SERIALISED_TAGGER", ProbFn, "rb");
  FILE_Tagger_.deserialise(Serialised_FILE_Tagger);
  try_close_file("SERIALISED_TAGGER", ProbFn, Serialised_FILE_Tagger);

  FILE_Tagger_.set_debug(TheFlags.getDebug());
  TaggerWord::setArrayTags(FILE_Tagger_.getArrayTags());

  FILE *UntaggedCorpus;
  MorphoStream* ms = setup_untagged_morpho_stream(
    FILE_Tagger_,
    NULL, UntaggedFn,
    NULL, &UntaggedCorpus);

  FILE_Tagger_.train(*ms, TheFunctionTypeOptionArgument);
  delete ms;
  close_untagged_files(
    NULL, UntaggedFn,
    NULL, UntaggedCorpus);

  Serialised_FILE_Tagger =
      try_open_file("SERIALISED_TAGGER", ProbFn, "wb");
  FILE_Tagger_.serialise(Serialised_FILE_Tagger);
  try_close_file("SERIALISED_TAGGER", ProbFn, Serialised_FILE_Tagger);
}

void apertium_tagger::s_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  if (TheFunctionTypeOptionArgument == 0) {
    expect_file_arguments(nonoptarg, 5, 7);
  } else {
    expect_file_arguments(nonoptarg, 6);
  }
  char *DicFn, *CrpFn, *TsxFn, *ProbFn, *TaggedFn, *UntaggedFn;
  bool do_unsup = nonoptarg == 6;

  get_file_arguments(
      do_unsup,
      &DicFn, &CrpFn, &TaggedFn, &UntaggedFn,
      &TsxFn, &ProbFn);
  init_FILE_Tagger(FILE_Tagger_, TsxFn);

  FILE *Dictionary, *UntaggedCorpus;
  MorphoStream* ms = setup_untagged_morpho_stream(
    FILE_Tagger_,
    DicFn, UntaggedFn,
    &Dictionary, &UntaggedCorpus);
  FILE *TaggedCorpus = try_open_file("TAGGED_CORPUS", TaggedFn, "r");
  FileMorphoStream tms(TaggedCorpus, true, &FILE_Tagger_.get_tagger_data());

  FILE_Tagger_.init_probabilities_from_tagged_text_(tms, *ms);
  try_close_file("TAGGED_CORPUS", TaggedFn, TaggedCorpus);
  delete ms;
  close_untagged_files(
    DicFn, UntaggedFn,
    Dictionary, UntaggedCorpus);

  if (do_unsup) {
    FILE *Corpus = try_open_file_utf8("CORPUS", CrpFn, "r");
    FILE_Tagger_.train(Corpus, TheFunctionTypeOptionArgument);
    try_close_file("CORPUS", CrpFn, Corpus);
 }

  FILE *Serialised_FILE_Tagger =
      try_open_file("SERIALISED_TAGGER", ProbFn, "wb");
  FILE_Tagger_.serialise(Serialised_FILE_Tagger);
  try_close_file("SERIALISED_TAGGER", ProbFn, Serialised_FILE_Tagger);
}

void apertium_tagger::t_FILE_Tagger(FILE_Tagger &FILE_Tagger_) {
  LtLocale::tryToSetLocale();

  expect_file_arguments(nonoptarg, 4);

  char *DicFn, *TsxFn, *ProbFn, *UntaggedFn;
  UntaggedFn = NULL;

  get_file_arguments(
      false,
      &DicFn, NULL, NULL, &UntaggedFn,
      &TsxFn, &ProbFn);
  init_FILE_Tagger(FILE_Tagger_, TsxFn);

  FILE *Dictionary, *UntaggedCorpus;
  MorphoStream* ms = setup_untagged_morpho_stream(
    FILE_Tagger_,
    DicFn, UntaggedFn,
    &Dictionary, &UntaggedCorpus);

  FILE_Tagger_.init_and_train(*ms, TheFunctionTypeOptionArgument);
  delete ms;
  close_untagged_files(
    DicFn, UntaggedFn,
    Dictionary, UntaggedCorpus);

  FILE *Serialised_FILE_Tagger =
      try_open_file("SERIALISED_TAGGER", ProbFn, "wb");
  FILE_Tagger_.serialise(Serialised_FILE_Tagger);
  try_close_file("SERIALISED_TAGGER", ProbFn, Serialised_FILE_Tagger);

}
}

int main(int argc, char **argv) {
  try {
    apertium_tagger(argc, argv);
  } catch (const err_Exception &err_Exception_) {
    std::wcerr << "Try 'apertium-tagger --help' for more information." << std::endl;
    return 1;
  } catch (...) {
    throw;
  }
}
