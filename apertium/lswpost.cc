/*
 * Copyright (C) 2005 Universitat d'Alacant / Universidad de Alicante
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/**
 *  Light Sliding-Window Part of Speech Tagger (LSWPoST) implementation (source)
 *
 *  @author   Gang Chen - pkuchengang@gmail.com
 */


#include <apertium/lswpost.h>
#include <apertium/tagger_utils.h>
#include  "apertium_config.h"
#include <apertium/unlocked_cstdio.h>
#include <lttoolbox/compression.h>

#ifdef WIN32
#define isnan(n) _isnan(n)
#define isinf(n) (!_finite(n))
#endif

#ifdef __clang__
#undef __GNUC__
#endif

#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <apertium/string_utils.h>
#include <cstdlib>

using namespace std;
using namespace Apertium;
using namespace tagger_utils;

void LSWPoST::deserialise(FILE *Serialised_FILE_Tagger) {
  tdlsw.read(Serialised_FILE_Tagger);
  eos = (tdlsw.getTagIndex())[L"TAG_SENT"];
}

std::vector<std::wstring> &LSWPoST::getArrayTags() {
  return tdlsw.getArrayTags();
}

void LSWPoST::train(FILE *Corpus, unsigned long Count) {
  train(Corpus);
  --Count;

  for (; Count > 0; --Count) {
    std::fseek(Corpus, 0, SEEK_SET);
    train(Corpus);
  }
}

void LSWPoST::serialise(FILE *Stream_) { tdlsw.write(Stream_); }

void LSWPoST::deserialise(const TaggerData &Deserialised_FILE_Tagger) {
  tdlsw = TaggerDataLSW(Deserialised_FILE_Tagger);
  eos = (tdlsw.getTagIndex())[L"TAG_SENT"];
}

void LSWPoST::init_probabilities_from_tagged_text_(FILE *TaggedCorpus,
                                                   FILE *UntaggedCorpus) {
  std::abort();
}

void LSWPoST::init_probabilities_kupiec_(FILE *Corpus) {
  init_probabilities(Corpus);
}

void LSWPoST::train_(FILE *Corpus, unsigned long Count) {
  for (; Count > 0; --Count) {
    std::fseek(Corpus, 0, SEEK_SET);
    train(Corpus);
  }
}

LSWPoST::LSWPoST() {}

LSWPoST::LSWPoST(TaggerDataLSW t) {
  tdlsw = t;
  eos = (tdlsw.getTagIndex())[L"TAG_SENT"];  
}

LSWPoST::~LSWPoST() {}

LSWPoST::LSWPoST(TaggerDataLSW *tdlsw) : tdlsw(*tdlsw) {}

void
LSWPoST::set_eos(TTag t) { 
  eos = t; 
} 

void
LSWPoST::init_probabilities(FILE *ftxt) {

  int N = tdlsw.getN();
  int nw = 0;
  TaggerWord *word = NULL;
  set<TTag> tags_left, tags_mid, tags_right;
  set<TTag>::iterator iter_left, iter_mid, iter_right;
  vector<vector<vector<double> > > para_matrix(N, vector<vector<double> >(N, vector<double>(N, 0)));
  Collection &output = tdlsw.getOutput();
  MorphoStream morpho_stream(ftxt, true, &tdlsw);
  int num_valid_seq = 0;
  
  word = new TaggerWord();          // word for tags left
  word->add_tag(eos, L"sent", tdlsw.getPreferRules());
  tags_left = word->get_tags();     // tags left
  if (tags_left.size()==0) { //This is an unknown word
    tags_left = tdlsw.getOpenClass();
  }
  if (output.has_not(tags_left)) {
    wstring errors;
    errors = L"A new ambiguity class was found. I cannot continue.\n";
    errors += L"Word '" + word->get_superficial_form() + L"' not found in the dictionary.\n";
    errors += L"New ambiguity class: " + word->get_string_tags() + L"\n";
    errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
    fatal_error(errors);
  }
  ++nw;
  delete word;
  word = morpho_stream.get_next_word();  // word for tags mid
  tags_mid = word->get_tags();           // tags mid
  if (tags_mid.size()==0) { //This is an unknown word
    tags_mid = tdlsw.getOpenClass();
  }
  if (output.has_not(tags_mid)) {
    wstring errors;
    errors = L"A new ambiguity class was found. I cannot continue.\n";
    errors += L"Word '" + word->get_superficial_form() + L"' not found in the dictionary.\n";
    errors += L"New ambiguity class: " + word->get_string_tags() + L"\n";
    errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
    fatal_error(errors);
  }
  ++nw;
  delete word;
  if (morpho_stream.getEndOfFile()) {
    return;
  }

  word = morpho_stream.get_next_word();   // word for tags right

  // count each element of the para matrix
  while (word != NULL) {
    if (++nw % 10000 == 0) {
      wcerr << L'.' << flush;
    }

    tags_right = word->get_tags();       // tags right
    if (tags_right.size()==0) { //This is an unknown word
      tags_right = tdlsw.getOpenClass();
    }
    if (output.has_not(tags_right)) {
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors += L"Word '" + word->get_superficial_form() + L"' not found in the dictionary.\n";
      errors += L"New ambiguity class: " + word->get_string_tags() + L"\n";
      errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
      fatal_error(errors);
    }

    num_valid_seq = tags_left.size() * tags_mid.size() * tags_right.size();
    for (iter_left = tags_left.begin(); iter_left != tags_left.end(); ++iter_left) {
      for (iter_mid = tags_mid.begin(); iter_mid != tags_mid.end(); ++iter_mid) {
        for (iter_right = tags_right.begin(); iter_right != tags_right.end(); ++iter_right) {
          if (!is_valid_seq(*iter_left, *iter_mid, *iter_right)) {
            --num_valid_seq;
          }
        } // for iter_right
      } // for iter_mid
    } // for iter_left

    if (num_valid_seq != 0) {
      for (iter_left = tags_left.begin(); iter_left != tags_left.end(); ++iter_left) {
        for (iter_mid = tags_mid.begin(); iter_mid != tags_mid.end(); ++iter_mid) {
          for (iter_right = tags_right.begin(); iter_right != tags_right.end(); ++iter_right) {
            if (is_valid_seq(*iter_left, *iter_mid, *iter_right)) {
              para_matrix[*iter_left][*iter_mid][*iter_right] += 1.0 / num_valid_seq;
            }
          } // for iter_right
        } // for iter_mid
      } // for iter_left
    }

    tags_left = tags_mid;
    tags_mid = tags_right;
    delete word;
    word = morpho_stream.get_next_word();
  } // while word != NULL

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      for (int k = 0; k < N; ++k) {
        tdlsw.getD()[i][j][k] = para_matrix[i][j][k];
      }
    }
  }

  wcerr << L"\n";
}

bool LSWPoST::is_valid_seq(TTag left, TTag mid, TTag right) {

  vector<TForbidRule> &forbid_rules = tdlsw.getForbidRules();
  vector<TEnforceAfterRule> &enforce_rules = tdlsw.getEnforceRules();

  for (size_t r = 0; r < forbid_rules.size(); ++r) {
    if ((left == forbid_rules[r].tagi && mid == forbid_rules[r].tagj)
        || (mid == forbid_rules[r].tagi && right == forbid_rules[r].tagj)) {
      return false;
    }
  }// for r in forbid rules

  for (size_t r = 0; r < enforce_rules.size(); ++r) {
    if (left == enforce_rules[r].tagi) {
      bool found = false;
      for (size_t j = 0; j < enforce_rules[r].tagsj.size(); ++j) {
        if (enforce_rules[r].tagsj[j] == mid) {
          found = true;
          break;
        }
      }
      if (!found) {
        return false;
      }
    } else if (mid == enforce_rules[r].tagi) {
      bool found = false;
      for (size_t j = 0; j < enforce_rules[r].tagsj.size(); ++j) {
        if (enforce_rules[r].tagsj[j] == right) {
          found = true;
          break;
        }
      }
      if (!found) {
        return false;
      }
    }
  } // for r in enforce rules

  return true;
}

void
LSWPoST::read_dictionary(FILE *fdic) {
  int i, k, nw = 0;
  TaggerWord *word = NULL;
  set<TTag> tags;
  Collection &output = tdlsw.getOutput();

  MorphoStream morpho_stream(fdic, true, &tdlsw);

  // In the input dictionary there must be all punctuation marks, including the end-of-sentece mark

  word = morpho_stream.get_next_word();

  while (word) {
    if (++nw % 10000 == 0)
      wcerr << L'.' << flush;

    tags = word->get_tags();

    if (tags.size() > 0)
      k = output[tags];

    delete word;
    word = morpho_stream.get_next_word();
  }
  wcerr << L"\n";

  // OPEN AMBIGUITY CLASS
  // It contains all tags that are not closed.
  // Unknown words are assigned the open ambiguity class
  k = output[tdlsw.getOpenClass()];

  int N = (tdlsw.getTagIndex()).size();

  // Create ambiguity class holding one single tag for each tag.
  // If not created yet
  for (i = 0; i != N; i++) {
    set < TTag > amb_class;
    amb_class.insert(i);
    k = output[amb_class];
  }

  wcerr << N << L" states\n";

  // set up the probability matrix of tdlsw, the pointer to the TaggerDataLSW object
  tdlsw.setProbabilities(N);
}

void
LSWPoST::train(FILE *ftxt) {

  int N = tdlsw.getN();
  int nw = 0;
  TaggerWord *word = NULL;
  set<TTag> tags_left, tags_mid, tags_right;
  set<TTag>::iterator iter_left, iter_mid, iter_right;
  vector<vector<vector<double> > > para_matrix_new(N, vector<vector<double> >(N, vector<double>(N, 0)));
  Collection &output = tdlsw.getOutput();
  MorphoStream morpho_stream(ftxt, true, &tdlsw);

  word = new TaggerWord();          // word for tags left
  word->add_tag(eos, L"sent", tdlsw.getPreferRules());
  tags_left = word->get_tags();     // tags left
  if (tags_left.size()==0) { //This is an unknown word
    tags_left = tdlsw.getOpenClass();
  }
  if (output.has_not(tags_left)) {
    wstring errors;
    errors = L"A new ambiguity class was found. I cannot continue.\n";
    errors += L"Word '" + word->get_superficial_form() + L"' not found in the dictionary.\n";
    errors += L"New ambiguity class: " + word->get_string_tags() + L"\n";
    errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
    fatal_error(errors);
  }
  ++nw;
  delete word;
  word = morpho_stream.get_next_word();  // word for tags mid
  tags_mid = word->get_tags();           // tags mid
  if (tags_mid.size()==0) { //This is an unknown word
    tags_mid = tdlsw.getOpenClass();
  }
  if (output.has_not(tags_mid)) {
    wstring errors;
    errors = L"A new ambiguity class was found. I cannot continue.\n";
    errors += L"Word '" + word->get_superficial_form() + L"' not found in the dictionary.\n";
    errors += L"New ambiguity class: " + word->get_string_tags() + L"\n";
    errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
    fatal_error(errors);
  }
  ++nw;
  delete word;
  if (morpho_stream.getEndOfFile()) {
    return;
  }

  word = morpho_stream.get_next_word();   // word for tags right

  while (word) {
    if (++nw % 10000 == 0) {
      wcerr << L'.' << flush;
    }

    tags_right = word->get_tags();       // tags right
    if (tags_right.size()==0) { //This is an unknown word
      tags_right = tdlsw.getOpenClass();
    }
    if (output.has_not(tags_right)) {
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors += L"Word '" + word->get_superficial_form() + L"' not found in the dictionary.\n";
      errors += L"New ambiguity class: " + word->get_string_tags() + L"\n";
      errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
      fatal_error(errors);
    }

    double normalization = 0;

    for (iter_left = tags_left.begin(); iter_left != tags_left.end(); ++iter_left) {
      for (iter_mid = tags_mid.begin(); iter_mid != tags_mid.end(); ++iter_mid) {
        for (iter_right = tags_right.begin(); iter_right != tags_right.end(); ++iter_right) {
          normalization += tdlsw.getD()[*iter_left][*iter_mid][*iter_right];
        }
      }
    }

    for (iter_left = tags_left.begin(); iter_left != tags_left.end(); ++iter_left) {
      for (iter_mid = tags_mid.begin(); iter_mid != tags_mid.end(); ++iter_mid) {
        for (iter_right = tags_right.begin(); iter_right != tags_right.end(); ++iter_right) {
          if (normalization > ZERO) {
            para_matrix_new[*iter_left][*iter_mid][*iter_right] +=
                tdlsw.getD()[*iter_left][*iter_mid][*iter_right] / normalization;
          }
        }
      }
    }

    tags_left = tags_mid;
    tags_mid = tags_right;
    delete word;
    word = morpho_stream.get_next_word();
  }

  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < N; ++j) {
      for (int k = 0; k < N; ++k) {
        tdlsw.getD()[i][j][k] = para_matrix_new[i][j][k];
      }
    }
  }
}

void
LSWPoST::print_para_matrix() {
  wcout << L"para matrix D\n----------------------------\n";
  for (int i = 0; i < tdlsw.getN(); ++i) {
    for (int j = 0; j < tdlsw.getN(); ++j) {
      for (int k = 0; k < tdlsw.getN(); ++k) {
        wcout << L"D[" << i << L"][" << j << L"][" << k << L"] = "
            << tdlsw.getD()[i][j][k] << "\n";
      }
    }
  }
}

void 
LSWPoST::tagger(FILE *Input, FILE *Output, const bool &First) {
  TaggerWord *word_left = NULL, *word_mid = NULL, *word_right = NULL;
  set<TTag> tags_left, tags_mid, tags_right;
  set<TTag>::iterator iter_left, iter_mid, iter_right;
  MorphoStream morpho_stream(Input, debug, &tdlsw);
  morpho_stream.setNullFlush(null_flush);                      
  Collection &output = tdlsw.getOutput();
 
  word_left = new TaggerWord();          // word left
  word_left->add_tag(eos, L"sent", tdlsw.getPreferRules());
  word_left->set_show_sf(show_sf);
  tags_left = word_left->get_tags();          // tags left
  if (output.has_not(tags_left)) {
    if (debug) {
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors += L"Word '" + word_left->get_superficial_form() + L"' not found Input the dictionary.\n";
      errors += L"New ambiguity class: " + word_left->get_string_tags() + L"\n";
      errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
      fatal_error(errors);
    }
    tags_left = find_similar_ambiguity_class(tags_left);
  }
  
  word_mid = morpho_stream.get_next_word(); // word mid
  word_mid->set_show_sf(show_sf);
  tags_mid = word_mid->get_tags();          // tags mid
  if (output.has_not(tags_mid)) {
    if (debug) {
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors += L"Word '" + word_mid->get_superficial_form() + L"' not found Input the dictionary.\n";
      errors += L"New ambiguity class: " + word_mid->get_string_tags() + L"\n";
      errors += L"Take a look at the dictionary and at the training corpus. Then, retrain.";
      fatal_error(errors);
    }
    tags_mid = find_similar_ambiguity_class(tags_mid);
  }
  if (morpho_stream.getEndOfFile()) {
    delete word_left;
    delete word_mid;
    return;
  }
  word_right = morpho_stream.get_next_word(); // word_right
  word_right->set_show_sf(show_sf);

  wstring micad;

  while (word_right) {

    tags_right = word_right->get_tags();
    if (output.has_not(tags_right)) {
      if (debug) {
        wstring errors;
        errors = L"A new ambiguity class was found. \n";
        errors+= L"Retraining the tagger is necessary so as to take it into account.\n";
        errors+= L"Word '"+word_right->get_superficial_form()+L"'.\n";
        errors+= L"New ambiguity class: "+word_right->get_string_tags()+L"\n";
        fatal_error(errors);
      }
      tags_right = find_similar_ambiguity_class(tags_right);
    }

    double max = -1;
    TTag tag_max = *tags_mid.begin();
    for (iter_mid = tags_mid.begin(); iter_mid != tags_mid.end(); ++iter_mid) {
      double n = 0;
      for (iter_left = tags_left.begin(); iter_left != tags_left.end(); ++iter_left) {
        for (iter_right = tags_right.begin(); iter_right != tags_right.end(); ++iter_right) {
          n += tdlsw.getD()[*iter_left][*iter_mid][*iter_right];
        }
      }
      if (n > max) {
        max = n;
        tag_max = *iter_mid;
      }
    }

    micad = word_mid->get_lexical_form(tag_max, (tdlsw.getTagIndex())[L"TAG_kEOF"]);
    fputws_unlocked(micad.c_str(), Output);
    if (morpho_stream.getEndOfFile()) {
      if (null_flush) {
        fputwc_unlocked(L'\0', Output);
      }
      fflush(Output);
      morpho_stream.setEndOfFile(false);
    }
  
    delete word_left;
    word_left = word_mid;
    tags_left = tags_mid;
    word_mid = word_right;
    tags_mid = tags_right;
    word_right = morpho_stream.get_next_word();
    if (word_right != NULL) {
      word_right->set_show_sf(show_sf);
    }
  }
  delete word_left;
  delete word_mid;
}

set<TTag>                                                                                     
LSWPoST::find_similar_ambiguity_class(set<TTag> c) {
  int size_ret = -1;                                                                          
  set<TTag> ret=tdlsw.getOpenClass(); // return open-class as default, if no better is found.
  bool skip_class;                                                                           
  Collection &output = tdlsw.getOutput();                                                       
                                                                                              
  for(int k=0; k<output.size(); k++) {                                                           
    if ((((int)output[k].size())>((int)size_ret)) && (((int)output[k].size())<((int)c.size()))) {    
      skip_class=false;                                                                      
      // Test if output[k] is a subset of class                                               
      for(set<TTag>::const_iterator it=output[k].begin(); it!=output[k].end(); it++) {        
        if (c.find(*it)==c.end()) { 
           skip_class=true; //output[k] is not a subset of class                             
           break;
        }  
      } 
      if (!skip_class) {                                                                     
        size_ret = output[k].size();                                                          
             ret = output[k];
      }      
    } 
  } 
  return ret;                                                                                 
} 

