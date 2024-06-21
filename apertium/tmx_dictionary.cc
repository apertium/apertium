/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*  From hunalign; for license see ../AUTHORS and ../COPYING.hunalign     *
*                                                                        *
*************************************************************************/
#include <apertium/tmx_dictionary.h>

#include <apertium/tmx_serialize_impl.h>
#include <apertium/tmx_strings_and_streams.h>

#include <fstream>
#include <iostream>
#include <set>
#include <cassert>
#include <sstream>

#include <cmath>
#include <lttoolbox/i18n.h>

#define massert(e) if (!(e)) { std::cerr << #e << " failed" << std::endl; throw "assert"; }

namespace TMXAligner
{

void eatwhite( std::istream& is )
{
  while (true)
  {
    char c=is.peek();
    if ( (c!=' ') && (c!='\t') )
    {
      break;
    }
    is.ignore();
    if (is.eof())
      break;
  }
}

void read( WordList& ph, std::istream& is )
{
  ph.clear();

  while (true)
  {
    if (is.eof())
    {
      break;
    }
    if (is.peek()=='\r')
    {
      is.ignore();
    }
    if (is.peek()=='\n')
    {
      is.ignore();
      break;
    }

    Word w;
    is >> w;

    eatwhite(is);

    if (w.empty())
      break;

    ph.push_back(w);
  }
}

void SentenceList::read( std::istream& is )
{
  clear();

  while (!is.eof())
  {
    Sentence sentence;

    is >> sentence.id;

    if (sentence.id.empty())
      break;

    if (is.peek()!='\t')
      break;
    is.ignore();

    TMXAligner::read( sentence.words, is );

    push_back(sentence);
  }
}

void SentenceList::readNoIds( std::istream& is )
{
  clear();

  while ( (is.good()) && (!is.eof()) )
  {
    Sentence sentence;

    TMXAligner::read( sentence.words, is );

    push_back(sentence);
  }
}

void SentenceList::write( std::ostream& os ) const
{
  for ( size_t i=0; i<size(); ++i )
  {
    const Sentence& sentence = operator[](i);
    os << sentence.id << "\t" << sentence.words << "\n";
  }
  os.flush();
}

void SentenceList::writeNoIds( std::ostream& os ) const
{
  for ( size_t i=0; i<size(); ++i )
  {
    const Sentence& sentence = operator[](i);
    os << sentence.words << "\n";
  }
  os.flush();
}

void readBicorpus( std::istream& is, SentenceList& huSentenceList, SentenceList& enSentenceList)
{
  huSentenceList.clear();
  enSentenceList.clear();

  while ( (is.good()) && (!is.eof()) )
  {
    std::string line;

    std::vector<std::string> halfs;
    std::getline(is,line,'\n');

    if (line.empty())
    {
      break;
    }

    split( line, halfs );
    if (halfs.size()!=2)
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81330", {"records", "line"},
        {std::to_string(halfs.size()).c_str(), std::to_string(huSentenceList.size()).c_str()}, false);
      throw I18n(APR_I18N_DATA, "apertium").format("APR81220");
    }

    {
      std::istringstream iss(halfs[0]);

      Sentence sentence;
      read( sentence.words, iss );

      huSentenceList.push_back(sentence);
    }
    {
      std::istringstream iss(halfs[1]);

      Sentence sentence;
      read( sentence.words, iss );

      enSentenceList.push_back(sentence);
    }
  }
}

void writeBicorpus( std::ostream& os, const SentenceList& huSentenceList, const SentenceList& enSentenceList)
{
  assert(huSentenceList.size()==enSentenceList.size());

  for ( size_t i=0; i<huSentenceList.size(); ++i )
  {
    os << huSentenceList[i].words << "\t" << enSentenceList[i].words << "\n";
  }
  os.flush();
}

void HalfDictionary::read( std::istream& is )
{
  clear();

  while (!is.eof())
  {
    WordList ph;
    TMXAligner::read(ph,is);

    if (ph.empty())
      continue;

    push_back(ph);
  }
}

void DictionaryItems::read( std::istream& is )
{
  clear();

  while (!is.eof())
  {
    WordList hu;
    WordList en;
    Word delimiter;

    bool engPart = true;

    while (true)
    {
      Word w;
      is >> w;

      if (w.empty())
        break;

      // We allow vonyo7's "@" delimiter, and vonyokornai's "@V", "@N" etc. delimiters.
      if ( (w.size()<=2) && (w[0]=='@') )
      {
        engPart = false;
        delimiter = w;
      }
      else if (engPart)
      {
        en.push_back(w);
      }
      else
      {
        hu.push_back(w);
      }

      while ( (is.peek()==' ') || (is.peek()=='\r') )
      {
        is.ignore();
      }

      if (is.peek()=='\n')
      {
        is.ignore();
        break;
      }
    }

    if (en.empty())
      break;

    push_back(std::make_pair(en,hu));

  }

}


void Dictionary::read( const char* dictionaryFile )
{
  throw I18n(APR_I18N_DATA, "apertium").format("APR81230");
}

void Dictionary::build( const DictionaryItems& dictionaryItems )
{
  throw I18n(APR_I18N_DATA, "apertium").format("APR81230");
}

void Dictionary::reverse( const Dictionary& dic )
{
  throw I18n(APR_I18N_DATA, "apertium").format("APR81230");
}

bool Dictionary::lookupWord( const Word& word, DictionaryItems& results ) const
{
  return false;
}

bool Dictionary::lookupWordSet( const WordList& words, DictionaryItems& results ) const
{
  return false;
}

void FrequencyMap::add( const Word& word )
{
  ++operator[](word);
}

void FrequencyMap::remove( const Word& word )
{
  --operator[](word);
}

void FrequencyMap::build( const WordList& wordList )
{
  for ( size_t j=0; j<wordList.size(); ++j )
  {
    add(wordList[j]);
  }
}

void FrequencyMap::remove( const WordList& wordList )
{
  for ( size_t j=0; j<wordList.size(); ++j )
  {
    remove(wordList[j]);
  }
}

void FrequencyMap::build( const SentenceList& sentenceList )
{
  for ( size_t i=0; i<sentenceList.size(); ++i )
  {
    for ( size_t j=0; j<sentenceList[i].words.size(); ++j )
    {
      add(sentenceList[i].words[j]);
    }
  }
}

int FrequencyMap::total() const
{
  const_iterator it;

  int totalItemNum(0);
  for ( it=begin(); it!=end(); ++it )
  {
    totalItemNum += it->second;
  }
  return totalItemNum;
}

void FrequencyMap::dump( std::ostream& os, int itemNum ) const
{
  FrequencyMap::ReFrequencyMap reFrequencyMap;
  reverseMap(reFrequencyMap);

  FrequencyMap::ReFrequencyMap::reverse_iterator rit;
  for ( rit=reFrequencyMap.rbegin(); rit!=reFrequencyMap.rend(); ++rit )
  {
    os << rit->first << "\t" << rit->second << "\n";

    --itemNum;
    if (itemNum==0)
      break;
  }
  os.flush();
}

void FrequencyMap::highPassFilter( WordList& allowedWords, double ratio ) const
{
  allowedWords.clear();

  FrequencyMap::ReFrequencyMap reFrequencyMap;
  reverseMap(reFrequencyMap);

  FrequencyMap::ReFrequencyMap::reverse_iterator rit;

  int totalItemNum = total();

  int localItemNum(0);
  for ( rit=reFrequencyMap.rbegin(); rit!=reFrequencyMap.rend(); ++rit )
  {
    localItemNum += rit->first;
    if ( ((double)localItemNum)/totalItemNum > ratio )
      break;

    allowedWords.push_back(rit->second);
  }
}

void FrequencyMap::lowPassFilter( WordList& allowedWords, double ratio ) const
{
  allowedWords.clear();

  FrequencyMap::ReFrequencyMap reFrequencyMap;
  reverseMap(reFrequencyMap);

  FrequencyMap::ReFrequencyMap::iterator rit;

  int totalItemNum = total();

  int localItemNum(0);
  for ( rit=reFrequencyMap.begin(); rit!=reFrequencyMap.end(); ++rit )
  {
    localItemNum += rit->first;

    if ( ((double)localItemNum)/totalItemNum > ratio )
      break;

    allowedWords.push_back(rit->second);
  }
}

void FrequencyMap::reverseMap( FrequencyMap::ReFrequencyMap& reFrequencyMap ) const
{
  reFrequencyMap.clear();

  for ( const_iterator it=begin(); it!=end(); ++it )
  {
    reFrequencyMap.insert( FrequencyMap::ReFrequencyMap::value_type(it->second,it->first) );
  }
}


void filterSentences( SentenceList& sentenceList, const WordList& words )
{
  std::set<Word> wordSet;

  for (size_t i=0; i<words.size(); ++i )
  {
    wordSet.insert(words[i]);
  }

  for (size_t i=0; i<sentenceList.size(); ++i )
  {
    WordList& wordList = sentenceList[i].words;

    for ( size_t j=0; j<wordList.size(); ++j )
    {
      if ( wordSet.find(wordList[j]) == wordSet.end() )
      {
        wordList.erase(wordList.begin()+j);
        --j;
      }
    }
  }
}


void cStyleStringsToStringSet( const char** wordsPtr, std::set<Word>& words )
{
  words.clear();
  const char** currWordsPtr=wordsPtr;
  while (**currWordsPtr!='\0')
  {
    words.insert(*currWordsPtr);
    ++currWordsPtr;
  }
}

void removeHungarianStopwords( SentenceList& huSentenceList )
{
  const char* huStopwordsC[] =
  {
    "a", "az",

    "egy",

    "\xe9s",

    "nem", "ne",

    "is",

    "van",

    "\xf5",

    "ha",

    ""
  };

  std::set<Word> stopwords;
  cStyleStringsToStringSet( huStopwordsC, stopwords );


  for ( size_t i=0; i<huSentenceList.size(); ++i )
  {


    WordList& huWords = huSentenceList[i].words;
    for ( size_t j=0; j<huWords.size(); )
    {
      if (stopwords.find(huWords[j])!=stopwords.end())
      {
        huWords.erase(huWords.begin()+j);
      }
      else
      {
        ++j;
      }
    }
  }
}

void removeEnglishStopwords( SentenceList& enSentenceList )
{
  // Mar megbocsasson mindenki, hogy ezt programkodban rogzitem, de rogzitem.
  const char* enStopwordsC[] =
  {
    "the", "it",

    "a", "an", "one",

    "and",

    "not", "no",

    "too",

    "is", "be", // Az 1984 be-re stemmeli az is-t.

    "to",

    "he", "she",

    "if",

    "of",

    ""
  };

  std::set<Word> stopwords;
  cStyleStringsToStringSet( enStopwordsC, stopwords );


  for (size_t i=0; i<enSentenceList.size(); ++i )
  {

    WordList& enWords = enSentenceList[i].words;
    for (size_t j=0; j<enWords.size(); )
    {
      if (stopwords.find(enWords[j])!=stopwords.end())
      {
        enWords.erase(enWords.begin()+j);
      }
      else
      {
        ++j;
      }
    }
  }
}

void removeStopwords( SentenceList& huSentenceList, SentenceList& enSentenceList )
{
  removeHungarianStopwords( huSentenceList );
  removeEnglishStopwords  ( enSentenceList );
}

void TransLex::add( const Word& huWord, const Word& enWord )
{
  forward .insert( WordMultimap::value_type( huWord, enWord ) );
  backward.insert( WordMultimap::value_type( enWord, huWord ) );
}

// Note that multiword phrases are simply ignored.
void TransLex::build( const DictionaryItems& dictionaryItems )
{
  int added(0), ignored(0);
  for ( size_t i=0; i<dictionaryItems.size(); ++i )
  {
    if ( (dictionaryItems[i].first.size()==1) && (dictionaryItems[i].second.size()==1) )
    {
      add( dictionaryItems[i].first[0], dictionaryItems[i].second[0] );
      ++added;
    }
    else
    {
      ++ignored;
    }
  }
  std::cerr << added << " items added to TransLex, " << ignored << " multiword items ignored." << std::endl;
}

TransLex::DictInterval TransLex::lookupLeftWord ( const Word& huWord ) const
{
  return (forward.equal_range(huWord));
}

TransLex::DictInterval TransLex::lookupRightWord( const Word& enWord ) const
{
  return (backward.equal_range(enWord));
}

bool TransLex::isPresent( const Word& huWord, const Word& enWord ) const
{
  DictInterval dictInterval = lookupLeftWord(huWord);

  for ( WordMultimapIt it=dictInterval.first; it!=dictInterval.second; ++it )
  {
    if (it->second == enWord)
    {
      return true;
    }
  }
  return false;
}

double IBMModelOne::lookup( const Word& hu, const Word& en ) const
{
  TransProbs::const_iterator ft = transProbs.find( std::make_pair(hu,en) );

  if (ft==transProbs.end())
  {
    return 0;
  }
  else
  {
    return ft->second;
  }
}

void IBMModelOne::build( const SentenceList& huSentenceList, const SentenceList& enSentenceList )
{
  transProbs.clear();

  massert( huSentenceList.size()==enSentenceList.size() );


  std::map<Word,double> huProb;

  for ( size_t sen=0; sen<huSentenceList.size(); ++sen )
  {
    const Phrase& hu = huSentenceList[sen].words;
    const Phrase& en = enSentenceList[sen].words;

    double huRatio = 1.0 / hu.size();

    for ( size_t huPos=0; huPos<hu.size(); ++huPos )
    {
      const Word& huWord = hu[huPos];
      huProb[huWord] += huRatio;

      for ( size_t enPos=0; enPos<en.size(); ++enPos )
      {
        transProbs[ std::make_pair(huWord, en[enPos]) ] += huRatio ;
      }
    }
  }

  for ( TransProbs::iterator it=transProbs.begin(); it!=transProbs.end(); ++it )
  {
    it->second /= huProb[it->first.first];
  }
}

void IBMModelOne::reestimate( const SentenceList& huSentenceList, const SentenceList& enSentenceList )
{
  throw I18n(APR_I18N_DATA, "apertium").format("APR81230");
}

//
double IBMModelOne::distance( const Phrase& hu, const Phrase& en ) const
{
  double val = log(1.0+hu.size()) / en.size() ;

  for ( size_t enPos=0; enPos<en.size(); ++enPos )
  {
    double sum = 0;
    const Word& enWord = en[enPos];

    for ( size_t huPos=0; huPos<hu.size(); ++huPos )
    {
      sum += lookup( hu[huPos], enWord );
    }

    massert( sum>0 );

    val -= log(sum);
  }

  throw I18n(APR_I18N_DATA, "apertium").format("APR81230");
}

} // namespace TMXAligner
