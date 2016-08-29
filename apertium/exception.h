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

#ifndef EXCEPTION_APERTIUM_TAGGER_H
#define EXCEPTION_APERTIUM_TAGGER_H

#include "exception_type.h"

#include <sstream>

namespace Apertium {
namespace Exception {

#define EXCEPTION(EXCEPTION_TYPE)                                              \
  class EXCEPTION_TYPE : public ::Apertium::ExceptionType {                    \
  public:                                                                      \
    EXCEPTION_TYPE(const char *const what_) : ExceptionType(what_) {}          \
    EXCEPTION_TYPE(const std::string &what_) : ExceptionType(what_) {}         \
    EXCEPTION_TYPE(const std::stringstream &what_) : ExceptionType(what_) {}   \
    ~EXCEPTION_TYPE() throw() {}                                               \
  };

EXCEPTION(UnalignedStreams);

namespace Analysis {
EXCEPTION(TheMorphemes_empty)
}

namespace Shell {
EXCEPTION(UnexpectedFileArgumentCount)
EXCEPTION(StreamOpenError)
EXCEPTION(FopenError)
EXCEPTION(FcloseError)
}

namespace apertium_tagger {
EXCEPTION(deserialise)
EXCEPTION(optarg_eq_NULL)
EXCEPTION(str_end_not_eq_NULL)
EXCEPTION(ERANGE_)
EXCEPTION(InvalidArgument)
EXCEPTION(InvalidArgumentCombination)
EXCEPTION(InvalidOption)
EXCEPTION(UnexpectedFlagOption)
EXCEPTION(UnexpectedFunctionTypeOption)
EXCEPTION(UnexpectedFunctionTypeTypeOption)
EXCEPTION(UnimplementedOpcode)
}

namespace Deserialiser {
EXCEPTION(size_t_)
EXCEPTION(not_Stream_good)
EXCEPTION(wchar_t_)
}

namespace LexicalUnit {
EXCEPTION(TheAnalyses_empty)
}

namespace Morpheme {
EXCEPTION(TheLemma_empty)
EXCEPTION(TheTags_empty)
}

namespace Optional {
EXCEPTION(TheOptionalTypePointer_null)
}

namespace Serialiser {
EXCEPTION(not_Stream_good)
EXCEPTION(size_t_)
EXCEPTION(wchar_t_)
}

namespace Tag {
EXCEPTION(TheTags_empty)
}

namespace wchar_t_ExceptionType {
EXCEPTION(EILSEQ_)
}

#undef EXCEPTION
}
}

#endif // EXCEPTION_APERTIUM_TAGGER_H
