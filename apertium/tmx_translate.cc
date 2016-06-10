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
#include <apertium/tmx_translate.h>

#include <apertium/tmx_words.h>
#include <apertium/tmx_dictionary.h>
#include <apertium/tmx_dic_tree.h>
#include <apertium/string_utils.h>

#include <algorithm>
#include <fstream>

namespace TMXAligner
{

void buildDumbDictionary( const DictionaryItems& dictionary, DumbDictionary& dumbDictionary )
{
  dumbDictionary.clear();

  for (size_t i=0; i<dictionary.size(); ++i )
  {
    const Phrase& en = dictionary[i].first;
    const Phrase& hu = dictionary[i].second;

    if (hu.size()==1)
    {
      dumbDictionary[ hu[0] ] = en ;
      // std::wcerr << hu[0] << "\t" << en << std::endl;
    }
  }
}

void buildDumbDictionaryUsingFrequencies( 
       const DictionaryItems& dictionary, 
       FrequencyMap& enFreq, 
       DumbDictionary& dumbDictionary )
{
  dumbDictionary.clear();


  for (size_t i=0; i<dictionary.size(); ++i )
  {
    const Phrase& en = dictionary[i].first;
    const Phrase& hu = dictionary[i].second;

    if (hu.size()==1)
    {
      Word originalWord = hu[0];
      DumbDictionary::const_iterator ft = dumbDictionary.find(originalWord);
      bool overWrite = false;
      if (ft!=dumbDictionary.end())
      {
        // Phrases with both length of k>1 are incomparable.

        const Phrase& oldTrans = ft->second;

        // Shorter phrases are better than longer ones.
        if (oldTrans.size()>en.size())
        {
          overWrite = true;
        }

        // More frequent words are better than less frequent ones.
        if ( (oldTrans.size()==1) && (en.size()==1) )
        {
          if ( enFreq[oldTrans[0]] < enFreq[en[0]] )
          {
            overWrite = true;
          }
        }
      }
      else
      {
        overWrite = true;
      }

      if (overWrite)
        dumbDictionary[originalWord] = en ;
    }
  }
}

void buildDumbDictionary( TMXAligner::DumbDictionary& dumbDictionary,
                          const std::string& dictionaryFilename,
                          const TMXAligner::SentenceList& enSentenceList
                        )
{
  TMXAligner::DictionaryItems dictionary;
  {
    std::ifstream is( dictionaryFilename.c_str() );
    dictionary.read( is );
    std::wcerr << dictionary.size() << " dictionary items read." << std::endl;
  }

  if (!enSentenceList.empty())
  {
    TMXAligner::FrequencyMap enFreq;
    enFreq.build(enSentenceList);
    TMXAligner::buildDumbDictionaryUsingFrequencies( dictionary, enFreq, dumbDictionary );
  }
  else
  {
    TMXAligner::buildDumbDictionary( dictionary, dumbDictionary );
  }
}

void trivialTranslateWord(
                     const DumbDictionary& dumbDictionary,
                     const Word& originalWord,
                     Phrase& words
                     )
{
  words.clear();

  DumbDictionary::const_iterator ft = dumbDictionary.find(originalWord);
  if (ft!=dumbDictionary.end())
  {
    words = ft->second;
  }
  else
  {
    bool leaveAsItis(false);

    // This worsens the score for the 1984 corpus, most possibly because of the false cognates a(a), is(is), van(van).
    bool alwaysLeaveAsItis = true;
    if (alwaysLeaveAsItis)
    {
      leaveAsItis = true;
    }

    if ( !leaveAsItis && (originalWord[0]>='A') && (originalWord[0]<='Z') )
    {
      leaveAsItis = true;
    }

    if (!leaveAsItis)
    {
      bool isNumber(true);
      for ( size_t k=0; k<originalWord.size(); ++k )
      {
        char c = originalWord[k];
        if ( (c!='.') && ( (c<'0') || (c>'9') ) )
        {
          isNumber = false;
          break;
        }
      }

      if (isNumber)
      {
        leaveAsItis = true;
      }
    }

    if (leaveAsItis)
    {
      words.push_back(originalWord);
    }
  }
}

void trivialTranslate(
                     const DumbDictionary& dumbDictionary,
                     const Sentence& sentence,
                           Sentence& translatedSentence
                     )
{
  bool logging = false;

  std::ofstream* translateLogsPtr = 0;
  if (logging)
  {
    translateLogsPtr = new std::ofstream( "/dev/null", std::ios::app );
  }
  std::ostream& logs = translateLogsPtr ? *translateLogsPtr : std::cout ;

  translatedSentence.id = sentence.id;
  Phrase& words = translatedSentence.words;

  if (logging && !translatedSentence.id.empty())
    logs << translatedSentence.id << "\t";

  const Phrase& originalWords = sentence.words;

  for ( size_t j=0; j<originalWords.size(); ++j )
  {
    Word originalWord = originalWords[j];

    Phrase phrase;
    trivialTranslateWord( dumbDictionary, originalWord, phrase );
    
    for (size_t k=0; k<phrase.size(); ++k )
    {
      words.push_back(phrase[k]);
    }

    if (logging)
      logs << originalWord << "(";
    for (size_t k=0; k<phrase.size(); ++k )
    {
      if (logging)
      {
        logs << phrase[k];
        if (k<phrase.size()-1)
          logs << " ";
      }
    }
    if (logging)
      logs << ") ";
  }

  if (logging)
    logs << "\n";

  if (logging)
  {
    delete translateLogsPtr;
  }
}

void trivialTranslateSentenceList(
                     const DumbDictionary& dumbDictionary,
                     const SentenceList& sentenceList,
                           SentenceList& translatedSentenceList
                     )
{
  {
    std::ofstream translateLogs( "/dev/null" );
  }
  
  translatedSentenceList.clear();

  for ( size_t i=0; i<sentenceList.size(); ++i )
  {
    Sentence translatedSentence;

    trivialTranslate( dumbDictionary, 
                      sentenceList[i],
                      translatedSentence
                     );

    translatedSentenceList.push_back(translatedSentence);
  }
}


void naiveTranslate(
                     const DictionaryItems& dictionary,
                     const SentenceList& sentenceList,
                           SentenceList& translatedSentenceList
                     )
{
  translatedSentenceList.clear();

  SubsetLookup<Word,int> subsetLookup;
  {
    for ( size_t i=0; i<dictionary.size(); ++i )
    {
      subsetLookup.add( dictionary[i].second, i+1 ); // !!! i+1
    }
    std::wcerr << "Index tree built." << std::endl;
  }

  for ( size_t i=0; i<sentenceList.size(); ++i )
  {
    Sentence sentence;
    sentence.id = sentenceList[i].id;
    Phrase& words = sentence.words;

    std::set<int> results;
    subsetLookup.lookup( sentenceList[i].words, results );

    for ( std::set<int>::const_iterator it=results.begin(); it!=results.end(); ++it )
    {
      const Phrase& phrase = dictionary[*it-1].first; // !!! i-1

      for ( size_t i=0; i<phrase.size(); ++i )
      {
        words.push_back(phrase[i]);
      }
    }

    translatedSentenceList.push_back(sentence);
  }

  std::wcerr << "Analysis ready." << std::endl;
}


void buildDumbMultiDictionary( const DictionaryItems& dictionary, DumbMultiDictionary& dumbMultiDictionary, bool reverse )
{
  dumbMultiDictionary.clear();

  for (size_t i=0; i<dictionary.size(); ++i )
  {
    const Phrase& en = dictionary[i].first;
    const Phrase& hu = dictionary[i].second;

    if (!reverse)
    {
      if (hu.size()==1)
      {
        dumbMultiDictionary.insert( DumbMultiDictionary::value_type( hu[0], en ) );
      }
    }
    else
    {
      if (en.size()==1)
      {
        dumbMultiDictionary.insert( DumbMultiDictionary::value_type( en[0], hu ) );
      }
    }
  }
}


void sortNormalizeSentences( TMXAligner::SentenceList& sentenceList )
{
  {
    for ( size_t pos=0; pos<sentenceList.size(); ++pos )
    {
      TMXAligner::Phrase& sentence = sentenceList[pos].words;
      std::sort(sentence.begin(),sentence.end());
    }
  }
}


void normalizeTextsForIdentity( const DictionaryItems& dictionary,
                                const SentenceList& huSentenceListPretty,  const SentenceList& enSentenceListPretty,
                                      SentenceList& huSentenceListGarbled,       SentenceList& enSentenceListGarbled )
{
  DumbDictionary dumbDictionary;

  FrequencyMap enFreq;
  enFreq.build(enSentenceListPretty);
  buildDumbDictionaryUsingFrequencies( dictionary, enFreq, dumbDictionary );

//  std::wcerr << "Simplified dictionary ready." << std::endl;

  SentenceList huSentenceList;

  trivialTranslateSentenceList( dumbDictionary, huSentenceListPretty, huSentenceListGarbled );

//  std::wcerr << "Rough translation ready." << std::endl;

  sortNormalizeSentences(huSentenceListGarbled);

  enSentenceListGarbled = enSentenceListPretty;
  sortNormalizeSentences(enSentenceListGarbled);
}


} // namespace TMXAligner
