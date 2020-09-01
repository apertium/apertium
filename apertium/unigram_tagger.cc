// Copyright (C) 2020 Universitat d'Alacant / Universidad de Alicante
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

#include "unigram_tagger.h"

#include "apertium_config.h"

#include "deserialiser.h"
#include "serialiser.h"
#include "lexical_unit.h"
#include "streamed_type.h"

#include <iomanip>
#include <iostream>
#include <limits>

namespace Apertium {

UnigramTagger::UnigramTagger(Flags &Flags_)
    : basic_Tagger(Flags_),
      model(UnigramTaggerModelUnknown), OccurrenceCoefficient(1) {}

UnigramTagger::~UnigramTagger() {}

UnigramTaggerModel
UnigramTagger::getModel()
{
  return model;
}

void
UnigramTagger::setModel(const UnigramTaggerModel& m)
{
  model = m;
}

void
UnigramTagger::serialise(std::ostream& o) const
{
  switch(model)
  {
    case UnigramTaggerModel1:
      ::serialise(Model1, o);
      break;
    case UnigramTaggerModel2:
      ::serialise(Model2, o);
      break;
    case UnigramTaggerModel3:
      ::serialise(Model3_l_t, o);
      ::serialise(Model3_cl_ct, o);
      ::serialise(Model3_ct_cl, o);
      break;
    default:
      throw Exception::apertium_tagger::InvalidArgument(
          "can't serialise without first selecting a model");
  }
}

void
UnigramTagger::deserialise(std::istream& s)
{
  switch(model)
  {
    case UnigramTaggerModel1:
      Model1 = Deserialiser<std::map<Analysis, std::size_t> >::deserialise(s);
      break;
    case UnigramTaggerModel2:
      Model2 = Deserialiser<std::map<a, std::map<Lemma, std::size_t> > >::deserialise(s);
      break;
    case UnigramTaggerModel3:
      Model3_l_t   = Deserialiser<std::map<i, std::map<Lemma, std::size_t> > >::deserialise(s);
      Model3_cl_ct = Deserialiser<std::map<i, std::map<Lemma, std::size_t> > >::deserialise(s);
      Model3_ct_cl = Deserialiser<std::map<Lemma, std::map<i, std::size_t> > >::deserialise(s);
      break;
    default:
      throw Exception::apertium_tagger::InvalidArgument(
          "can't read tagger without first selecting a model");
  }
}

long double
UnigramTagger::score(const Analysis& Analysis_) {
  switch(model)
  {
    case UnigramTaggerModel1:
    {
      long double s = 1;
      if(Model1.find(Analysis_) != Model1.end())
      {
        s += Model1[Analysis_];
      }
      if(TheFlags.getDebug())
      {
        score_DEBUG << s;
      }
      return s;
    }
      break;
    case UnigramTaggerModel2:
    {
      long double tokenCount_r_a = 1;
      long double tokenCount_a = 1;
      long double typeCount_a = 1;

      a a_(Analysis_);

      if(Model2.find(a_) != Model2.end())
      {
        Lemma l_(Analysis_);

        if(Model2[a_].find(l_) != Model2[a_].end())
        {
          tokenCount_r_a += Model2[a_][l_];
          typeCount_a = 0;
        }
        typeCount_a += Model2.find(a_)->second.size();

        for(auto& it : Model2[a_])
        {
          tokenCount_a += it.second;
        }
      }
      if(TheFlags.getDebug())
      {
        score_DEBUG << L"(" << tokenCount_r_a << L" * "
                    << tokenCount_a << L") /\n    ("
                    << tokenCount_a << L" + " << typeCount_a << L")";
      }
      return (tokenCount_r_a * tokenCount_a) / (tokenCount_a + typeCount_a);
    }
      break;
    case UnigramTaggerModel3:
      return model3_score(Analysis_);
      break;
    default:
      throw Exception::apertium_tagger::InvalidArgument(
          "can't score analysis without first selecting a model");
  }
}

long double
UnigramTagger::model3_score(const Analysis &Analysis_)
{
  long double tokenCount_r_i = 1;
  long double tokenCount_i = 1;
  long double typeCount_i = 1;

  i i_(Analysis_);
  Lemma l_(Analysis_);
  std::wstringstream score_DEBUG_div;
  if(Model3_l_t.find(i_) != Model3_l_t.end())
  {
    if(Model3_l_t[i_].find(l_) != Model3_l_t[i_].end())
    {
      tokenCount_r_i += Model3_l_t[i_][l_];
    }
    for(auto& Lemma_ : Model3_l_t[i_])
    {
      tokenCount_i += Lemma_.second;
    }
    typeCount_i = (1 - Model3_l_t[i_].count(l_)) + Model3_l_t[i_].size();
  }
  if(TheFlags.getDebug())
  {
    score_DEBUG << L"(" << tokenCount_r_i << L" * " << tokenCount_i;
    std::wstringstream score_DEBUG_div;
    score_DEBUG_div << L"(" << tokenCount_i << L" + " << typeCount_i << L")";
  }

  long double score = tokenCount_r_i * tokenCount_i;
  long double score_Divisor = tokenCount_i + typeCount_i;

  for(std::size_t n = 1; n < Analysis_.TheMorphemes.size(); n++)
  {
    Lemma l(Analysis_.TheMorphemes[n]);
    i i_cur(Analysis_.TheMorphemes[n]);
    i i_prev(Analysis_.TheMorphemes[n-1]);

    long double tokenCount_d_i = 1;
    long double tokenCount_i_d = 1;
    long double tokenCount_i = 1;
    long double typeCount_i = 1;
    long double tokenCount_d = 1;
    long double typeCount_d = 1;

    if(Model3_cl_ct.find(i_prev) != Model3_cl_ct.end())
    {
      if(Model3_cl_ct[i_prev].find(l) != Model3_cl_ct[i_prev].end())
      {
        tokenCount_d_i += Model3_cl_ct[i_prev][l];
      }
      for(auto& Lemma_ : Model3_cl_ct[i_prev])
      {
        tokenCount_i += Lemma_.second;
      }
      typeCount_i = (1 - Model3_cl_ct[i_prev].count(l)) +
                    Model3_cl_ct[i_prev].size();
    }
    if(Model3_ct_cl.find(l) != Model3_ct_cl.end())
    {
      if(Model3_ct_cl[l].find(i_cur) != Model3_ct_cl[l].end())
      {
        tokenCount_i_d += Model3_ct_cl[l][i_cur];
      }
      for(auto& i_ : Model3_ct_cl[l])
      {
        tokenCount_d += i_.second;
      }
      typeCount_d = (1 - Model3_ct_cl[l].count(i_cur)) +
                    Model3_ct_cl[l].size();
    }
    if(TheFlags.getDebug())
    {
      score_DEBUG << L" * " << tokenCount_d_i << L" * " << tokenCount_i_d;
      score_DEBUG_div << L" * (" << tokenCount_i << L" + " << typeCount_i
                      << L") * (" << tokenCount_d << L" + " << typeCount_d << L")";
    }

    score *= (tokenCount_d_i * tokenCount_i_d);
    score_Divisor *= ((tokenCount_i + typeCount_i) * (tokenCount_d + typeCount_d));
  }
  if(TheFlags.getDebug())
  {
    score_DEBUG << L") /\n    [" << score_DEBUG_div.str() << L"]";
  }

  return score / score_Divisor;
}

void
UnigramTagger::tag(Stream &Input, std::wostream &Output)
{
  while (true) {
    StreamedType StreamedType_ = Input.get();
    Output << StreamedType_.TheString;

    if (!StreamedType_.TheLexicalUnit) {
      if (!Input.flush_())
        break;

      Output << std::flush;
      continue;
    }
    if(TheFlags.getDebug())
    {
      std::wcerr << L"\n\n";
    }

    tag(*StreamedType_.TheLexicalUnit, Output);

    if (Input.flush_())
      Output << std::flush;
  }
}

void
UnigramTagger::tag(const LexicalUnit &LexicalUnit_, std::wostream &Output)
{
  Optional<Analysis> TheAnalysis;
  long double max_score = 0;

  for(std::size_t n = 0, lim = LexicalUnit_.TheAnalyses.size(); n < lim; n++)
  {
    if(TheFlags.getDebug())
    {
      score_DEBUG.str(L"");
    }
    const Analysis& a_ = LexicalUnit_.TheAnalyses[n];
    long double s = score(a_);
    if(n == 0 || s > max_score)
    {
      TheAnalysis = a_;
      max_score = s;
    }
    if(TheFlags.getDebug())
    {
      std::wcerr << L"score(\"" << a_ << L"\") ==\n "
                 << score_DEBUG.str() << L" ==\n  " << std::fixed
                 << std::setprecision(std::numeric_limits<long double>::digits10)
                 << s << L"\n";
    }
  }

  outputLexicalUnit(LexicalUnit_, TheAnalysis, Output);
}

void
UnigramTagger::train_Analysis(const Analysis &Analysis_, const std::size_t &Coefficient_)
{
  switch(model)
  {
    case UnigramTaggerModel1:
      Model1.insert(std::make_pair(Analysis_, 0)).first->second += Coefficient_;
      break;
    case UnigramTaggerModel2:
      Model2.insert(std::make_pair(static_cast<a>(Analysis_),
                                   std::map<Lemma, std::size_t>()))
          .first->second.insert(std::make_pair(static_cast<Lemma>(Analysis_), 0))
          .first->second += Coefficient_;
      break;
    case UnigramTaggerModel3:
      Model3_l_t.insert(
                       std::make_pair(i(Analysis_), std::map<Lemma, std::size_t>()))
          .first->second.insert(std::make_pair(Lemma(Analysis_), 0))
          .first->second += Coefficient_;

      for (std::vector<Morpheme>::const_iterator Morpheme_ =
               Analysis_.TheMorphemes.begin() + 1;
           Morpheme_ != Analysis_.TheMorphemes.end(); ++Morpheme_) {
        Model3_cl_ct.insert(std::make_pair(i(*(Morpheme_ - 1)),
                                                  std::map<Lemma, std::size_t>()))
            .first->second.insert(std::make_pair(Lemma(*Morpheme_), 0))
            .first->second += Coefficient_;
        Model3_ct_cl.insert(std::make_pair(Lemma(*Morpheme_),
                                                   std::map<i, std::size_t>()))
            .first->second.insert(std::make_pair(i(*Morpheme_), 0))
            .first->second += Coefficient_;
      }
      break;
    default:
      throw Exception::apertium_tagger::InvalidArgument(
          "can't train model without first selecting a model");
  }
}

void
UnigramTagger::multiplyModel(const std::size_t &OccurrenceCoefficientMultiplier)
{
  switch(model)
  {
    case UnigramTaggerModel1:
      for(auto& Analysis_ : Model1)
      {
        Analysis_.second *= OccurrenceCoefficientMultiplier;
      }
      break;
    case UnigramTaggerModel2:
      for(auto& a_ : Model2)
      {
        for(auto& r_ : a_.second)
        {
          r_.second *= OccurrenceCoefficientMultiplier;
        }
      }
      break;
    case UnigramTaggerModel3:
      for(auto& i_ : Model3_l_t)
      {
        for(auto& Lemma_ : i_.second)
        {
          Lemma_.second *= OccurrenceCoefficientMultiplier;
        }
      }

      for(auto& i_ : Model3_cl_ct)
      {
        for(auto& Lemma_ : i_.second)
        {
          Lemma_.second *= OccurrenceCoefficientMultiplier;
        }
      }

      for(auto& Lemma_ : Model3_ct_cl)
      {
        for(auto& i_ : Lemma_.second)
        {
          i_.second *= OccurrenceCoefficientMultiplier;
        }
      }
      break;
    default:
      throw Exception::apertium_tagger::InvalidArgument(
          "can't multiplyModel() without first selecting a model");
  }
}

void
UnigramTagger::train(Stream &TaggedCorpus) {
  while (true) {
    StreamedType StreamedType_ = TaggedCorpus.get();

    if (!StreamedType_.TheLexicalUnit)
      break;

    if (StreamedType_.TheLexicalUnit->TheAnalyses.empty())
      throw Exception::LexicalUnit::TheAnalyses_empty(
          "can't train LexicalUnit comprising empty Analysis std::vector");

    std::size_t analysis_count = StreamedType_.TheLexicalUnit->TheAnalyses.size();

    if (OccurrenceCoefficient % analysis_count != 0) {
      OccurrenceCoefficient *= analysis_count;
      multiplyModel(analysis_count);
    }

    std::size_t coefficient = OccurrenceCoefficient / analysis_count;
    for (auto& Analysis_ : StreamedType_.TheLexicalUnit->TheAnalyses) {
      train_Analysis(Analysis_, coefficient);
    }
  }
}

}
