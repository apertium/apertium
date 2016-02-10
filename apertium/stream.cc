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

#include "stream.h"

#include "analysis.h"
#include "basic_tagger.h"
#include "streamed_type.h"
#include "wchar_t_exception.h"

#include <climits>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <string>

namespace Apertium {
Stream::Stream(const basic_Tagger::Flags &Flags_)
    : TheCharacterStream(std::wcin), TheFilename(), TheLineNumber(1), TheLine(),
      TheFlags(Flags_), private_flush_(false), ThePreviousCase() {}

Stream::Stream(const basic_Tagger::Flags &Flags_,
               std::wifstream &CharacterStream_, const char *const Filename_)
    : TheCharacterStream(CharacterStream_), TheFilename(Filename_),
      TheLineNumber(1), TheLine(), TheFlags(Flags_), private_flush_(false),
      ThePreviousCase() {}

Stream::Stream(const basic_Tagger::Flags &Flags_,
               std::wifstream &CharacterStream_, const std::string &Filename_)
    : TheCharacterStream(CharacterStream_), TheFilename(Filename_),
      TheLineNumber(1), TheLine(), TheFlags(Flags_), private_flush_(false),
      ThePreviousCase() {}

Stream::Stream(const basic_Tagger::Flags &Flags_,
               std::wifstream &CharacterStream_,
               const std::stringstream &Filename_)
    : TheCharacterStream(CharacterStream_), TheFilename(Filename_.str()),
      TheLineNumber(1), TheLine(), TheFlags(Flags_), private_flush_(false),
      ThePreviousCase() {}

StreamedType Stream::get() {
  StreamedType TheStreamedType;
  std::wstring Lemma;
  private_flush_ = false;

  if (!is_eof_throw_if_not_TheCharacterStream_good()) {
    while (true) {
      const wchar_t Character_ = TheCharacterStream.get();

      if (is_eof_throw_if_not_TheCharacterStream_good(TheStreamedType, Lemma,
                                                      Character_))
        break;

      TheLine.push_back(Character_);

      switch (Character_) {
      case L'\\': // <\>  92,  Hex 5c,  Octal 134
        case_0x5c(TheStreamedType, Lemma, Character_);
        continue;
      case L'[':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case L']':
          case L'$':
            break;
          default:
            std::wstringstream Message;
            Message << L"unexpected '" << Character_ << L"' following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '[' expected to follow ']' or '$'";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case L']':
        if (!ThePreviousCase) {
          std::wstringstream Message;
          Message << L"unexpected '" << Character_
                  << L"', ']' expected to follow '['";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case L'[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        default:
          std::wstringstream Message;
          Message << L"unexpected '" << Character_ << L"' following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', ']' expected to follow '['";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        std::abort();
      case L'^':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case L'[':
            push_back_Character(TheStreamedType, Lemma, Character_);
            continue;
          case L']':
          case L'$':
            break;
          default:
            std::wstringstream Message;
            Message << L"unexpected '" << Character_ << L"' following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '^' expected to follow '[', ']', or '$'";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }
        }

        TheStreamedType.TheLexicalUnit = LexicalUnit();
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case L'/':
        if (!ThePreviousCase) {
          std::wstringstream Message;
          Message
              << L"unexpected '" << Character_
              << L"', '/' expected to follow '[', to follow '>' immediately, "
                 L"or to follow '^' or '#' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case L'[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case L'^':
          if (ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message << L"unexpected '" << Character_
                    << L"' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '/' expected to follow '[', to follow '>' "
                       L"immediately, or to follow '^' or '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          ThePreviousCase = PreviousCaseType(Character_);

          {
            const wchar_t Character_ = TheCharacterStream.get();

            if (is_eof_throw_if_not_TheCharacterStream_good(
                    TheStreamedType, Lemma, Character_)) {
              std::wstringstream Message;
              Message << L"unexpected end-of-file following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', end-of-file expected to follow ']' or '$'";
              throw wchar_t_Exception::Stream::UnexpectedEndOfFile(
                  Message_what(Message));
            }

            TheLine.push_back(Character_);

            switch (Character_) {
            case L'\\':
              TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
              TheStreamedType.TheLexicalUnit->TheAnalyses.back()
                  .TheMorphemes.push_back(Morpheme());
              case_0x5c(TheStreamedType, Lemma, Character_);
              continue;
            case L'*':
              ThePreviousCase = PreviousCaseType(Character_);
              continue;
            case L'\n': {
              std::wstringstream Message;
              Message << L"unexpected newline following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', newline expected to follow '[', ']', or '$'";
              throw wchar_t_Exception::Stream::UnexpectedCharacter(
                  Message_what(Message));
            };
            case L'[':
            case L']':
            case L'^':
            case L'#':
            case L'<':
            case L'>':
            case L'+':
            case L'$': {
              std::wstringstream Message;
              Message << L"unexpected '" << Character_
                      << L"' immediately following '"
                      << ThePreviousCase->ThePreviousCase << L"', expected '*'";
              throw wchar_t_Exception::Stream::UnexpectedPreviousCase(
                  Message_what(Message));
            }
            default:
              TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
              TheStreamedType.TheLexicalUnit->TheAnalyses.back()
                  .TheMorphemes.push_back(Morpheme());
              push_back_Character(TheStreamedType, Lemma, Character_);
              continue;
            }
          }

          continue;
        case L'>':
          if (!ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message << L"unexpected '" << Character_
                    << L"' not immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '/' expected to follow '[', to follow '>' "
                       L"immediately, or to follow '^' or '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          break;
        case L'#':
          if (ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message << L"unexpected '" << Character_
                    << L"' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '/' expected to follow '[', to follow '>' "
                       L"immediately, or to follow '^' or '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          break;
        default:
          std::wstringstream Message;
          Message << L"unexpected '" << Character_ << L"' following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', '/' expected to follow '[', to follow '>' "
                     L"immediately, or to follow '^' or '#' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
        TheStreamedType.TheLexicalUnit->TheAnalyses.back()
            .TheMorphemes.push_back(Morpheme());
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case L'*':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case L'[':
          case L']':
          case L'$':
            break;
          default:
            std::wstringstream Message;
            Message
                << L"unexpected '" << Character_ << L"' following '"
                << ThePreviousCase->ThePreviousCase
                << L"', '*' expected to follow '[', ']', or '$' or to follow "
                   L"'/' immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      case L'<':
        if (!ThePreviousCase) {
          std::wstringstream Message;
          Message
              << L"unexpected '" << Character_
              << L"', '<' expected to follow '[', to follow '>' immediately, "
                 L"or to follow '#', '/' or '+' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case L'[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case L'/':
        case L'#':
        case L'+':
          if (ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message
                << L"unexpected '" << Character_ << L"' immediately following '"
                << ThePreviousCase->ThePreviousCase
                << L"', '<' expected to follow '[', to follow '>' immediately, "
                   L"or to follow '#', '/' or '+' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          break;
        case L'>':
          if (!ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message
                << L"unexpected '" << Character_
                << L"' not immediately following '"
                << ThePreviousCase->ThePreviousCase
                << L"', '<' expected to follow '[', to follow '>' immediately, "
                   L"or to follow '#', '/' or '+' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          break;
        default:
          std::wstringstream Message;
          Message
              << L"unexpected '" << Character_ << L"' following '"
              << ThePreviousCase->ThePreviousCase
              << L"', '<' expected to follow '[', to follow '>' immediately, "
                 L"or to follow '#', '/' or '+' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        TheStreamedType.TheLexicalUnit->TheAnalyses.back()
            .TheMorphemes.back()
            .TheTags.push_back(Tag());
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case L'>':
        if (!ThePreviousCase) {
          std::wstringstream Message;
          Message << L"unexpected '" << Character_
                  << L"', '>' expected to "
                     L"follow '[' or to follow "
                     L"'<' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case L'[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case L'<':
          if (ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message << L"unexpected '" << Character_
                    << L"' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '>' expected to "
                       L"follow '[' or to follow "
                       L"'<' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        default:
          std::wstringstream Message;
          Message << L"unexpected '" << Character_ << L"' following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', '>' expected to "
                     L"follow '[' or to follow "
                     L"'<' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        std::abort();
      case L'#':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case L'[':
          case L']':
          case L'$':
            push_back_Character(TheStreamedType, Lemma, Character_);
            continue;
          case L'/':
            if (ThePreviousCase->isPreviousCharacter) {
              std::wstringstream Message;
              Message
                  << L"unexpected '" << Character_
                  << L"' immediately following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', '#' expected to follow '[', ']', or '$', to follow "
                     L"'>' immediately, or to follow '/' not immediately";
              throw wchar_t_Exception::Stream::UnexpectedCase(
                  Message_what(Message));
            }

            break;
          case L'>':
            if (!ThePreviousCase->isPreviousCharacter) {
              std::wstringstream Message;
              Message
                  << L"unexpected '" << Character_
                  << L"' not immediately following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', '#' expected to follow '[', ']', or '$', to follow "
                     L"'>' immediately, or to follow '/' not immediately";
              throw wchar_t_Exception::Stream::UnexpectedCase(
                  Message_what(Message));
            }

            break;
          default:
            std::wstringstream Message;
            Message << L"unexpected '" << Character_ << L"' following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '#' expected to follow '[', ']', or '$', to follow "
                       L"'>' immediately, or to follow '/' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      case L'+':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case L'[':
          case L']':
          case L'$':
            push_back_Character(TheStreamedType, Lemma, Character_);
            continue;
          case L'>':
            if (!ThePreviousCase->isPreviousCharacter) {
              std::wstringstream Message;
              Message
                  << L"unexpected '" << Character_
                  << L"' not immediately following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', '+' expected to follow '[', ']', or '$', to follow "
                     L"'>' "
                     L"immediately, or to follow '#' not immediately";
              throw wchar_t_Exception::Stream::UnexpectedCase(
                  Message_what(Message));
            }

            break;
          case L'#':
            if (ThePreviousCase->isPreviousCharacter) {
              std::wstringstream Message;
              Message
                  << L"unexpected '" << Character_
                  << L"' immediately following '"
                  << ThePreviousCase->ThePreviousCase
                  << L"', '+' expected to follow '[', ']', or '$', to follow "
                     L"'>' "
                     L"immediately, or to follow '#' not immediately";
              throw wchar_t_Exception::Stream::UnexpectedCase(
                  Message_what(Message));
            }

            break;
          default: {
            std::wstringstream Message;
            Message << L"unexpected '" << Character_ << L"' following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', '+' expected to follow '[', ']', or '$', to follow "
                       L"'>' immediately, or to follow '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }
          }

          TheStreamedType.TheLexicalUnit->TheAnalyses.back()
              .TheMorphemes.push_back(Morpheme());
          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      case L'$':
        if (!ThePreviousCase) {
          std::wstringstream Message;
          Message
              << L"unexpected '" << Character_
              << L"', '$' expected to follow '[', to follow '>' immediately, "
                 L"or to follow '*' or '#' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case L'[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case L'*':
          if (ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message
                << L"unexpected '" << Character_ << L"' immediately following '"
                << ThePreviousCase->ThePreviousCase
                << L"', '$' expected to follow '[', to follow '>' immediately, "
                   L"or to follow '*' or '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          if (TheFlags.getDebug()) {
            if (Lemma != TheStreamedType.TheLexicalUnit->TheSurfaceForm)
              std::wcerr << L"unexpected lemma \"" << Lemma
                         << L"\", expected \""
                         << TheStreamedType.TheLexicalUnit->TheSurfaceForm
                         << L"\"\n";
          }

          ThePreviousCase = PreviousCaseType(Character_);
          return TheStreamedType;
        case L'>':
          if (!ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message
                << L"unexpected '" << Character_
                << L"' not immediately following '"
                << ThePreviousCase->ThePreviousCase
                << L"', '$' expected to follow '[', to follow '>' immediately, "
                   L"or to follow '*' or '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          break;
        case L'#':
          if (ThePreviousCase->isPreviousCharacter) {
            std::wstringstream Message;
            Message
                << L"unexpected '" << Character_ << L"' immediately following '"
                << ThePreviousCase->ThePreviousCase
                << L"', '$' expected to follow '[', to follow '>' immediately, "
                   L"or to follow '*' or '#' not immediately";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }

          break;
        default:
          std::wstringstream Message;
          Message
              << L"unexpected '" << Character_ << L"' following '"
              << ThePreviousCase->ThePreviousCase
              << L"', '$' expected to follow '[', to follow '>' immediately, "
                 L"or to follow '*' or '#' not immediately";
          throw wchar_t_Exception::Stream::UnexpectedCase(
              Message_what(Message));
        }

        ThePreviousCase = PreviousCaseType(Character_);
        return TheStreamedType;
      case L'\n':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case L'[':
          case L']':
          case L'$':
            break;
          default:
            std::wstringstream Message;
            Message << L"unexpected newline following '"
                    << ThePreviousCase->ThePreviousCase
                    << L"', newline expected to follow '[', ']', or '$'";
            throw wchar_t_Exception::Stream::UnexpectedCase(
                Message_what(Message));
          }
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        ++TheLineNumber;
        TheLine.clear();
        continue;
      default:
        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      }

      std::abort();
    }
  }

  if (ThePreviousCase) {
    switch (ThePreviousCase->ThePreviousCase) {
    case L']':
    case L'$':
      break;
    default:
      std::wstringstream Message;
      Message << L"unexpected end-of-file following '"
              << ThePreviousCase->ThePreviousCase
              << L"', end-of-file expected to follow ']' "
                 L"or '$'";
      throw wchar_t_Exception::Stream::UnexpectedEndOfFile(
          Message_what(Message));
    }
  }

  return TheStreamedType;
}

bool Stream::flush_() const { return private_flush_; }

Stream::PreviousCaseType::PreviousCaseType(const wchar_t &PreviousCase_)
    : ThePreviousCase(PreviousCase_), isPreviousCharacter(true) {}

bool Stream::is_eof_throw_if_not_TheCharacterStream_good() const {
  if (TheCharacterStream.eof())
    return true;

  if (!TheCharacterStream) {
    std::wstringstream Message;
    Message << L"can't get const wchar_t: TheCharacterStream not good";
    throw wchar_t_Exception::Stream::TheCharacterStream_not_good(
        Message_what(Message));
  }

  return false;
}

std::wstring Stream::Message_what(const std::wstringstream &Message) const {
  std::wstringstream what_;

  if (TheFilename)
    what_ << std::wstring(TheFilename->begin(), TheFilename->end()) << L": ";

  what_ << TheLineNumber << L":" << TheLine.size() << L": " << Message.str()
        << L'\n' << TheLine << L'\n' << std::wstring(TheLine.size() - 1, L' ')
        << L'^';
  return what_.str();
}

bool
Stream::is_eof_throw_if_not_TheCharacterStream_good(StreamedType &StreamedType_,
                                                    std::wstring &Lemma,
                                                    const wchar_t &Character_) {
  if (isTheCharacterStream_eof(StreamedType_, Lemma, Character_))
    return true;

  if (!TheCharacterStream) {
    std::wstringstream Message;
    Message << L"can't get const wchar_t: TheCharacterStream not good";
    throw wchar_t_Exception::Stream::TheCharacterStream_not_good(
        Message_what(Message));
  }

  return false;
}

bool Stream::isTheCharacterStream_eof(StreamedType &StreamedType_,
                                      std::wstring &Lemma,
                                      const wchar_t &Character_) {
  if (TheCharacterStream.eof())
    return true;

  if (TheFlags.getNullFlush()) {
    if (Character_ == L'\0') {
      push_back_Character(StreamedType_, Lemma, Character_);
      private_flush_ = true;
      return true;
    }
  }

  return false;
}

void Stream::push_back_Character(StreamedType &StreamedType_,
                                 std::wstring &Lemma,
                                 const wchar_t &Character_) {
  if (ThePreviousCase) {
    switch (ThePreviousCase->ThePreviousCase) {
    case L'[':
      StreamedType_.TheString += Character_;
      break;
    case L']':
      StreamedType_.TheString += Character_;
      break;
    case L'^':
      StreamedType_.TheLexicalUnit->TheSurfaceForm += Character_;
      break;
    case L'/':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case L'*':
      Lemma += Character_;
      break;
    case L'<':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheTags.back()
          .TheTag += Character_;
      break;
    case L'>': {
      std::wstringstream Message;
      Message << L"unexpected '" << Character_ << L"' immediately following '"
              << ThePreviousCase->ThePreviousCase << L"'";
      throw wchar_t_Exception::Stream::UnexpectedCharacter(
          Message_what(Message));
    }
    case L'#':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case L'+':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case L'$':
      StreamedType_.TheString += Character_;
      break;
    default:
      std::wstringstream Message;
      Message << L"unexpected previous reserved or special character '"
              << ThePreviousCase->ThePreviousCase << L"'";
      throw wchar_t_Exception::Stream::UnexpectedPreviousCase(
          Message_what(Message));
    }

    ThePreviousCase->isPreviousCharacter = false;
    return;
  }

  StreamedType_.TheString += Character_;
}

void Stream::case_0x5c(StreamedType &StreamedType_, std::wstring &Lemma,
                       const wchar_t &Character_) {
  push_back_Character(StreamedType_, Lemma, Character_);

  {
    const wchar_t Character_ = TheCharacterStream.get();

    if (is_eof_throw_if_not_TheCharacterStream_good(StreamedType_, Lemma,
                                                    Character_)) {
      std::wstringstream Message;
      Message << L"unexpected end-of-file following '\\', end-of-file "
                 L"expected to follow ']' or '$'";
      throw wchar_t_Exception::Stream::UnexpectedEndOfFile(
          Message_what(Message));
    }

    TheLine.push_back(Character_);
    push_back_Character(StreamedType_, Lemma, Character_);
  }
}
}
