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

#include "stream.h"

#include "analysis.h"
#include "exception.h"

#include <climits>
#include <cstdlib>
#include <fstream>
#include <iostream>

namespace Apertium {
Stream::Stream(TaggerFlags &Flags_)
    : TheLineNumber(1), TheCharacterStream(std::cin), TheFilename(), TheLine(),
      TheFlags(Flags_), private_flush_(false), ThePreviousCase() {}

Stream::Stream(TaggerFlags &Flags_,
               std::ifstream &CharacterStream_, const char *const Filename_)
    : TheLineNumber(1), TheCharacterStream(CharacterStream_), TheFilename(Filename_),
      TheLine(), TheFlags(Flags_), private_flush_(false),
      ThePreviousCase() {}

Stream::Stream(TaggerFlags &Flags_,
               std::ifstream &CharacterStream_, const std::string &Filename_)
    : TheLineNumber(1), TheCharacterStream(CharacterStream_), TheFilename(Filename_),
      TheLine(), TheFlags(Flags_), private_flush_(false),
      ThePreviousCase() {}

Stream::Stream(TaggerFlags &Flags_,
               std::ifstream &CharacterStream_,
               const std::stringstream &Filename_)
    : TheLineNumber(1), TheCharacterStream(CharacterStream_), TheFilename(Filename_.str()),
      TheLine(), TheFlags(Flags_), private_flush_(false),
      ThePreviousCase() {}

StreamedType Stream::get() {
  StreamedType TheStreamedType;
  UString Lemma;
  private_flush_ = false;

  //TheCharacterStream.clear();
  if (!is_eof_throw_if_not_TheCharacterStream_good()) {
    while (true) {
      const UChar Character_ = TheCharacterStream.get();

      if (is_eof_throw_if_not_TheCharacterStream_good(TheStreamedType, Lemma,
                                                      Character_))
        break;

      TheLine.push_back(Character_);

      switch (Character_) {
      case '\\': // <\>  92,  Hex 5c,  Octal 134
        case_0x5c(TheStreamedType, Lemma, Character_);
        continue;
      case '[':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case '[':
          case ']':
          case '$':
            break;
          default:
            std::stringstream Message;
            Message << "unexpected '" << Character_ << "' following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '[' expected to follow '[', ']' or '$'";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case ']':
        if (!ThePreviousCase) {
          std::stringstream Message;
          Message << "unexpected '" << Character_
                  << "', ']' expected to follow '['";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case '[':
        case ']':
          push_back_Character(TheStreamedType, Lemma, Character_);
          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        default:
          std::stringstream Message;
          Message << "unexpected '" << Character_ << "' following '"
                  << ThePreviousCase->ThePreviousCase
                  << "', ']' expected to follow '[' or ']'";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        std::abort();
      case '^':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case '[':
            push_back_Character(TheStreamedType, Lemma, Character_);
            continue;
          case ']':
          case '$':
            break;
          default:
            std::stringstream Message;
            Message << "unexpected '" << Character_ << "' following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '^' expected to follow '[', ']', or '$'";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }
        }

        TheStreamedType.TheLexicalUnit = LexicalUnit();
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case '/':
        if (!ThePreviousCase) {
          std::stringstream Message;
          Message << "unexpected '" << Character_
                  << "', '/' expected to follow '[', to follow '>' "
                     "immediately, or to follow '^' or '#' not immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case '[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case '^':
          if (ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '/' expected to follow '[', to follow '>' "
                       "immediately, or to follow '^' or '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          ThePreviousCase = PreviousCaseType(Character_);

          {
            const UChar Character_ = TheCharacterStream.get();

            if (is_eof_throw_if_not_TheCharacterStream_good(
                    TheStreamedType, Lemma, Character_)) {
              std::stringstream Message;
              Message << "unexpected end-of-file following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', end-of-file expected to follow ']' or '$'";
              throw Exception::Stream::UnexpectedEndOfFile(
                  Message_what(Message));
            }

            TheLine.push_back(Character_);

            switch (Character_) {
            case '\\':
              TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
              TheStreamedType.TheLexicalUnit->TheAnalyses.back()
                  .TheMorphemes.push_back(Morpheme());
              case_0x5c(TheStreamedType, Lemma, Character_);
              continue;
            case '*':
              ThePreviousCase = PreviousCaseType(Character_);
              continue;
            case '\n': {
              std::stringstream Message;
              Message << "unexpected newline following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', newline expected to follow '[', ']', or '$'";
              throw Exception::Stream::UnexpectedCharacter(
                  Message_what(Message));
            };
            case '<':
              TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
              TheStreamedType.TheLexicalUnit->TheAnalyses.back()
                .TheMorphemes.push_back(Morpheme());
              TheStreamedType.TheLexicalUnit->TheAnalyses.back()
                .TheMorphemes.back()
                .TheTags.push_back(Tag());
              ThePreviousCase = PreviousCaseType(Character_);
              continue;

            case '[':
            case ']':
            case '^':
            case '#':
            case '>':
            case '+':
            case '$': {
              std::stringstream Message;
              Message << "unexpected '" << Character_
                      << "' immediately following '"
                      << ThePreviousCase->ThePreviousCase << "', expected '*'";
              throw Exception::Stream::UnexpectedPreviousCase(
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
        case '>':
          if (!ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' not immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '/' expected to follow '[', to follow '>' "
                       "immediately, or to follow '^' or '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          break;
        case '#':

          if (ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '/' expected to follow '[', to follow '>' "
                       "immediately, or to follow '^' or '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          break;
        default:
          std::stringstream Message;
          Message << "unexpected '" << Character_ << "' following '"
                  << ThePreviousCase->ThePreviousCase
                  << "', '/' expected to follow '[', to follow '>' "
                     "immediately, or to follow '^' or '#' not immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        TheStreamedType.TheLexicalUnit->TheAnalyses.push_back(Analysis());
        TheStreamedType.TheLexicalUnit->TheAnalyses.back()
            .TheMorphemes.push_back(Morpheme());
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case '*':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case '[':
          case ']':
          case '$':
            break;
          default:
            std::stringstream Message;
            Message << "unexpected '" << Character_ << "' following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '*' expected to follow '[', ']', or '$' or to "
                       "follow '/' immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      case '<':
        if (!ThePreviousCase) {
          std::stringstream Message;
          Message << "unexpected '" << Character_
                  << "', '<' expected to follow '[', to follow '>' "
                     "immediately, or to follow '#', '/' or '+' not "
                     "immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case '[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case '/':
          break;
        case '#':
          //std::cerr << "[306] Character: " << Character_ << "||| Lemma: " << Lemma << std::endl ;
        case '+':
          if (ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '<' expected to follow '[', '/', '>'"
                       "immediately, or to follow '#' or '+' not "
                       "immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          break;
        case '>':
          break;
        default:
          std::stringstream Message;
          Message << "unexpected '" << Character_ << "' following '"
                  << ThePreviousCase->ThePreviousCase
                  << "', '<' expected to follow '[', to follow '>' "
                     "immediately, or to follow '#', '/' or '+' not "
                     "immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        TheStreamedType.TheLexicalUnit->TheAnalyses.back()
            .TheMorphemes.back()
            .TheTags.push_back(Tag());
        ThePreviousCase = PreviousCaseType(Character_);
        continue;
      case '>':
        if (!ThePreviousCase) {
          std::stringstream Message;
          Message << "unexpected '" << Character_
                  << "', '>' expected to follow '[' or to follow '<' not "
                     "immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case '[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case '<':
          if (ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '>' expected to follow '[' or to follow '<' not "
                       "immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        default:
          std::stringstream Message;
          Message << "unexpected '" << Character_ << "' following '"
                  << ThePreviousCase->ThePreviousCase
                  << "', '>' expected to follow '[' or to follow '<' not "
                     "immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        std::abort();
      case '#':
        //std::cerr << "[391] Character: " << Character_ << "||| Lemma: " << Lemma << std::endl ;
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case '[':
          case ']':
          case '^':
          case '$':
            push_back_Character(TheStreamedType, Lemma, Character_);
            continue;
          case '/':
            if (ThePreviousCase->isPreviousCharacter) {
              std::stringstream Message;
              Message << "unexpected '" << Character_
                      << "' immediately following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', '#' expected to follow '[', ']', or '$', to "
                         "follow '>' immediately, or to follow '/' not "
                         "immediately";
              throw Exception::Stream::UnexpectedCase(Message_what(Message));
            }

            break;
          case '>':
            if (!ThePreviousCase->isPreviousCharacter) {
              std::stringstream Message;
              Message << "unexpected '" << Character_
                      << "' not immediately following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', '#' expected to follow '[', ']', or '$', to "
                         "follow '>' immediately, or to follow '/' not "
                         "immediately";
              throw Exception::Stream::UnexpectedCase(Message_what(Message));
            }

            break;
          default:
            std::stringstream Message;
            Message << "unexpected '" << Character_ << "' following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '#' expected to follow '[', ']', or '$', to follow "
                       "'>' immediately, or to follow '/' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          ThePreviousCase = PreviousCaseType(Character_);
          push_back_Character(TheStreamedType, Lemma, Character_);
          //std::cerr << "[440] Character: " << Character_ << "||| Lemma: " << Lemma << std::endl ;
          continue;
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      case '+':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case '[':
          case ']':
          case '^':
          case '/':
          case '$':
            push_back_Character(TheStreamedType, Lemma, Character_);
            continue;
          case '>':
            if (!ThePreviousCase->isPreviousCharacter) {
              std::stringstream Message;
              Message << "unexpected '" << Character_
                      << "' not immediately following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', '+' expected to follow '[', ']', '^', '/' or "
                         "'$', to follow '>' immediately, or to follow '#' "
                         "not immediately";
              throw Exception::Stream::UnexpectedCase(Message_what(Message));
            }

            break;
          case '#':
            if (ThePreviousCase->isPreviousCharacter) {
              std::stringstream Message;
              Message << "unexpected '" << Character_
                      << "' immediately following '"
                      << ThePreviousCase->ThePreviousCase
                      << "', '+' expected to follow '[', ']', or '$', to "
                         "follow '>' immediately, or to follow '#' not "
                         "immediately";
              throw Exception::Stream::UnexpectedCase(Message_what(Message));
            }

            break;
          default: {
            std::stringstream Message;
            Message << "unexpected '" << Character_ << "' following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '+' expected to follow '[', ']', or '$', to follow "
                       "'>' immediately, or to follow '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }
          }

          TheStreamedType.TheLexicalUnit->TheAnalyses.back()
              .TheMorphemes.push_back(Morpheme());
          ThePreviousCase = PreviousCaseType(Character_);
          continue;
        }

        push_back_Character(TheStreamedType, Lemma, Character_);
        continue;
      case '$':
        if (!ThePreviousCase) {
          std::stringstream Message;
          Message << "unexpected '" << Character_
                  << "', '$' expected to follow '[', to follow '>' "
                     "immediately, or to follow '*' or '#' not immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        switch (ThePreviousCase->ThePreviousCase) {
        case '[':
          push_back_Character(TheStreamedType, Lemma, Character_);
          continue;
        case '*':
          if (ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '$' expected to follow '[', to follow '>' "
                       "immediately, or to follow '*' or '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          if (TheFlags.getDebug()) {
            if (Lemma != TheStreamedType.TheLexicalUnit->TheSurfaceForm)
              std::cerr << "unexpected lemma \"" << Lemma
                         << "\", expected \""
                         << TheStreamedType.TheLexicalUnit->TheSurfaceForm
                         << "\"\n";
          }

          ThePreviousCase = PreviousCaseType(Character_);
          return TheStreamedType;
        case '>':
          if (!ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' not immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '$' expected to follow '[', to follow '>' "
                       "immediately, or to follow '*' or '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          break;
        case '#':
          if (ThePreviousCase->isPreviousCharacter) {
            std::stringstream Message;
            Message << "unexpected '" << Character_
                    << "' immediately following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', '$' expected to follow '[', to follow '>' "
                       "immediately, or to follow '*' or '#' not immediately";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
          }

          break;
        default:
          std::stringstream Message;
          Message << "unexpected '" << Character_ << "' following '"
                  << ThePreviousCase->ThePreviousCase
                  << "', '$' expected to follow '[', to follow '>' "
                     "immediately, or to follow '*' or '#' not immediately";
          throw Exception::Stream::UnexpectedCase(Message_what(Message));
        }

        ThePreviousCase = PreviousCaseType(Character_);
        return TheStreamedType;
      case '\n':
        if (ThePreviousCase) {
          switch (ThePreviousCase->ThePreviousCase) {
          case '[':
          case ']':
          case '$':
            break;
          default:
            std::stringstream Message;
            Message << "unexpected newline following '"
                    << ThePreviousCase->ThePreviousCase
                    << "', newline expected to follow '[', ']', or '$'";
            throw Exception::Stream::UnexpectedCase(Message_what(Message));
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
    case ']':
    case '$':
      break;
    default:
      std::stringstream Message;
      Message << "unexpected end-of-file following '"
              << ThePreviousCase->ThePreviousCase
              << "', end-of-file expected to follow ']' or '$'";
      throw Exception::Stream::UnexpectedEndOfFile(Message_what(Message));
    }
  }

  return TheStreamedType;
}

StreamedType Stream::peek() {
  bool prev_flush = private_flush_;
  std::ios::iostate state = TheCharacterStream.rdstate();
  int pos = TheCharacterStream.tellg();

  StreamedType token = get();

  TheCharacterStream.clear(state);
  TheCharacterStream.seekg(pos);
  private_flush_ = prev_flush;
  return token;
}

bool Stream::peekIsBlank() {
  std::ios::iostate state = TheCharacterStream.rdstate();
  int pos = TheCharacterStream.tellg();

  const UChar newline1 = TheCharacterStream.get();
  const UChar newline2 = TheCharacterStream.get();

  TheCharacterStream.clear(state);
  TheCharacterStream.seekg(pos);

  return newline1 == '\n' && newline2 == '\n';
}

bool Stream::flush_() const { return private_flush_; }

void Stream::outputLexicalUnit(
    const LexicalUnit &lexical_unit, const Optional<Analysis> analysis,
    std::ostream &output, TaggerFlags &flags) {
  using namespace std::rel_ops;
  output << "^";

  if (lexical_unit.TheAnalyses.empty() || !analysis) {
    if (flags.getShowSuperficial())
      output << lexical_unit.TheSurfaceForm << "/";

    output << "*" << lexical_unit.TheSurfaceForm << "$";
    return;
  }

  if (flags.getMark()) {
    if (lexical_unit.TheAnalyses.size() != 1)
      output << "=";
  }

  if (flags.getShowSuperficial())
    output << lexical_unit.TheSurfaceForm << "/";

  output << *analysis;

  if (flags.getFirst()) {
    for (std::vector<Analysis>::const_iterator other_analysis =
             lexical_unit.TheAnalyses.begin();
         // Call .end() each iteration to save memory.
         other_analysis != lexical_unit.TheAnalyses.end(); ++other_analysis) {
      if (*other_analysis != *analysis)
        output << "/" << *other_analysis;
    }
  }

  output << "$";
}

Stream::PreviousCaseType::PreviousCaseType(const UChar &PreviousCase_)
    : ThePreviousCase(PreviousCase_), isPreviousCharacter(true) {}

bool Stream::is_eof_throw_if_not_TheCharacterStream_good() const {
  if (TheCharacterStream.eof())
    return true;

  if (!TheCharacterStream) {
    std::cerr << "State bad " << TheCharacterStream.good() << " "
                                << TheCharacterStream.eof() << " "
                                << TheCharacterStream.fail() << " "
                                << TheCharacterStream.bad() << "\n";
    std::stringstream Message;
    Message << "can't get const UChar: TheCharacterStream not good";
    throw Exception::Stream::TheCharacterStream_not_good(
        Message_what(Message));
  }

  return false;
}

UString Stream::Message_what(const std::stringstream &Message) const {
  std::stringstream what_;

  if (TheFilename)
    what_ << UString(TheFilename->begin(), TheFilename->end()) << ": ";

  what_ << TheLineNumber << ":" << TheLine.size() << ": " << Message.str()
        << '\n' << TheLine << '\n' << UString(TheLine.size() - 1, ' ')
        << '^';
  return to_ustring(what_.str().c_str());
}

bool
Stream::is_eof_throw_if_not_TheCharacterStream_good(StreamedType &StreamedType_,
                                                    UString &Lemma,
                                                    const UChar &Character_) {
  if (isTheCharacterStream_eof(StreamedType_, Lemma, Character_))
    return true;

  if (!TheCharacterStream) {
    std::stringstream Message;
    Message << "can't get const UChar: TheCharacterStream not good";
    throw Exception::Stream::TheCharacterStream_not_good(
        Message_what(Message));
  }

  return false;
}

bool Stream::isTheCharacterStream_eof(StreamedType &StreamedType_,
                                      UString &Lemma,
                                      const UChar &Character_) {
  if (TheCharacterStream.eof())
    return true;

  if (TheFlags.getNullFlush()) {
    if (Character_ == '\0') {
      push_back_Character(StreamedType_, Lemma, Character_);
      private_flush_ = true;
      return true;
    }
  }

  return false;
}

void Stream::push_back_Character(StreamedType &StreamedType_,
                                 UString &Lemma,
                                 const UChar &Character_) {
  if (ThePreviousCase) {
    switch (ThePreviousCase->ThePreviousCase) {
    case '[':
      StreamedType_.TheString += Character_;
      break;
    case ']':
      StreamedType_.TheString += Character_;
      break;
    case '^':
      StreamedType_.TheLexicalUnit->TheSurfaceForm += Character_;
      break;
    case '/':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case '*':
      Lemma += Character_;
      break;
    case '<':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheTags.back()
          .TheTag += Character_;
      break;
    case '>':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case '#':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case '+':
      StreamedType_.TheLexicalUnit->TheAnalyses.back()
          .TheMorphemes.back()
          .TheLemma.push_back(Character_);
      break;
    case '$':
      StreamedType_.TheString += Character_;
      break;
    default:
      std::stringstream Message;
      Message << "unexpected previous reserved or special character '"
              << ThePreviousCase->ThePreviousCase << "'";
      throw Exception::Stream::UnexpectedPreviousCase(Message_what(Message));
    }

    ThePreviousCase->isPreviousCharacter = false;
    return;
  }

  StreamedType_.TheString += Character_;
}

void Stream::case_0x5c(StreamedType &StreamedType_, UString &Lemma,
                       const UChar &Character_) {
  push_back_Character(StreamedType_, Lemma, Character_);

  {
    const UChar Character_ = TheCharacterStream.get();

    if (is_eof_throw_if_not_TheCharacterStream_good(StreamedType_, Lemma,
                                                    Character_)) {
      std::stringstream Message;
      Message << "unexpected end-of-file following '\\', end-of-file "
                 "expected to follow ']' or '$'";
      throw Exception::Stream::UnexpectedEndOfFile(Message_what(Message));
    }

    TheLine.push_back(Character_);
    push_back_Character(StreamedType_, Lemma, Character_);
  }
}
}
