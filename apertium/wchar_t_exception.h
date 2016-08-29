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

#ifndef WCHAR_T_EXCEPTION_H
#define WCHAR_T_EXCEPTION_H

#include "wchar_t_exception_type.h"

#include <sstream>
#include <string>

namespace Apertium {
namespace wchar_t_Exception {

#define WCHAR_T_EXCEPTION(WCHAR_T_EXCEPTION_TYPE)                              \
  class WCHAR_T_EXCEPTION_TYPE : public ::Apertium::wchar_t_ExceptionType {    \
  public:                                                                      \
    WCHAR_T_EXCEPTION_TYPE(const wchar_t *wchar_t_what_)                       \
        : wchar_t_ExceptionType(wchar_t_what_) {}                              \
    WCHAR_T_EXCEPTION_TYPE(const std::wstring &wchar_t_what_)                  \
        : wchar_t_ExceptionType(wchar_t_what_) {}                              \
    WCHAR_T_EXCEPTION_TYPE(const std::wstringstream &wchar_t_what_)            \
        : wchar_t_ExceptionType(wchar_t_what_) {}                              \
    ~WCHAR_T_EXCEPTION_TYPE() throw() {}                                       \
  };

WCHAR_T_EXCEPTION(UnalignedStreams);

namespace PerceptronTagger {
WCHAR_T_EXCEPTION(CorrectAnalysisUnavailable)
}

namespace Stream {
WCHAR_T_EXCEPTION(TheCharacterStream_not_good)
WCHAR_T_EXCEPTION(UnexpectedAnalysis)
WCHAR_T_EXCEPTION(UnexpectedCase)
WCHAR_T_EXCEPTION(UnexpectedCharacter)
WCHAR_T_EXCEPTION(UnexpectedEndOfFile)
WCHAR_T_EXCEPTION(UnexpectedLemma)
WCHAR_T_EXCEPTION(UnexpectedPreviousCase)
}

#undef WCHAR_T_EXCEPTION
}
}

#endif // WCHAR_T_EXCEPTION_H
