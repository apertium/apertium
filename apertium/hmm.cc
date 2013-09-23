
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
/*
 *  First order hidden Markov model (HMM) implementation (source)
 *
 *  @author	Felipe Sánchez-Martínez - fsanchez@dlsi.ua.es
 */

#include <apertium/hmm.h>
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

using namespace Apertium;
using namespace tagger_utils;

HMM::HMM(TaggerDataHMM *tdhmm)
{
  this->tdhmm = tdhmm;

  debug=false;
  show_sf=false;
  null_flush = false;
  eos = (tdhmm->getTagIndex())[L"TAG_SENT"];  
}

HMM::~HMM()
{
}

void
HMM::init()
{
}

void
HMM::set_eos(TTag t) 
{ 
  eos = t; 
} 

void
HMM::set_debug(bool d)
{ 
  debug = d; 
} 

void
HMM::set_show_sf(bool sf)
{ 
  show_sf = sf; 
} 

void 
HMM::read_ambiguity_classes(FILE *in) 
{
  while(in)
  {
    int ntags = Compression::multibyte_read(in);

    if(feof(in))
    {
      break;
    }
    set<TTag> ambiguity_class;

    for(; ntags != 0; ntags--)
    {
      ambiguity_class.insert(Compression::multibyte_read(in));
    }
    
    if(ambiguity_class.size() != 0)
    {
      tdhmm->getOutput().add(ambiguity_class);
    }     
  }
  
  tdhmm->setProbabilities(tdhmm->getTagIndex().size(), tdhmm->getOutput().size());
}

void 
HMM::write_ambiguity_classes(FILE *out) 
{
  for(int i=0, limit = tdhmm->getOutput().size(); i != limit; i++) 
  {
    set<TTag> const &ac = (tdhmm->getOutput())[i];
    Compression::multibyte_write(ac.size(), out);
    for(set<TTag>::const_iterator it = ac.begin(), limit2 = ac.end();
        it != limit2; it++)
    {
      Compression::multibyte_write(*it, out);
    }
  } 
}  

void 
HMM::read_probabilities(FILE *in)
{
  tdhmm->read(in);
}

void 
HMM::write_probabilities(FILE *out)
{
  tdhmm->write(out);  
}  

void 
HMM::init_probabilities_kupiec (FILE *is)
{
  int N = tdhmm->getN();
  int M = tdhmm->getM();
  int i=0, j=0, k=0, k1=0, k2=0, nw=0;
#ifdef __GNUC__
  double classes_ocurrences[M]; //M = Number of ambiguity classes
  double classes_pair_ocurrences[M][M];
  double tags_estimate[N]; //N = Number of tags (states)
  double tags_pair_estimate[N][N];
#else
  vector <double> classes_ocurrences (M, 1);
  vector <vector <double> > classes_pair_ocurrences(M, vector<double>(M, 1));
  vector <double> tags_estimate(N, 0);
  vector <vector <double> > tags_pair_estimate(N, vector<double>(N, 0));
#endif
  
  Collection &output = tdhmm->getOutput();
 
  MorphoStream lexmorfo(is, true, tdhmm);
  
  TaggerWord *word=NULL;

#ifdef __GNUC__
  for(k=0; k<M; k++) {
    classes_ocurrences[k]=1; 
    for (k2=0; k2<M; k2++)
      classes_pair_ocurrences[k][k2]=1;
  }
#endif

  set<TTag> tags;
  tags.insert(eos);  
  k1=output[tags]; //The first tag (ambiguity class) seen is the end-of-sentence
  
  //We count for each ambiguity class the number of ocurrences
  word = lexmorfo.get_next_word();
  while((word)) {
    if (++nw%10000==0) wcerr<<L'.'<<flush; 
    
    tags=word->get_tags();

    if (tags.size()==0) { //This is an unknown word
      tags = tdhmm->getOpenClass();
    }
    else if (output.has_not(tags)) { 
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors+= L"Word '"+word->get_superficial_form()+L"' not found in the dictionary.\n";
      errors+= L"New ambiguity class: "+word->get_string_tags()+L"\n";
      errors+= L"Take a look at the dictionary and at the training corpus. Then, retrain.";      
      fatal_error(errors);      
    }    

    k2=output[tags];

    classes_ocurrences[k1]++;
    classes_pair_ocurrences[k1][k2]++;  //k1 followed by k2
    delete word;
    word=lexmorfo.get_next_word();

    k1=k2;

  }  

  //Estimation of the number of time each tags occurs in the training text
  for(i=0; i<N; i++) {  
#ifdef __GNUC__
    tags_estimate[i]=0;
#endif
    for(k=0; k<M;  k++) { 
  
      if(output[k].find(i) != output[k].end())
        tags_estimate[i] += classes_ocurrences[k]/output[k].size();	
    }
  }
  
#ifdef __GNUC__
  //Estimation of the number of times each tag pair occurs
  for(i=0; i<N; i++)
    for(j=0; j<N; j++)
      tags_pair_estimate[i][j]=0;
#endif

  set<TTag> tags1, tags2;
  set<TTag>::iterator itag1, itag2;
  for(k1=0; k1<M; k1++) {
    tags1=output[k1];
    for(k2=0; k2<M; k2++) {
      tags2=output[k2];
      double nocurrences=classes_pair_ocurrences[k1][k2]/((double)(tags1.size()*tags2.size()));
      for (itag1=tags1.begin(); itag1!=tags1.end(); itag1++) {
        for (itag2=tags2.begin(); itag2!=tags2.end(); itag2++)
          tags_pair_estimate[*itag1][*itag2]+=nocurrences;
      }
    }
  }

   //a[i][j] estimation.
  double sum;
  for(i=0; i<N; i++) {
    sum=0;
    for(j=0; j<N; j++)
      sum+=tags_pair_estimate[i][j];

    for(j=0; j<N; j++) {  
      if (sum>0)
        (tdhmm->getA())[i][j] = tags_pair_estimate[i][j]/sum;
      else {
        (tdhmm->getA())[i][j] = 0;
      }
    }
  }

  //b[i][k] estimation
  for(i=0; i<N; i++) {
    for(k=0; k<M; k++)  {
      if (output[k].find(i)!=output[k].end()) {
        if (tags_estimate[i]>0)
          (tdhmm->getB())[i][k] = (classes_ocurrences[k]/output[k].size())/tags_estimate[i];
        else 
	  (tdhmm->getB())[i][k] = 0;
      }
    }
  }
  wcerr<<L"\n";
}

void 
HMM::init_probabilities_from_tagged_text(FILE *ftagged, FILE *funtagged) {
  int i, j, k, nw=0;
  int N = tdhmm->getN();
  int M = tdhmm->getM();
#ifdef __GNUC__
  double tags_pair[N][N];
  double emission[N][M];
#else
  vector <vector <double> > tags_pair(N, vector<double>(N, 0));
  vector <vector <double> > emission(N, vector<double>(M, 0));
#endif


  MorphoStream stream_tagged(ftagged, true, tdhmm);
  MorphoStream stream_untagged(funtagged, true, tdhmm);
  
  TaggerWord *word_tagged=NULL, *word_untagged=NULL;
  Collection &output = tdhmm->getOutput();

  
  set<TTag> tags;

#ifdef __GNUC__ 
  // Init counters - each event appears at least once. 
  // Espected likelihood estimate (ELE) with a fixed initial count of 1
  for(i=0; i<N; i++) {
    for(j=0; j<N; j++)
      tags_pair[i][j]=0;
  }
  for(k=0; k<M; k++) {
    for(i=0; i<N; i++) {
      if (output[k].find(i)!=output[k].end())
        emission[i][k] = 0;
    }  
  }
#endif 

  TTag tag1, tag2;  
  tag1 = eos; // The first seen tag is the end-of-sentence tag
  
  word_tagged = stream_tagged.get_next_word();
  word_untagged = stream_untagged.get_next_word();
  while(word_tagged) {
    wcerr<<*word_tagged;
    wcerr<<L" -- "<<*word_untagged<<L"\n"; 

    if (word_tagged->get_superficial_form()!=word_untagged->get_superficial_form()) {              
      wcerr<<L"\nTagged text (.tagged) and analyzed text (.untagged) streams are not aligned.\n";
      wcerr<<L"Take a look at tagged text (.tagged).\n";
      wcerr<<L"Perhaps this is caused by a multiword unit that is not a multiword unit in one of the two files.\n";
      wcerr<<*word_tagged<<L" -- "<<*word_untagged<<L"\n"; 
      exit(1);
    }

    if (++nw%100==0) wcerr<<L'.'<<flush; 
    
    tag2 = tag1;
   
    if (word_untagged==NULL) {
      wcerr<<L"word_untagged==NULL\n";
      exit(1);
    }

    if (word_tagged->get_tags().size()==0) // Unknown word
      tag1 = -1;
    else if (word_tagged->get_tags().size()>1) // Ambiguous word
      wcerr<<L"Error in tagged text. An ambiguous word was found: "<<word_tagged->get_superficial_form()<<L"\n";
    else
      tag1 = *(word_tagged->get_tags()).begin();


    if ((tag1>=0) && (tag2>=0))
      tags_pair[tag2][tag1]++;
    

    if (word_untagged->get_tags().size()==0) { // Unknown word
      tags = tdhmm->getOpenClass();
    }
    else if (output.has_not(word_untagged->get_tags())) { //We are training, there is no problem
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors+= L"Word '"+word_untagged->get_superficial_form()+L"' not found in the dictionary.\n";
      errors+= L"New ambiguity class: "+word_untagged->get_string_tags()+L"\n";
      errors+= L"Take a look at the dictionary, then retrain.";
      fatal_error(errors);      
    }    
    else {
      tags = word_untagged->get_tags();
    }

    k=output[tags];
    if(tag1>=0)
      emission[tag1][k]++;
                   
    delete word_tagged;
    word_tagged=stream_tagged.get_next_word();
    delete word_untagged;
    word_untagged=stream_untagged.get_next_word();       
  }
  
  
  //Estimate of a[i][j]
  for(i=0; i<N; i++) {
    double sum=0;
    for(j=0; j<N; j++)  
      sum += tags_pair[i][j]+1.0;
    for(j=0; j<N; j++)  
      (tdhmm->getA())[i][j] = (tags_pair[i][j]+1.0)/sum;
  }
    
  
  //Estimate of b[i][k]
  for(i=0; i<N; i++) {
    int nclasses_appear=0;
    double times_appear=0.0;
    for(k=0; k<M; k++)  {
      if (output[k].find(i)!=output[k].end())  {
	nclasses_appear++;	
	times_appear+=emission[i][k];
      }
    }	      
    for(k=0; k<M; k++)  {
      if (output[k].find(i)!=output[k].end())
	(tdhmm->getB())[i][k] = (emission[i][k]+(((double)1.0)/((double)nclasses_appear)))/(times_appear+((double)1.0));
    }
   }
  
  wcerr<<L"\n";  
}
  
void
HMM::apply_rules()
{
  vector<TForbidRule> &forbid_rules = tdhmm->getForbidRules();
  vector<TEnforceAfterRule> &enforce_rules = tdhmm->getEnforceRules();
  int N = tdhmm->getN();
  int i, j, j2;
  bool found;
   
  for(i=0; i<(int) forbid_rules.size(); i++) {
    (tdhmm->getA())[forbid_rules[i].tagi][forbid_rules[i].tagj] = ZERO;
  }

  for(i=0; i<(int) enforce_rules.size(); i++) {
    for(j=0; j<N; j++) {
      found = false;
      for (j2=0; j2<(int) enforce_rules[i].tagsj.size(); j2++) {
	if (enforce_rules[i].tagsj[j2]==j) {
	  found = true;
	  break;
	}	  
      }
      if (!found)
        (tdhmm->getA())[enforce_rules[i].tagi][j] = ZERO;
    }
  }
    
  // Normalize probabilities
  for(i=0; i<N; i++) {
    double sum=0;
    for(j=0; j<N; j++) 
      sum += (tdhmm->getA())[i][j];
    for(j=0; j<N; j++) {
      if (sum>0)
	(tdhmm->getA())[i][j] = (tdhmm->getA())[i][j]/sum;
      else
	(tdhmm->getA())[i][j] = 0;
    }
  }
}

void 
HMM::read_dictionary (FILE *fdic) {
  int i, k, nw=0;
  TaggerWord *word=NULL;
  set <TTag> tags;
  Collection &output = tdhmm->getOutput();
  
  MorphoStream morpho_stream(fdic, true, tdhmm);
  
  // In the input dictionary there must be all punctuation marks, including the end-of-sentece mark
   
  word = morpho_stream.get_next_word();
  
  while (word) {
    if (++nw%10000==0) wcerr<<L'.'<<flush;
    
    tags = word->get_tags();

    if (tags.size()>0)
      k = output[tags];

    delete word;
    word = morpho_stream.get_next_word();
  }
  wcerr<<L"\n";
  
  // OPEN AMBIGUITY CLASS
  // It contains all tags that are not closed.
  // Unknown words are assigned the open ambiguity class
  k=output[tdhmm->getOpenClass()];

  int N = (tdhmm->getTagIndex()).size();  
  
  // Create ambiguity class holding one single tag for each tag.
  // If not created yet
  for(i = 0; i != N; i++) {
    set<TTag> amb_class;
    amb_class.insert(i);
    k=output[amb_class];
  }

  int M = output.size();
  
  wcerr<< N <<L" states and "<< M <<L" ambiguity classes\n";
  tdhmm->setProbabilities(N, M);
}

void
HMM::filter_ambiguity_classes(FILE *in, FILE *out) {
  set<set<TTag> > ambiguity_classes;
  MorphoStream morpho_stream(in, true, tdhmm);
  
  TaggerWord *word = morpho_stream.get_next_word();
  
  while(word) {
    set<TTag> tags = word->get_tags();
    if(tags.size() > 0) {     
      if(ambiguity_classes.find(tags) == ambiguity_classes.end()) {
	    ambiguity_classes.insert(tags);
	    word->outputOriginal(out);
	    //wcerr<<word->get_string_tags()<<L"\n";
      }
    }
    delete word;
    word = morpho_stream.get_next_word();
  }
}

void 
HMM::train (FILE *ftxt) {
  int i, j, k, t, len, nw = 0;
  TaggerWord *word=NULL;
  TTag tag; 
  set<TTag> tags, pretags;
  set<TTag>::iterator itag, jtag;
  map <int, double> gamma;
  map <int, double>::iterator jt, kt;
  map < int, map <int, double> > alpha, beta, xsi, phi;
  map < int, map <int, double> >::iterator it;
  double prob, loli;              
  vector < set<TTag> > pending;
  Collection &output = tdhmm->getOutput();
  
  int ndesconocidas=0;
  // alpha => forward probabilities
  // beta  => backward probabilities
  
  MorphoStream morpho_stream(ftxt, true, tdhmm);

  loli = 0;
  tag = eos;
  tags.clear();
  tags.insert(tag);
  pending.push_back(tags);

  alpha[0].clear();      
  alpha[0][tag] = 1;

  word = morpho_stream.get_next_word();

  while (word) {   

    //wcerr<<L"Enter para continuar\n";
    //getchar();

    if (++nw%10000==0) wcerr<<L'.'<<flush;

    //wcerr<<*word<<L"\n";

    pretags = pending.back();

    tags = word->get_tags();    
    
    if (tags.size()==0) { // This is an unknown word
      tags = tdhmm->getOpenClass();
      ndesconocidas++;
    }
    
    if (output.has_not(tags)) {
      wstring errors;
      errors = L"A new ambiguity class was found. I cannot continue.\n";
      errors+= L"Word '"+word->get_superficial_form()+L"' not found in the dictionary.\n";
      errors+= L"New ambiguity class: "+word->get_string_tags()+L"\n";
      errors+= L"Take a look at the dictionary, then retrain.";
      fatal_error(errors);      
    }
    
    k = output[tags];    
    len = pending.size();
    alpha[len].clear();     
      
    //Forward probabilities
    for (itag=tags.begin(); itag!=tags.end(); itag++) {
      i=*itag;
      for (jtag=pretags.begin(); jtag!=pretags.end(); jtag++) {
         j=*jtag;
         //cerr<<"previous alpha["<<len<<"]["<<i<<"]="<<alpha[len][i]<<"\n";
	 //cerr<<"alpha["<<len-1<<"]["<<j<<"]="<<alpha[len-1][j]<<"\n";
         //cerr<<"a["<<j<<"]["<<i<<"]="<<a[j][i]<<"\n";
         //cerr<<"b["<<i<<"]["<<k<<"]="<<b[i][k]<<"\n";
	 alpha[len][i] += alpha[len-1][j]*(tdhmm->getA())[j][i]*(tdhmm->getB())[i][k];
      }
      if (alpha[len][i]==0)
        alpha[len][i]=DBL_MIN;
      //cerr<<"alpha["<<len<<"]["<<i<<"]="<<alpha[len][i]<<"\n--------\n";
    }

    if (tags.size()>1) {
      pending.push_back(tags);
    } else {  // word is unambiguous
      tag = *tags.begin(); 
      beta[0].clear();
      beta[0][tag] = 1;   
      
      prob = alpha[len][tag];
      
      //cerr<<"prob="<<prob<<"\n";
      //cerr<<"alpha["<<len<<"]["<<tag<<"]="<<alpha[len][tag]<<"\n";
      loli -= log(prob);  
      
      for (t=0; t<len; t++) {  // loop from T-1 to 0	
	  pretags = pending.back();
	  pending.pop_back();
   	  k = output[tags];
	     beta[1-t%2].clear();
	     for (itag=tags.begin(); itag!=tags.end(); itag++) {
	       i=*itag;
	       for (jtag=pretags.begin(); jtag!=pretags.end(); jtag++) {
	         j = *jtag;	      
	         beta[1-t%2][j] += (tdhmm->getA())[j][i]*(tdhmm->getB())[i][k]*beta[t%2][i];
	         xsi[j][i] += alpha[len-t-1][j]*(tdhmm->getA())[j][i]*(tdhmm->getB())[i][k]*beta[t%2][i]/prob;
	       }
	       double previous_value = gamma[i];
       
	       gamma[i] +=  alpha[len-t][i]*beta[t%2][i]/prob;		       
	       if (isnan(gamma[i])) {
	          wcerr<<L"NAN(3) gamma["<<i<<L"] = "<<gamma[i]<<L" alpha["<<len-t<<L"]["<<i<<L"]= "<<alpha[len-t][i]
	               <<L" beta["<<t%2<<L"]["<<i<<L"] = "<<beta[t%2][i]<<L" prob = "<<prob<<L" previous gamma = "<<previous_value<<L"\n";
	          exit(1);	               
	       }
	       if (isinf(gamma[i])) {
	          wcerr<<L"INF(3) gamma["<<i<<L"] = "<<gamma[i]<<L" alpha["<<len-t<<L"]["<<i<<L"]= "<<alpha[len-t][i]
	               <<L" beta["<<t%2<<L"]["<<i<<L"] = "<<beta[t%2][i]<<L" prob = "<<prob<<L" previous gamma = "<<previous_value<<L"\n";
	          exit(1);	               
	       }
	       if (gamma[i]==0) {
	          //cout<<"ZERO(3) gamma["<<i<<"] = "<<gamma[i]<<" alpha["<<len-t<<"]["<<i<<"]= "<<alpha[len-t][i]
	          //    <<" beta["<<t%2<<"]["<<i<<"] = "<<beta[t%2][i]<<" prob = "<<prob<<" previous gamma = "<<previous_value<<"\n";
	          gamma[i]=DBL_MIN;
	          //exit(1);	               
	       }
	        phi[i][k] += alpha[len-t][i]*beta[t%2][i]/prob;
	     }
	     tags=pretags;
      }
	
      tags.clear();
      tags.insert(tag);
      pending.push_back(tags);
      alpha[0].clear();
      alpha[0][tag] = 1;
    }
    
    delete word; 
    word = morpho_stream.get_next_word();
  }  

  if ((pending.size()>1) || ((tag!=eos)&&(tag != (tdhmm->getTagIndex())[L"TAG_kEOF"]))) 
    wcerr<<L"Warning: Thee las tag is not the end-of-sentence-tag\n";
  
  
  int N = tdhmm->getN();
  int M = tdhmm->getM();
  
  //Clean previous values  
  for(i=0; i<N; i++) {
     for(j=0; j<N; j++)
        (tdhmm->getA())[i][j]=ZERO;
     for(k=0; k<M; k++)
        (tdhmm->getB())[i][k]=ZERO;
  }
  
  // new parameters
  for (it=xsi.begin(); it!=xsi.end(); it++) {
    i = it->first;
    for (jt=xsi[i].begin(); jt!=xsi[i].end(); jt++) {
      j = jt->first;
      if (xsi[i][j]>0) {        
        if (gamma[i]==0) {
          wcerr<<L"Warning: gamma["<<i<<L"]=0\n";
          gamma[i]=DBL_MIN;
        }
        
        (tdhmm->getA())[i][j] = xsi[i][j]/gamma[i];
	
        if (isnan((tdhmm->getA())[i][j])) {
          wcerr<<L"NAN\n";
          wcerr <<L"Error: BW - NAN(1) a["<<i<<L"]["<<j<<L"]="<<(tdhmm->getA())[i][j]<<L"\txsi["<<i<<L"]["<<j<<L"]="<<xsi[i][j]<<L"\tgamma["<<i<<L"]="<<gamma[i]<<L"\n";
	  exit(1);
        }
	if (isinf((tdhmm->getA())[i][j])) {
	  wcerr<<L"INF\n"; 
          wcerr <<L"Error: BW - INF(1) a["<<i<<L"]["<<j<<L"]="<<(tdhmm->getA())[i][j]<<L"\txsi["<<i<<L"]["<<j<<L"]="<<xsi[i][j]<<L"\tgamma["<<i<<L"]="<<gamma[i]<<L"\n";
          exit(1);
        }
	if ((tdhmm->getA())[i][j]==0) {
          //cerr <<"Error: BW - ZERO(1) a["<<i<<"]["<<j<<"]="<<(tdhmm->getA())[i][j]<<"\txsi["<<i<<"]["<<j<<"]="<<xsi[i][j]<<"\tgamma["<<i<<"]="<<gamma[i]<<"\n";
	  //     exit(1);
        }
      }
    }
  }

  for (it=phi.begin(); it!=phi.end(); it++) {
    i = it->first;
    for (kt=phi[i].begin(); kt!=phi[i].end(); kt++) {
      k = kt->first;
      if (phi[i][k]>0) {
        (tdhmm->getB())[i][k] = phi[i][k]/gamma[i];	
        
	if (isnan((tdhmm->getB())[i][k])) {
          wcerr<<L"Error: BW - NAN(2) b["<<i<<L"]["<<k<<L"]="<<(tdhmm->getB())[i][k]<<L"\tphi["<<i<<L"]["<<k<<L"]="<<phi[i][k]<<L"\tgamma["<<i<<L"]="<<gamma[i]<<L"\n";
	       exit(1);
        }
	if (isinf((tdhmm->getB())[i][k])) {
          wcerr<<L"Error: BW - INF(2) b["<<i<<L"]["<<k<<L"]="<<(tdhmm->getB())[i][k]<<L"\tphi["<<i<<L"]["<<k<<L"]="<<phi[i][k]<<L"\tgamma["<<i<<L"]="<<gamma[i]<<L"\n";
	       exit(1);
        }
	if ((tdhmm->getB())[i][k]==0) {
          //cerr <<"Error: BW - ZERO(2) b["<<i<<"]["<<k<<"]="<<(tdhmm->getB())[i][k]<<"\tphi["<<i<<"]["<<k<<"]="<<phi[i][k]<<"\tgamma["<<i<<"]="<<gamma[i]<<"\n";
	  //     exit(1);
        }
      }
    }
  }

  //It can be possible that a probability is not updated
  //We normalize the probabilitites
  for(i=0; i<N; i++) {
    double sum=0;
    for(j=0; j<N; j++)
      sum+=(tdhmm->getA())[i][j];
    for(j=0; j<N; j++)
      (tdhmm->getA())[i][j]=(tdhmm->getA())[i][j]/sum;
  }

  for(i=0; i<N; i++) {
    double sum=0;
    for(k=0; k<M; k++) {
      if(output[k].find(i)!=output[k].end())
        sum+=(tdhmm->getB())[i][k];
    }
    for(k=0; k<M; k++) {
      if(output[k].find(i)!=output[k].end())
        (tdhmm->getB())[i][k]=(tdhmm->getB())[i][k]/sum;
    }
  }

  wcerr<<L"Log="<<loli<<L"\n";
}

void 
HMM::tagger(FILE *in, FILE *out, bool show_all_good_first) {
  int i, j, k, nw;
  TaggerWord *word=NULL;
  TTag tag;
  
  set <TTag> tags, pretags;
  set <TTag>::iterator itag, jtag;
  
  double prob, loli, x;
  int N = tdhmm->getN();  
#ifdef __GNUC__
  double alpha[2][N];
  vector<TTag> best[2][N];
#else
  vector <vector <double> > alpha(2, vector<double>(N));
  vector <vector <vector<TTag> > > best(2, vector <vector <TTag> >(N));
#endif
  
  vector <TaggerWord> wpend; 
  int nwpend;
  
  MorphoStream morpho_stream(in, debug, tdhmm);                             
  morpho_stream.setNullFlush(null_flush);
  
  Collection &output = tdhmm->getOutput();
  
  loli = nw = 0;
  
  //Initialization
  tags.insert(eos);
  alpha[0][eos] = 1;
   
  word = morpho_stream.get_next_word();
 
  while (word) {
    wpend.push_back(*word);    	    
    nwpend = wpend.size();
    
    pretags = tags; // Tags from the previous word

    tags = word->get_tags();
  
    if (tags.size()==0) // This is an unknown word
      tags = tdhmm->getOpenClass();
                       
    if (output.has_not(tags)) {  // Encontrada una clase de ambigüedad desconocida hasta el momento      
      if (debug) {
        wstring errors;
	errors = L"A new ambiguity class was found. \n";
	errors+= L"Retraining the tagger is necessary so as to take it into account.\n";
	errors+= L"Word '"+word->get_superficial_form()+L"'.\n";
	errors+= L"New ambiguity class: "+word->get_string_tags()+L"\n";
	wcerr<<L"Error: "<<errors;
      }
      tags = find_similar_ambiguity_class(tags);
    } 
         
    k = output[tags];  //Ambiguity class the word belongs to
    
#ifdef __GNUC__
    clear_array_double(alpha[nwpend%2], N);    
    clear_array_vector(best[nwpend%2], N);
#endif
    
    //Induction
    for (itag=tags.begin(); itag!=tags.end(); itag++) { //For all tag from the current word
      i=*itag;
      for (jtag=pretags.begin(); jtag!=pretags.end(); jtag++) {	//For all tags from the previous word
	j=*jtag;
	x = alpha[1-nwpend%2][j]*(tdhmm->getA())[j][i]*(tdhmm->getB())[i][k];
	if (alpha[nwpend%2][i]<=x) {
	  if (nwpend>1) 
	    best[nwpend%2][i] = best[1-nwpend%2][j];
	  best[nwpend%2][i].push_back(i);
	  alpha[nwpend%2][i] = x;
	}
      }
    }
    
    //Backtracking
    if (tags.size()==1) {       
      tag = *tags.begin();      
      
      prob = alpha[nwpend%2][tag];
      
      if (prob>0) 
	loli -= log(prob);
      else {
        if (debug)
	  wcerr<<L"Problem with word '"<<word->get_superficial_form()<<L"' "<<word->get_string_tags()<<L"\n";
      }
      for (unsigned t=0; t<best[nwpend%2][tag].size(); t++) {
	if (show_all_good_first) {
	  wstring const &micad = wpend[t].get_all_chosen_tag_first(best[nwpend%2][tag][t], (tdhmm->getTagIndex())[L"TAG_kEOF"]);
	  fputws_unlocked(micad.c_str(), out); 
	} else {
	  // print out
	  wpend[t].set_show_sf(show_sf);
	  wstring const &micad = wpend[t].get_lexical_form(best[nwpend%2][tag][t], (tdhmm->getTagIndex())[L"TAG_kEOF"]);
	  fputws_unlocked(micad.c_str(), out); 
	}
      }
      
      //Return to the initial state
      wpend.clear();   
      alpha[0][tag] = 1;
    }
    
    delete word;
    
    if(morpho_stream.getEndOfFile())
    {
      if(null_flush)
      { 
        fputwc_unlocked(L'\0', out);
      }
      
      fflush(out);
      morpho_stream.setEndOfFile(false);
    }
    word = morpho_stream.get_next_word();    
  }
  
  if ((tags.size()>1)&&(debug)) {
    wstring errors;
    errors = L"The text to disambiguate has finished, but there are ambiguous words that has not been disambiguated.\n";
    errors+= L"This message should never appears. If you are reading this ..... these are very bad news.\n";
    wcerr<<L"Error: "<<errors;
  }  
}


void
HMM::print_A() {
  int i,j;
    
  cout<<"TRANSITION MATRIX (A)\n------------------------------\n";  
  for(i=0; i != tdhmm->getN(); i++)
    for(j=0; j != tdhmm->getN(); j++) {
      cout<<"A["<<i<<"]["<<j<<"] = "<<(tdhmm->getA())[i][j]<<"\n";
    }    
}

void
HMM::print_B() {
  int i,k;  

  cout<<"EMISSION MATRIX (B)\n-------------------------------\n";
  for(i=0; i != tdhmm->getN(); i++)
    for(k=0; k != tdhmm->getM(); k++) {
      Collection &output = tdhmm->getOutput();
      if(output[k].find(i)!=output[k].end())
        cout<<"B["<<i<<"]["<<k<<"] = "<<(tdhmm->getB())[i][k]<<"\n";
    }
}

void HMM::print_ambiguity_classes() {
  set<TTag> ambiguity_class;
  set<TTag>::iterator itag;
  cout<<"AMBIGUITY CLASSES\n-------------------------------\n";
  for(int i=0; i != tdhmm->getM(); i++) {
    ambiguity_class = (tdhmm->getOutput())[i];
    cout <<i<<": ";
    for (itag=ambiguity_class.begin(); itag!=ambiguity_class.end(); itag++) {
      cout << *itag <<" ";
    }
    cout << "\n";
  }
}   

set<TTag>
HMM::find_similar_ambiguity_class(set<TTag> c) {
  int size_ret = -1;
  set<TTag> ret=tdhmm->getOpenClass(); //Se devolver� si no encontramos ninguna clase mejor
  bool skeep_class;
  Collection &output = tdhmm->getOutput();

  for(int k=0; k<output.size(); k++) {
    if ((((int)output[k].size())>((int)size_ret)) && (((int)output[k].size())<((int)c.size()))) {
      skeep_class=false;
      // Test if output[k] is a subset of class
      for(set<TTag>::const_iterator it=output[k].begin(); it!=output[k].end(); it++) {
        if (c.find(*it)==c.end()) { 
	   skeep_class=true; //output[k] is not a subset of class
	   break;
	}
      }
      if (!skeep_class) {
        size_ret = output[k].size();
	     ret = output[k];
      }
    }
  }
  return ret;
}

void
HMM::setNullFlush(bool nf)
{
  null_flush = nf;
}
