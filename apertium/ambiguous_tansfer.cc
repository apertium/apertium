#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <map>
#include <vector>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <sys/time.h>
#include <sstream>

#include <omp.h>

#include "ambiguous_transfer.h"
#include "case_handler.h"
#include "transfer_literals.h"

using namespace std;
using namespace elem;

void
AmbiguousTransfer::transfer (string transferFilePath, string modelsFileDest, string k,
			     FILE* lextorFile, FILE* outFile)
{

  xmlDoc* doc = xmlReadFile (transferFilePath.c_str (), NULL, 0);

  if (doc == NULL)
    {
      cerr << "Error: Could not parse file \'" << transferFilePath << "\'." << endl;
      exit (EXIT_FAILURE);
    }

  xmlNode* transfer = xmlDocGetRootElement (doc);

  map<string, vector<vector<string> > > attrs = AmbiguousChunker::getAttrs (transfer);
  map<string, string> vars = AmbiguousChunker::getVars (transfer);
  map<string, vector<string> > lists = AmbiguousChunker::getLists (transfer);

  map<string, map<string, vector<float> > > classesWeights = loadYasmetModels (
      modelsFileDest);

  int beam;
  stringstream buffer (k);
  buffer >> beam;

  char buff[10240];
  string tokenizedSentence;
  while (fgets (buff, 10240, lextorFile))
    {
      tokenizedSentence = buff;

      // spaces after each token
      vector<string> spaces;

      // tokens in the sentence order
      vector<string> slTokens, tlTokens;

      // tags of tokens in order
      vector<vector<string> > slTags, tlTags;

      AmbiguousChunker::lexFormsTokenizer (&slTokens, &tlTokens, &slTags, &tlTags,
					   &spaces, tokenizedSentence);

      // map of tokens ids and their matched categories
      map<unsigned, vector<string> > catsApplied;

      AmbiguousChunker::matchCats (&catsApplied, slTokens, slTags, transfer);

      // map of matched rules and a pair of first token id and patterns number
      map<xmlNode*, vector<pair<unsigned, unsigned> > > rulesApplied;

      AmbiguousChunker::matchRules (&rulesApplied, slTokens, catsApplied, transfer);

      // rule and (target) token map to specific output
      // if rule has many patterns we will choose the first token only
      map<unsigned, map<unsigned, string> > ruleOutputs;

      // map (target) token to all matched rules ids and the number of pattern items of each rule
      map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

      AmbiguousChunker::ruleOuts (&ruleOutputs, &tokenRules, slTokens, slTags, tlTokens,
				  tlTags, rulesApplied, attrs, lists, &vars, spaces);

      // final outputs
      vector<string> outs;
      // number of generated combinations
      unsigned compNum;
      // nodes for every token and rule
      map<unsigned, vector<AmbiguousChunker::Node> > nodesPool;
      // ambiguous informations
      vector<AmbiguousChunker::AmbigInfo> ambigInfo;
      // beam tree
      vector<pair<vector<AmbiguousChunker::Node>, float> > beamTree;
      // rules combinations
      vector<vector<AmbiguousChunker::Node> > combNodes;

      nodesPool = AmbiguousChunker::getNodesPool (tokenRules);

      AmbiguousChunker::getAmbigInfo (tokenRules, nodesPool, &ambigInfo, &compNum);

      vector<AmbiguousChunker::AmbigInfo> newAmbigInfo;
      for (unsigned j = 0; j < ambigInfo.size (); j++)
	if (ambigInfo[j].combinations.size () > 1)
	  newAmbigInfo.push_back (ambigInfo[j]);

      beamSearch (&beamTree, beam, slTokens, newAmbigInfo, classesWeights);

      AmbiguousChunker::getOuts (&outs, &combNodes, beamTree, nodesPool, ruleOutputs,
				 spaces);

      // write the outs
      for (unsigned j = 0; j < outs.size (); j++)
	{
	  fputs (outs[j].c_str (), outFile);
	}
    }

}

// to sort translations from best to worth by their weight
bool
sortParameter (pair<vector<AmbiguousChunker::Node>, float> a,
	       pair<vector<AmbiguousChunker::Node>, float> b)
{
  return (a.second > b.second);
}

void
beamSearch (vector<pair<vector<AmbiguousChunker::Node>, float> > *beamTree, unsigned beam,
	    vector<string> slTokens, vector<AmbiguousChunker::AmbigInfo> ambigInfo,
	    map<string, map<string, vector<float> > > classesWeights)
{
  // Initialization
  (*beamTree).push_back (pair<vector<AmbiguousChunker::Node>, float> ());

  for (unsigned i = 0; i < ambigInfo.size (); i++)
    {

      AmbiguousChunker::AmbigInfo ambig = ambigInfo[i];

      unsigned ambigRulesSize = ambig.combinations.size ();

      // name of the file is the concatenation of rules ids
      string rulesNums;
      for (unsigned x = 0; x < ambigRulesSize; x++)
	{
	  // avoid dummy node
	  for (unsigned y = 1; y < ambig.combinations[x].size (); y++)
	    {
	      stringstream ss;
	      ss << ambig.combinations[x][y].ruleId;
	      rulesNums += ss.str ();

	      if (y + 1 < ambig.combinations[x].size ())
		rulesNums += "_";
	    }
	  rulesNums += "+";
	}

      map<string, vector<float> > classWeights = classesWeights[(rulesNums + ".model")];

      // build new tree for the new words
      vector<pair<vector<AmbiguousChunker::Node>, float> > newTree;

      // initialize the new tree
      for (unsigned x = 0; x < ambigRulesSize; x++)
	{
	  newTree.push_back (
	      pair<vector<AmbiguousChunker::Node>, float> (
		  vector<AmbiguousChunker::Node> (), 0));
	}
      // put rules
      for (unsigned z = 0; z < ambigRulesSize; z++)
	{
	  for (unsigned y = 0; y < ambig.combinations[z].size (); y++)
	    {
	      newTree[z].first.push_back (ambig.combinations[z][y]);
	    }
	}

      for (unsigned x = ambig.firTokId; x < ambig.firTokId + ambig.maxPat; x++)
	{
	  // word key is the word and it's order in the rule
	  stringstream ss;
	  ss << x - ambig.firTokId;
	  string num = "_" + ss.str ();

	  // handle the case of two lemmas separated by a space
	  for (unsigned t = 0; t < slTokens[x].size (); t++)
	    if (slTokens[x][t] == ' ')
	      slTokens[x].replace (t, 1, "_");

	  string word = CaseHandler::toLowerCase (slTokens[x]) + num;
	  vector<float> wordWeights = classWeights[word];

	  // put weights
	  if (wordWeights.empty ())
	    {
	      for (unsigned z = 0; z < ambigRulesSize; z++)
		newTree[z].second += 1;
	      cout << "word : " << word << "  is not found in dataset : " << rulesNums
		  << endl;
	    }

	  else
	    for (unsigned z = 0; z < ambigRulesSize; z++)
	      newTree[z].second += wordWeights[z];

	}

      // expand beamTree
      unsigned initSize = beamTree->size ();
      for (unsigned z = 0; z < ambigRulesSize - 1; z++)
	{
	  for (unsigned x = 0; x < initSize; x++)
	    {
	      beamTree->push_back (
		  pair<vector<AmbiguousChunker::Node>, float> ((*beamTree)[x]));
	    }
	}

      // merge the two trees
      for (unsigned z = 0; z < ambigRulesSize; z++)
	{
	  for (unsigned x = initSize * z; x < initSize * (z + 1); x++)
	    {
	      // put the new rules with the old
	      (*beamTree)[x].first.insert ((*beamTree)[x].first.end (),
					   newTree[z].first.begin (),
					   newTree[z].first.end ());

	      // add their wiehgts
	      (*beamTree)[x].second += newTree[z].second;
	    }
	}

      // sort beam tree
      sort (beamTree->begin (), beamTree->end (), sortParameter);

      // remove elements more than (beam)
      if (beamTree->size () > beam)
	beamTree->erase (beamTree->begin () + beam, beamTree->end ());
    }

  // keep only the best sentence
  beamTree->erase (beamTree->begin () + 1, beamTree->end ());
}

map<string, map<string, vector<float> > >
loadYasmetModels (string modelsFilePath)
{
  // map with key yasmet model name and the value is
  // another map with key word name and the value is
  // vector of weights in order
  map<string, map<string, vector<float> > > classWeights;

  ifstream modelsFile ((modelsFilePath).c_str ());

  if (modelsFile.is_open ())
    {
      string line, model, token, weight;

      // localeid
//      getline (modelsFile, line);
//      *localeid = line;

      while (getline (modelsFile, line))
	{
	  // 0=>word , 1=>rule_num & 2=>wieght
	  // we don't need rule number , because
	  // the weights are already sorted

	  char lineChar[line.size ()];
	  strcpy (lineChar, line.c_str ());

	  token = strtok (lineChar, ": ");
	  if (token == "file")
	    {
	      model = strtok (NULL, ": ");
	      continue;
	    }
	  // skip rule_num
	  strtok (NULL, ": ");

	  weight = strtok (NULL, ": ");

	  float w = strtof (weight.c_str (), NULL);

	  classWeights[model][token].push_back (w);
	}
    }
  else
    {
      cout << "error in opening models file" << endl;
    }

  return classWeights;
}
