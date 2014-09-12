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
#ifndef __TMXALIGNER_ALIGNMENT_TRANSLATE_H
#define __TMXALIGNER_ALIGNMENT_TRANSLATE_H

#include <apertium/tmx_words.h>
#include <apertium/tmx_dictionary.h>

namespace TMXAligner
{

typedef std::map< std::string, Phrase > DumbDictionary;

// This will become a class, with dictionary initialization, and a translate method.
// It will have various implementations.

void buildDumbDictionary( const DictionaryItems& dictionary, DumbDictionary& dumbDictionary );

void buildDumbDictionaryUsingFrequencies( 
       const DictionaryItems& dictionary, 
       FrequencyMap& enFreq, 
       DumbDictionary& dumbDictionary );

void buildDumbDictionary( TMXAligner::DumbDictionary& dumbDictionary,
                          const std::string& dictionaryFilename,
                          const TMXAligner::SentenceList& enSentenceList = TMXAligner::SentenceList()
                        );

void trivialTranslateWord(
                     const DumbDictionary& dumbDictionary,
                     const Word& originalWord,
                     Phrase& words
                     );

void trivialTranslate(
                     const DumbDictionary& dumbDictionary,
                     const Sentence& sentence,
                           Sentence& translatedSentence
                     );

void trivialTranslateSentenceList(
                     const DumbDictionary& dumbDictionary,
                     const SentenceList& sentenceList,
                           SentenceList& translatedSentenceList
                     );

void naiveTranslate(
                     const DictionaryItems& dictionary,
                     const SentenceList& sentenceList,
                           SentenceList& translatedSentenceList
                     );

typedef std::multimap< std::string, Phrase > DumbMultiDictionary;

void buildDumbMultiDictionary( const DictionaryItems& dictionary, DumbMultiDictionary& dumbMultiDictionary, bool reverse );

void sortNormalizeSentences( TMXAligner::SentenceList& sentenceList );

// This function preprocesses the sentences so that sentenceListsToAlignMatrixIdentity can be applied to them.
// It does a rough translation and an alphabetic sort of words.
void normalizeTextsForIdentity( const DictionaryItems& dictionary,
                                const SentenceList& huSentenceListPretty,  const SentenceList& enSentenceListPretty,
                                      SentenceList& huSentenceListGarbled,       SentenceList& enSentenceListGarbled );

} // namespace TMXAligner

#endif // #define __TMXALIGNER_ALIGNMENT_TRANSLATE_H
