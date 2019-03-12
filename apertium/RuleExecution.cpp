/*
 * RuleExecution.cpp
 *
 *  Created on: May 5, 2018
 *      Author: aboelhamd
 */

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <algorithm>

#include "pugixml.hpp"
#include "TranElemLiterals.h"
#include "CLExec.h"

using namespace std;
using namespace pugi;
using namespace elem;

#include "RuleExecution.h"

void
putCombination (vector<vector<RuleExecution::Node> >* combinations,
		vector<RuleExecution::Node> combination)
{
  for (unsigned i = 0; i < combinations->size (); i++)
    (*combinations)[i].insert ((*combinations)[i].end (), combination.begin (),
			       combination.end ());
}

vector<vector<RuleExecution::Node> >
putCombinations (vector<vector<RuleExecution::Node> > combinations,
		 vector<vector<RuleExecution::Node> > nestedcombinations)
{
  vector<vector<RuleExecution::Node> > newcombinations;

  for (unsigned i = 0; i < combinations.size (); i++)
    {
      for (unsigned j = 0; j < nestedcombinations.size (); j++)
	{
	  vector<RuleExecution::Node> newcombination = vector<RuleExecution::Node> (
	      combinations[i]);
	  // +1 to skip dummy node
	  newcombination.insert (newcombination.end (),
				 nestedcombinations[j].begin () + 1,
				 nestedcombinations[j].end ());

	  newcombinations.push_back (newcombination);
	}
    }

  return newcombinations;
}

void
putOut (vector<string>* outputs, string output, unsigned tokenIndex,
	vector<string> spaces)
{
  for (unsigned i = 0; i < outputs->size (); i++)
    {
      (*outputs)[i] += output;
      (*outputs)[i] += (spaces[tokenIndex]);
    }
}

vector<string>
putOuts (vector<string> outputs, vector<string> nestedOutputs)
{
//  cout << endl << "nestedOutputs : " << tokenIndex << endl;
//  for (unsigned i = 0; i < nestedOutputs.size (); i++)
//    {
//      for (unsigned j = 0; j < nestedOutputs[i].size (); j++)
//	{
//	  cout << nestedOutputs[i][j];
//	}
//      cout << endl;
//    }

  vector<string> newOutputs;

  for (unsigned i = 0; i < outputs.size (); i++)
    {
      for (unsigned j = 0; j < nestedOutputs.size (); j++)
	{
	  string newOutput;

	  newOutput += outputs[i];
	  newOutput += nestedOutputs[j];
//	  newOutput.push_back (spaces[tokenIndex]);

	  newOutputs.push_back (newOutput);
	}
    }

  return newOutputs;
}

//vector<string>
//RuleExecution::getOuts (vector<vector<Node*> > ambigRules,
//			map<unsigned, map<unsigned, string> > ruleOutputs,
//			vector<string> spaces)
//{
//  vector<string> outs;
//
//  for (unsigned i = 0; i < ambigRules.size (); i++)
//    {
//      string out;
//      // skip first dummy node
//      for (unsigned j = 1; j < ambigRules[i].size (); j++)
//	{
//	  Node node = *ambigRules[i][j];
//	  out += ruleOutputs[node.ruleId][node.tokenId] + spaces[node.tokenId];
//	}
//      outs.push_back (out);
//    }
//
//  return outs;
//}

void
RuleExecution::getOuts (vector<string>* finalOuts, vector<vector<Node> >* finalCombNodes,
			vector<pair<vector<RuleExecution::Node>, float> > beamTree,
			map<unsigned, vector<RuleExecution::Node> > nodesPool,
			map<unsigned, map<unsigned, string> > ruleOutputs,
			vector<string> spaces)
{
  for (unsigned i = 0; i < beamTree.size (); i++)
    {
      map<unsigned, Node> bestNodes;
      for (unsigned j = 0; j < beamTree[i].first.size (); j++)
	{
//	  unsigned tokId = beamTree[i].first[j].tokenId;
//	  Node node = beamTree[i].first[j];
	  bestNodes[beamTree[i].first[j].tokenId] = beamTree[i].first[j];
//	  bestNodes.insert (pair<unsigned, Node> (tokId, node));
	}

      vector<Node> nodes;
      string out;
      for (unsigned j = 0; j < nodesPool.size ();)
	{
	  Node node;
	  if (bestNodes.count (j))
	    node = bestNodes[j];
	  else
	    node = nodesPool[j][0];

	  out += ruleOutputs[node.ruleId][node.tokenId]
	      + spaces[node.tokenId + node.patNum - 1];

	  nodes.push_back (node);

	  j += node.patNum;
	}

      finalCombNodes->push_back (nodes);
      finalOuts->push_back (out);
    }
}

void
RuleExecution::getOuts (vector<string>* finalOuts, vector<vector<Node> >* finalCombNodes,
			vector<RuleExecution::AmbigInfo> ambigInfo,
			map<unsigned, vector<RuleExecution::Node> > nodesPool,
			map<unsigned, map<unsigned, string> > ruleOutputs,
			vector<string> spaces)
{
  map<unsigned, AmbigInfo> ambigMap;
  for (unsigned i = 0; i < ambigInfo.size (); i++)
    {
      ambigMap.insert (pair<unsigned, AmbigInfo> (ambigInfo[i].firTokId, ambigInfo[i]));
    }

//  cout << ambigInfo.size () << endl;
//  cout << ambigInfo[0]->combinations.size () << endl;
  for (unsigned i = 0; (i < ambigInfo.size ()) || (i < 1); i++)
    {
      vector<vector<Node> > combNodes;
      combNodes.push_back (vector<Node> ());
      vector<string> outs;
      outs.push_back ("");

      for (unsigned j = 0; j < nodesPool.size ();)
	{
	  vector<RuleExecution::Node> nodes = nodesPool[j];

//	  cout << "i = " << i << " , curAmbig = " << curAmbig << " , j = " << j << endl;
	  if (nodes.size () > 1 && ambigMap.count (j))
	    {
	      vector<vector<Node> > combinations = ambigMap[j].combinations;

//	      cout << "comNum = " << combinations.size () << " , "
//		  << ambigInfo[i].firTokId << " ;; "
//		  << (combinations.size () > 1 && ambigInfo[i].firTokId == j) << endl;
//	      cout << "size= " << finalOuts->size () << endl;
//	      cout << "size= " << combinations[0].size () << endl;
	      if (ambigInfo[i].firTokId == j)
		{
		  combNodes = putCombinations (combNodes, combinations);

		  vector<string> ambigOuts;

		  for (unsigned k = 0; k < combinations.size (); k++)
		    {
		      string ambigOut;
		      // skip the dummy node
		      for (unsigned l = 1; l < combinations[k].size (); l++)
			{
			  ambigOut +=
			      ruleOutputs[combinations[k][l].ruleId][combinations[k][l].tokenId]
				  + spaces[combinations[k][l].tokenId
				      + combinations[k][l].patNum - 1];
//			  cout << i << " : " << j << " , " << ambigOut << endl;
			}
		      ambigOuts.push_back (ambigOut);
		    }

		  outs = putOuts (outs, ambigOuts);
		}
	      else
		{
		  putCombination (
		      &combNodes,
		      vector<Node> (combinations[0].begin () + 1,
				    combinations[0].end ()));
		  // take the first combination only , while solving the last space issue
		  string ambigOut;
		  // skip the dummy node
		  unsigned l = 1;
		  for (; l < combinations[0].size () - 1; l++)
		    {
		      ambigOut +=
			  ruleOutputs[combinations[0][l].ruleId][combinations[0][l].tokenId]
			      + spaces[combinations[0][l].tokenId
				  + combinations[0][l].patNum - 1];
//		      cout << i << " : " << j << " , " << ambigOut << endl;
		    }
		  ambigOut +=
		      ruleOutputs[combinations[0][l].ruleId][combinations[0][l].tokenId];
//		  cout << i << " : " << j << " , " << ambigOut << endl;
		  putOut (&outs, ambigOut,
			  combinations[0][l].tokenId + combinations[0][l].patNum - 1,
			  spaces);
		}

	      j += ambigMap[j].maxPat;
	    }
	  // make it else if nodes.size()==1
	  else
	    {
	      putCombination (&combNodes, nodes);
	      // put the method above this method
//	      cout << "here  " << nodes.size () << endl;
//	      cout << nodes[0]->tokenId << "  " << nodes[0]->patNum << "  "
//		  << spaces.size () << endl;
//	      cout << i << " : " << j << " , "
//		  << ruleOutputs[nodes[0].ruleId][nodes[0].tokenId] << endl;
	      putOut (&outs, ruleOutputs[nodes[0].ruleId][nodes[0].tokenId],
		      nodes[0].tokenId + nodes[0].patNum - 1, spaces);
	      j += nodes[0].patNum;
	    }
	}

      // put only different outputs
      for (unsigned j = 0; j < outs.size (); j++)
	{
	  if ((!ambigInfo.empty () && ambigInfo[i].combinations.size () > 1)
	      || find (finalOuts->begin (), finalOuts->end (), outs[j])
		  == finalOuts->end ())
	    {
	      finalOuts->push_back (outs[j]);
	      finalCombNodes->push_back (combNodes[j]);
	    }
	}
//      finalOuts->insert (finalOuts->end (), outs.begin (), outs.end ());
//      finalCombNodes->insert (finalCombNodes->end (), combNodes.begin (),
//			      combNodes.end ());
    }
}

void
RuleExecution::getCombinations (Node root, vector<Node> path,
				vector<vector<Node> >* ambigRules)
{
//  if (ambigRules->size () >= 100000)
//    return;

  path.push_back (root);

  for (unsigned i = 0; i < root.neighbors.size (); i++)
    getCombinations (root.neighbors[i], path, ambigRules);

  if (root.neighbors.empty ())
    {
      // if the rule0 in a combi   nation , don't count it
      for (unsigned i = 0; i < path.size (); i++)
	if (path[i].ruleId == 0)
	  return;

      ambigRules->push_back (path);
    }
}

vector<vector<float> >
RuleExecution::normaliseWeights (vector<vector<float> > vweights)
{
  vector<vector<float> > newvweights;

  for (unsigned i = 0; i < vweights.size (); i++)
    {
      vector<float> weights = vweights[i];

      // get sum of weights of an ambigInfo
      float sum = 0;
      for (unsigned k = 0; k < weights.size (); k++)
	{
	  sum += weights[k];
	}
      // Then normalize it
      for (unsigned k = 0; k < weights.size (); k++)
	{
	  // if sum=0 , to avoid nans we will make them all equal in weights
	  if (sum)
	    weights[k] /= sum;
	  else
	    weights[k] = 1 / weights.size ();
	}

      newvweights.push_back (weights);
    }
  return newvweights;
}

void
RuleExecution::normaliseWeights (vector<float>* weights)
{
  // get sum of weights of an ambigInfo
  float sum = 0;
  for (unsigned k = 0; k < weights->size (); k++)
    {
      sum += (*weights)[k];
    }
  // Then normalize it
  for (unsigned k = 0; k < weights->size (); k++)
    {
      // if sum=0 , to avoid nans we will make them all equal in weights
      if (sum)
	(*weights)[k] /= sum;
      else
	(*weights)[k] = 1 / weights->size ();
    }
}

void
RuleExecution::normaliseWeights (vector<vector<float> >* vweights,
				 vector<vector<RuleExecution::AmbigInfo> >* vambigInfo)
{
//  vector<vector<float> > newvweights;
//  cout << vambigInfo.size () << "  " << vweights->size () << endl;
  for (unsigned i = 0; i < vambigInfo->size (); i++)
    {
//      vector<float> weights = vweights[i];
      unsigned weigInd = 0;

      for (unsigned j = 0; j < (*vambigInfo)[i].size (); j++)
	{
	  // get sum of weights of an ambigInfo
	  float sum = 0;
	  for (unsigned k = 0; k < (*vambigInfo)[i][j].combinations.size (); k++)
	    {
	      sum += (*vweights)[i][weigInd + k];
	    }
	  // Then normalize it
	  for (unsigned k = 0; k < (*vambigInfo)[i][j].combinations.size (); k++)
	    {
	      // if sum=0 , to avoid nans we will make them all equal in weights
	      if (sum)
		(*vweights)[i][weigInd + k] /= sum;
	      else
		(*vweights)[i][weigInd + k] = 1
		    / (*vambigInfo)[i][j].combinations.size ();
	    }

	  // update weighInd
	  weigInd += (*vambigInfo)[i][j].combinations.size ();
	}
//      newvweights.push_back (weights);
    }
//  return vweights;
}

void
RuleExecution::normaliseWeights (vector<float>* weights,
				 vector<RuleExecution::AmbigInfo> ambigInfo)
{
//  vector<vector<float> > newvweights;
//  cout << vambigInfo.size () << "  " << vweights->size () << endl;
//  for (unsigned i = 0; i < vambigInfo->size (); i++)
//    {
//      vector<float> weights = vweights[i];
  unsigned weigInd = 0;

  for (unsigned j = 0; j < ambigInfo.size (); j++)
    {
      // get sum of weights of an ambigInfo
      float sum = 0;
      for (unsigned k = 0; k < ambigInfo[j].combinations.size (); k++)
	{
	  sum += (*weights)[weigInd + k];
	}
      // Then normalize it
      for (unsigned k = 0; k < ambigInfo[j].combinations.size (); k++)
	{
	  // if sum=0 , to avoid nans we will make them all equal in weights
	  if (sum)
	    (*weights)[weigInd + k] /= sum;
	  else
	    (*weights)[weigInd + k] = 1 / ambigInfo[j].combinations.size ();
	}

      // update weighInd
      weigInd += ambigInfo[j].combinations.size ();
    }
//      newvweights.push_back (weights);
//    }
//  return vweights;
}

//void
//RuleExecution::getCombinations (Node root, vector<pair<unsigned, unsigned> > path,
//				vector<vector<pair<unsigned, unsigned> > >* ambigRules)
//{
//  if (ambigRules->size () >= 1000000)
//    return;
//
//  path.push_back (pair<unsigned, unsigned> (root.tokenId, root.ruleId));
//
//  for (unsigned i = 0; i < root.neighbors.size (); i++)
//    getCombinations (*root.neighbors[i], path, ambigRules);
//
//  if (root.neighbors.empty ())
//    {
//      // remove the first dummy node
//      path.erase (path.begin ());
//      ambigRules->push_back (path);
//    }
//}

void
getMaxPat (int curMaxPat, unsigned curToken,
	   map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules, unsigned* count)
{
  if (curMaxPat == 0)
    return;

  int maxPat = 0;
  vector<pair<unsigned, unsigned> > rules = tokenRules[curToken];

  if (rules.size ())
    maxPat = rules[0].second;
//  for (unsigned i = 0; i < rules.size (); i++)
//    {
//      for (unsigned j = curToken; j < curToken + maxPat; j++)
//	{
//	  for (unsigned k = 0; k < tokenRules[j].size (); k++)
//	    {
//	      if (rules[i].first == tokenRules[j][k].first)
//		{
//		  tokenRules[j].erase (tokenRules[j].begin () + k);
//		  break;
//		}
//	    }
//	}
//    }

  (*count)++;
//  return max (0, maxPat - curMaxPat)
//      + getMaxPat (max (curMaxPat - 1, maxPat - curMaxPat), curToken + 1, tokenRules);
  getMaxPat (max (curMaxPat - 1, maxPat - curMaxPat), curToken + 1, tokenRules, count);
}

void
RuleExecution::getAmbigInfo (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
			     map<unsigned, vector<RuleExecution::Node> > nodesPool,
			     vector<RuleExecution::AmbigInfo>* ambigInfo,
			     unsigned* combNum)
{
  *combNum = 0;
  for (unsigned tokId = 0; tokId < tokenRules.size ();)
    {
      unsigned maxPat = 0;
      vector<pair<unsigned, unsigned> > rules = tokenRules[tokId];
      getMaxPat (rules[0].second, tokId, tokenRules, &maxPat);

      // if there is ambiguity
      if (nodesPool[tokId].size () > 1)
	{
	  AmbigInfo ambig = AmbigInfo (tokId, maxPat);

	  Node dummy = ambiguousGraph (tokenRules, nodesPool, tokId, maxPat);
	  getCombinations (dummy, vector<Node> (), &ambig.combinations);

	  if (!ambig.combinations.empty ())
	    ambigInfo->push_back (ambig);

	  *combNum += ambig.combinations.size ();
	}
      tokId += maxPat;
    }
}

map<unsigned, vector<RuleExecution::Node> >
RuleExecution::getNodesPool (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules)
{
  map<unsigned, vector<Node> > nodesPool;
  for (map<unsigned, vector<pair<unsigned, unsigned> > >::iterator it =
      tokenRules.begin (); it != tokenRules.end (); it++)
    {
      unsigned tokenId = it->first;
      vector<pair<unsigned, unsigned> > rules = it->second;
      for (unsigned i = 0; i < rules.size (); i++)
	{
	  unsigned ruleId = rules[i].first;
	  unsigned patNum = rules[i].second;
	  Node node = Node (tokenId, ruleId, patNum);
	  nodesPool[tokenId].push_back (node);
	}
    }
  return nodesPool;
}

RuleExecution::Node
RuleExecution::ambiguousGraph (
    map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
    map<unsigned, vector<Node> > nodesPool, unsigned firTok, unsigned maxPat)
{
  for (unsigned i = firTok; i < firTok + maxPat; i++)
    {
      vector<Node> nodes = nodesPool[i];
      for (unsigned j = 0; j < nodes.size (); j++)
	{
	  Node node = nodes[j];
	  // last nodes will point to nothing
	  if (node.tokenId + node.patNum < firTok + maxPat)
	    node.neighbors = nodesPool[node.tokenId + node.patNum];

	  nodes[j] = node;
	}
      nodesPool[i] = nodes;
    }

  // root(dummy) node points to the first token node/s
  Node root = Node (-1, -1, -1);
  root.neighbors = nodesPool[firTok];
  return root;
}

RuleExecution::Node
RuleExecution::ambiguousGraph (
    map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
    map<unsigned, vector<Node> > nodesPool)
{
  for (unsigned i = 0; i < nodesPool.size (); i++)
    {
      vector<Node> nodes = nodesPool[i];
      for (unsigned j = 0; j < nodes.size (); j++)
	{
	  Node node = nodes[j];
	  // last nodes will point to not existent nodes
	  if (nodesPool.count (node.tokenId + node.patNum))
	    node.neighbors = nodesPool[node.tokenId + node.patNum];

	  nodes[j] = node;
	}
      nodesPool[i] = nodes;
    }

  // root(dummy) node points to the first token node/s
  Node root = Node (-1, -1, -1);
  root.neighbors = nodesPool[0];
  return root;
}

// to sort rules in tokenRules descendingly by their number of pattern items
bool
sortParameter (pair<unsigned, unsigned> a, pair<unsigned, unsigned> b)
{
  return (a.second > b.second);
}

string
RuleExecution::noRuleOut (vector<string> analysis)
{
  vector<string> out;
  // unknown word
  if (analysis[0][0] == '*')
    out.push_back ("^unknown<unknown>{^");
  else
    out.push_back ("^default<default>{^");
  out.insert (out.end (), analysis.begin (), analysis.end ());
  out.push_back ("$}$");

  string str;
  for (unsigned i = 0; i < out.size (); i++)
    str += out[i];

  return str;
}

void
nestedRules (vector<string> tlTokens, string output,
	     vector<pair<unsigned, unsigned> > curNestedRules, vector<string>* outputs,
	     vector<vector<pair<unsigned, unsigned> > >* nestedOutsRules,
	     map<unsigned, map<unsigned, string> > ruleOuts,
	     map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
	     vector<string> spaces, unsigned curPatNum, unsigned curTokIndex)
{
  vector<pair<unsigned, unsigned> > rulesApplied = tokenRules[curTokIndex];

//  cout << "enter nestedrules , curTok = " << curTokIndex << " , rules applied size = "
//      << rulesApplied.size () << " , curPatNum = " << curPatNum << endl;

  for (unsigned i = 0; i < rulesApplied.size (); i++)
    {
      unsigned rule = rulesApplied[i].first;
      unsigned patNum = rulesApplied[i].second;

//      cout << "curTok = " << curTokIndex << " , rule id = " << rule << " , patNum = "
//	  << patNum << " , applied = " << (patNum <= curPatNum) << endl;

      if (patNum <= curPatNum)
	{
	  vector<pair<unsigned, unsigned> > newCurNestedRules = vector<
	      pair<unsigned, unsigned> > (curNestedRules);
	  newCurNestedRules.push_back (pair<unsigned, unsigned> (rule, curTokIndex));

	  string newOutput = output;
//
	  string ruleOut = ruleOuts[rule][curTokIndex];
//	  cout << curTokIndex << " " << patNum << endl;
	  ruleOut += spaces[curTokIndex + patNum - 1];
//
	  newOutput += ruleOut;

// remove that rule from all tokens

	  tokenRules[curTokIndex].erase (tokenRules[curTokIndex].begin ());
//	  for (unsigned j = curTokIndex; j < curTokIndex + patNum; j++)
//	    {
////	      cout << "tokenrules " << j << " size = " << tokenRules[j].size () << endl;
//	      tokenRules[j].erase (tokenRules[j].begin ());
//	    }

	  if (curPatNum == patNum)
	    {
	      (*outputs).push_back (newOutput);
	      (*nestedOutsRules).push_back (newCurNestedRules);
	    }
	  else
	    {
	      nestedRules (tlTokens, output, newCurNestedRules, outputs, nestedOutsRules,
			   ruleOuts, tokenRules, spaces, curPatNum - patNum,
			   curTokIndex + patNum);
	    }
	}
    }

//  for (unsigned i = curTokIndex + 1; i < curTokIndex + curPatNum; i++)
//    {
//      vector<pair<unsigned, unsigned> > rulesApplied = tokenRules[i];
//      if (rulesApplied.size ())
//	{
//	  // longest rule
//	  unsigned maxPatterns = 0;
//
//	  getMaxPat (rulesApplied[0].second, i, tokenRules, &maxPatterns);
//
//	  nestedRules (tlTokens, string (), vector<pair<unsigned, unsigned> > (), outputs,
//		       nestedOutsRules, ruleOuts, tokenRules, spaces, maxPatterns, i);
//
//	  break;
//	}
//    }
//  cout << "out tokeId = " << curTokIndex << endl;
//  cout << endl;
}

vector<string>
vectorToString (vector<vector<string> > vOutputs)
{
  vector<string> sOutputs;

  for (unsigned i = 0; i < vOutputs.size (); i++)
    {
      string output = "";
      for (unsigned j = 0; j < vOutputs[i].size (); j++)
	{
	  output += vOutputs[i][j];
	}
      sOutputs.push_back (output);

      //free memory
//      vOutputs[i].clear ();
    }

  return sOutputs;
}

vector<vector<unsigned> >
ruleVectorToIds (vector<vector<vector<xml_node> > > vOutRules)
{
  vector<vector<unsigned> > sOutRules;

  for (unsigned i = 0; i < vOutRules.size (); i++)
    {
      vector<unsigned> output;
      for (unsigned j = 0; j < vOutRules[i].size (); j++)
	{
	  for (unsigned k = 0; k < vOutRules[i][j].size (); k++)
	    {
	      // we want the id only (after the last '-')
	      output.push_back (vOutRules[i][j][k].attribute (ID).as_int ());
	    }
	  // free memory
//	  vOutRules[i][j].clear ();
	}
      // free memory
//      vOutRules[i].clear ();

      sOutRules.push_back (output);
    }

  return sOutRules;
}

vector<vector<vector<unsigned> > >
putRules (vector<vector<vector<unsigned> > > outsRules,
	  vector<vector<unsigned> > nestedOutsRules)
{
  vector<vector<vector<unsigned> > > newRules;

  for (unsigned i = 0; i < outsRules.size (); i++)
    {
      for (unsigned j = 0; j < nestedOutsRules.size (); j++)
	{
	  vector<vector<unsigned> > newRule = vector<vector<unsigned> > (outsRules[i]);

	  newRule.push_back (nestedOutsRules[j]);

	  newRules.push_back (newRule);

	  // free memory
//	  nestedOutsRules[j].clear();
	}

      // free memory
//      outsRules[i].clear ();
    }
//  cout << "inside putrules : " << outsRules.size () << "  " << nestedOutsRules.size ()
//      << "  " << newRules.size () << endl;
  return newRules;
}

vector<vector<pair<unsigned, unsigned> > >
putNestedRules (vector<vector<pair<unsigned, unsigned> > > outsRules,
		vector<vector<pair<unsigned, unsigned> > > nestedOutsRules)
{
//  cout << "outsrule size = " << outsRules.size () << endl;
//  cout << "nestedoutsrule size = " << nestedOutsRules.size () << endl;
  vector<vector<pair<unsigned, unsigned> > > newRules;

  for (unsigned i = 0; i < outsRules.size (); i++)
    {
      for (unsigned j = 0; j < nestedOutsRules.size (); j++)
	{
	  vector<pair<unsigned, unsigned> > newRule = vector<pair<unsigned, unsigned> > (
	      outsRules[i]);

	  newRule.insert (newRule.end (), nestedOutsRules[j].begin (),
			  nestedOutsRules[j].end ());

	  newRules.push_back (newRule);

	  // free memory
//	  nestedOutsRules[j].clear ();
	}

      // free memory
//      outsRules[i].clear ();
    }

  return newRules;
}

//void
//RuleExecution::getAmbigInfo (
//    vector<vector<vector<unsigned> > >* ambigRulIdsCombs,
//    map<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > >* ambigInfo,
//    vector<pair<unsigned, unsigned> > maxPatts,
//    vector<vector<RuleExecution::Node*> > tokRulIdsCombs)
//{
//  for (unsigned i = 0; i < tokRulIdsCombs.size (); i++)
//    {
//      vector<vector<unsigned> > ambigRulIdsComb;
//
//      vector<RuleExecution::Node*> tokRulIdsComb = tokRulIdsCombs[i];
//
//      for (unsigned j = 0; j < maxPatts.size (); j++)
//	{
//	  vector<unsigned> ambigRulIds;
//
//	  bool ambiguous = false;
//	  // start from k = 1 because the first node is dummy
//	  for (unsigned k = 1; k < tokRulIdsComb.size (); k++)
//	    {
//	      // check the node before this current node to
//	      // see if this current token has ambigous rules
//	      if (tokRulIdsComb[k]->tokenId == maxPatts[j].first)
//		ambiguous = tokRulIdsComb[k - 1]->neighbors.size () > 1;
//
//	      // we want the token ids = current maxpat
//	      if (tokRulIdsComb[k]->tokenId >= maxPatts[j].first
//		  && tokRulIdsComb[k]->tokenId < maxPatts[j].first + maxPatts[j].second)
//		ambigRulIds.push_back (tokRulIdsComb[k]->ruleId);
//
//	      else if (tokRulIdsComb[k]->tokenId
//		  == maxPatts[j].first + maxPatts[j].second)
//		break;
//	    }
//
//	  if (ambiguous)
//	    {
//	      (*ambigInfo)[pair<unsigned, unsigned> (maxPatts[j].first,
//						     maxPatts[j].second)].first =
//		  ambigRulIdsComb.size ();
//	      (*ambigInfo)[pair<unsigned, unsigned> (maxPatts[j].first,
//						     maxPatts[j].second)].second.push_back (
//		  ambigRulIds);
////	      cout << "here size = " << ambigRulIds.size () << endl;
//	    }
//
//	  ambigRulIdsComb.push_back (ambigRulIds);
//	}
//
//      ambigRulIdsCombs->push_back (ambigRulIdsComb);
//    }
//}

bool
RuleExecution::outputs (
    vector<string>* outs,
    vector<vector<pair<unsigned, unsigned> > >* rulesIds,
    vector<vector<vector<unsigned> > >* outsRules,
    vector<pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > > *ambigInfo,
    vector<string> tlTokens, vector<vector<string> > tags,
    map<unsigned, map<unsigned, string> > ruleOuts,
    map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules, vector<string> spaces)
{
  // position of ambiguous rule among other ones
  int ambigPos = -1;

  outs->push_back (string ());
  rulesIds->push_back (vector<pair<unsigned, unsigned> > ());
  outsRules->push_back (vector<vector<unsigned> > ());

  for (unsigned i = 0; i < tlTokens.size ();)
    {
      if (outs->size () > 16384)
	return false;
//	{
//	  (*outs) = vectorToString (outputs);
//	  (*rulesIds) = ruleVectorToIds (*outsRules);
//	  return;
//	}
      vector<pair<unsigned, unsigned> > rulesApplied = tokenRules[i];

//      if (rulesApplied.empty ())
//	{
//	  string defOut = noRuleOut (formatTokenTags (tlTokens[i], tags[i]));
//
//	  putOut (outs, defOut, i, spaces);
//
//	  i++;
//	}
//      else
//	{
      ambigPos++;

      // longest rule
      unsigned maxPatterns = 0;

      getMaxPat (rulesApplied[0].second, i, tokenRules, &maxPatterns);

      //cout << "curToken = " << i << "  maxPattern = " << maxPatterns << endl;

      vector<string> nestedOutputs;
      vector<vector<pair<unsigned, unsigned> > > nestedOutsRules;

      nestedRules (tlTokens, string (), vector<pair<unsigned, unsigned> > (),
		   &nestedOutputs, &nestedOutsRules, ruleOuts, tokenRules, spaces,
		   maxPatterns, i);

      vector<vector<unsigned> > nestedRules;
      for (unsigned j = 0; j < nestedOutsRules.size (); j++)
	{
	  vector<unsigned> nestedRule;
	  for (unsigned k = 0; k < nestedOutsRules[j].size (); k++)
	    nestedRule.push_back (nestedOutsRules[j][k].first);
	  nestedRules.push_back (nestedRule);
	}

      // if there is ambiguity save the info
      if (nestedOutsRules.size () > 1)
	{
	  ambigInfo->push_back (
	      pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > (
		  pair<unsigned, unsigned> (i, maxPatterns),
		  pair<unsigned, vector<vector<unsigned> > > (ambigPos, nestedRules)));
	}

      (*outs) = putOuts ((*outs), nestedOutputs/*, i + maxPatterns - 1, spaces*/);
      (*rulesIds) = putNestedRules ((*rulesIds), nestedOutsRules);
      (*outsRules) = putRules (*outsRules, nestedRules);

      i += maxPatterns;
//	}
    }

  return true;
//  (*outs) = vectorToString (outputs);
//  (*rulesIds) = ruleVectorToIds (*outsRules);
}

void
RuleExecution::weightIndices (
    vector<vector<unsigned> >* weigInds,
    vector<pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > > ambigInfo,
    vector<vector<vector<unsigned> > > outsRules)
{
  for (unsigned i = 0; i < ambigInfo.size (); i++)
    {
      pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > p1 =
	  ambigInfo[i];
      vector<vector<unsigned> > ambigRules = p1.second.second;
      unsigned rulePos = p1.second.first;

      for (unsigned x = 0; x < ambigRules.size (); x++)
	{
	  vector<unsigned> weigInd;
	  for (unsigned z = 0; z < outsRules.size (); z++)
	    {
	      vector<unsigned> ambigRule = outsRules[z][rulePos];

	      bool same = true;

	      if (ambigRule.size () == ambigRules[x].size ())
		{
		  for (unsigned ind = 0; ind < ambigRule.size (); ind++)
		    {
		      if (ambigRule[ind] != ambigRules[x][ind])
			{
			  same = false;
			  break;
			}
		    }
		}
	      else
		{
		  same = false;
		}

	      if (same)
		weigInd.push_back (z);
	    }
	  weigInds->push_back (weigInd);
	}
    }

}

void
pushDistinct (map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
	      unsigned tlTokInd, xml_node rule, unsigned patNum)
{
  vector<pair<unsigned, unsigned> > pairs = (*tokenRules)[tlTokInd];
  for (unsigned i = 0; i < pairs.size (); i++)
    {
      if (pairs[i].first == rule.attribute (ID).as_uint ())
	return;
    }

  (*tokenRules)[tlTokInd].push_back (
      pair<unsigned, unsigned> (rule.attribute (ID).as_uint (), patNum));

  sort ((*tokenRules)[tlTokInd].begin (), (*tokenRules)[tlTokInd].end (), sortParameter);
}

void
printNodeAttrs (xml_node node)
{
//  cout << node.name () << endl;
//  for (xml_node::attribute_iterator it = node.attributes_begin ();
//      it != node.attributes_end (); it++)
//    {
//      cout << it->name () << "=" << it->value () << "; ";
//    }
//  cout << endl << endl;
}

void
RuleExecution::ruleOuts (map<unsigned, map<unsigned, string> >* ruleOuts,
			 map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
			 vector<string> slTokens, vector<vector<string> > slTags,
			 vector<string> tlTokens, vector<vector<string> > tlTags,
			 map<xml_node, vector<pair<unsigned, unsigned> > > rulesApplied,
			 map<string, vector<vector<string> > > attrs,
			 map<string, vector<string> > lists, map<string, string>* vars,
			 vector<string> spaces, string localeId)
{
  //cout << "Inside  " << "ruleOuts" << endl;

  for (map<xml_node, vector<pair<unsigned, unsigned> > >::iterator it =
      rulesApplied.begin (); it != rulesApplied.end (); ++it)
    {
      xml_node rule = it->first;
      for (unsigned i = 0; i < rulesApplied[rule].size (); i++)
	{
	  vector<vector<string> > slAnalysisTokens, tlAnalysisTokens;

	  // format tokens and their tags into analysisTokens

	  unsigned firstMatTok = rulesApplied[rule][i].first;
	  unsigned patNum = rulesApplied[rule][i].second;

	  for (unsigned tokInd = firstMatTok; tokInd < firstMatTok + patNum; tokInd++)
	    {
	      vector<string> slAnalysisToken = RuleExecution::formatTokenTags (
		  slTokens[tokInd], slTags[tokInd]);

	      slAnalysisTokens.push_back (slAnalysisToken);

	      vector<string> tlAnalysisToken = RuleExecution::formatTokenTags (
		  tlTokens[tokInd], tlTags[tokInd]);

	      tlAnalysisTokens.push_back (tlAnalysisToken);

//	      cout << rule.attribute (ID).as_uint () << " " << tokInd << " "
//		  << tlTokens[tokInd] << "   " << noRuleOut (tlAnalysisToken) << endl;

	    }
	  // insert the rule (if not found) then sort the vector
	  pushDistinct (tokenRules, firstMatTok, rule, patNum);

	  vector<string> output;
	  if (rule.attribute (ID).as_uint () == 0)
	    output.push_back (noRuleOut (tlAnalysisTokens[0]));
	  else
	    output = RuleExecution::ruleExe (rule, &slAnalysisTokens, &tlAnalysisTokens,
					     attrs, lists, vars, spaces, firstMatTok,
					     localeId); // first pattern index

	  string str;
	  for (unsigned j = 0; j < output.size (); j++)
	    str += output[j];

	  (*ruleOuts)[rule.attribute (ID).as_uint ()][firstMatTok] = str;
	}
    }

}

vector<string>
RuleExecution::ruleExe (xml_node rule, vector<vector<string> >* slAnalysisTokens,
			vector<vector<string> >* tlAnalysisTokens,
			map<string, vector<vector<string> > > attrs,
			map<string, vector<string> > lists, map<string, string>* vars,
			vector<string> spaces, unsigned firPat, string localeId)
{
  //cout << "Inside  " << "rule : " << rule.attribute (ID).value () << endl;

  printNodeAttrs (rule);

  vector<string> output;

  map<unsigned, unsigned> paramToPattern = map<unsigned, unsigned> ();

  for (xml_node child = rule.child (ACTION).first_child (); child;
      child = child.next_sibling ())
    {
      vector<string> result;

      string childName = child.name ();
      if (childName == LET)
	{
	  let (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
	       localeId, paramToPattern);
	}
      else if (childName == CHOOSE)
	{
	  result = choose (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == CALL_MACRO)
	{
	  result = callMacro (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists,
			      vars, spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == OUT)
	{
	  result = out (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			firPat, localeId, paramToPattern);
	}

      else if (childName == MODIFY_CASE)
	modifyCase (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		    firPat, localeId, paramToPattern);

      else if (childName == APPEND)
	append (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
		localeId, paramToPattern);

      output.insert (output.end (), result.begin (), result.end ());
    }

  return output;
}

vector<string>
RuleExecution::out (xml_node out, vector<vector<string> >* slAnalysisTokens,
		    vector<vector<string> >* tlAnalysisTokens,
		    map<string, vector<vector<string> > > attrs,
		    map<string, string>* vars, vector<string> spaces, unsigned firPat,
		    string localeId, map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "out" << endl;
  printNodeAttrs (out);

  vector<string> output;

  for (xml_node child = out.first_child (); child; child = child.next_sibling ())
    {
      vector<string> result;

      string childName = child.name ();
      if (childName == B)
	{
	  result.push_back (b (child, spaces, firPat, localeId, paramToPattern));
	}
      else if (childName == CHUNK)
	{
	  result = chunk (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			  firPat, localeId, paramToPattern);
	}

//      cout << "result : ";
//      for (unsigned i = 0; i < result.size (); i++)
//	{
//	  cout << result[i];
//	}
//      cout << endl;

      output.insert (output.end (), result.begin (), result.end ());
    }

  return output;
}

vector<string>
RuleExecution::chunk (xml_node chunkNode, vector<vector<string> >* slAnalysisTokens,
		      vector<vector<string> >* tlAnalysisTokens,
		      map<string, vector<vector<string> > > attrs,
		      map<string, string>* vars, vector<string> spaces, unsigned firPat,
		      string localeId, map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "chunk" << endl;

  printNodeAttrs (chunkNode);

  vector<string> output;

  output.push_back ("^");

  string name = chunkNode.attribute (NAME).value ();
  if (name.empty ())
    {
      string varName = chunkNode.attribute (NAME_FROM).value ();
      name = (*vars)[varName];
    }

  output.push_back (name);

  // tags element must be first item
  vector<vector<string> > tagsResult;
  xml_node tags = chunkNode.child (TAGS);
  for (xml_node tag = tags.child (TAG); tag; tag = tag.next_sibling ())
    {
      printNodeAttrs (tag);

      vector<string> result;

      xml_node child = tag.first_child ();
      string childName = child.name ();

      if (childName == CLIP)
	{
	  result = clip (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern);
	}
      else if (childName == CONCAT)
	{
	  result = concat (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			   firPat, localeId, paramToPattern);
	}
      else if (childName == LIT_TAG)
	{
	  result = litTag (child);
	}
      else if (childName == LIT)
	{
	  result.push_back (lit (child));
	}
      else if (childName == B)
	{
	  result.push_back (b (child, spaces, firPat, localeId, paramToPattern));
	}
      else if (childName == CASE_OF)
	{
	  result.push_back (
	      caseOf (child, slAnalysisTokens, tlAnalysisTokens, localeId,
		      paramToPattern));
	}
      else if (childName == GET_CASE_FROM)
	{
	  result.push_back (
	      getCaseFrom (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			   firPat, localeId, paramToPattern));
	}
      else if (childName == VAR)
	{
	  result.push_back (var (child, vars));
	}

      tagsResult.push_back (result);
      output.insert (output.end (), result.begin (), result.end ());
    }

  output.push_back ("{");

  for (xml_node child = tags.next_sibling (); child; child = child.next_sibling ())
    {
//      printNodeAttrs (child);
      string childName = child.name ();
      if (childName == LU)
	{
	  // lu is the same as concat
	  vector<string> result = concat (child, slAnalysisTokens, tlAnalysisTokens,
					  attrs, vars, spaces, firPat, localeId,
					  paramToPattern, tagsResult);
	  output.push_back ("^");
	  output.insert (output.end (), result.begin (), result.end ());
	  output.push_back ("$");
	}
      else if (childName == MLU)
	{
	  output.push_back ("^");
	  // has only lu children
	  for (xml_node lu = child.first_child (); lu; lu = lu.next_sibling ())
	    {
	      vector<string> result = concat (lu, slAnalysisTokens, tlAnalysisTokens,
					      attrs, vars, spaces, firPat, localeId,
					      paramToPattern, tagsResult);
	      output.insert (output.end (), result.begin (), result.end ());

	      if (lu.next_sibling ())
		output.push_back ("+");
	    }
	  output.push_back ("$");
	}
      else if (childName == B)
	{
	  output.push_back (b (child, spaces, firPat, localeId, paramToPattern));
	}
    }

  output.push_back ("}$");

  return output;
}

vector<string>
RuleExecution::callMacro (xml_node callMacroNode,
			  vector<vector<string> >* slAnalysisTokens,
			  vector<vector<string> >* tlAnalysisTokens,
			  map<string, vector<vector<string> > > attrs,
			  map<string, vector<string> > lists, map<string, string>* vars,
			  vector<string> spaces, unsigned firPat, string localeId,
			  map<unsigned, unsigned> paramToPattern)
{
//  cout << "Inside  " << "callMacro" << "  " << callMacroNode.attribute (N).value ()
//      << endl;
  printNodeAttrs (callMacroNode);

  vector<string> output;

  string macroName = callMacroNode.attribute (N).value ();

  map<unsigned, unsigned> newParamToPattern;
  unsigned i = 1;
  for (xml_node with_param = callMacroNode.child (WITH_PARAM); with_param; with_param =
      with_param.next_sibling ())
    {
      unsigned pos = with_param.attribute (POS).as_uint ();
      if (paramToPattern.size ())
	pos = paramToPattern[pos];

      newParamToPattern[i++] = pos;
    }

  xml_node transfer = callMacroNode.root ().first_child ();

//  cout << callMacroNode.root ().name () << endl;
  xml_node macros = transfer.child (SECTION_DEF_MACROS);

  xml_node macro;
  for (macro = macros.child (DEF_MACRO); macro; macro = macro.next_sibling ())
    {
      if (string (macro.attribute (N).value ()) == macroName)
	break;
    }

  for (xml_node child = macro.first_child (); child; child = child.next_sibling ())
    {
      vector<string> result;

      string childName = child.name ();
      //cout << childName << endl;
      if (childName == CHOOSE)
	result = choose (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			 spaces, firPat, localeId, newParamToPattern);

      else if (childName == OUT)
	result = out (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		      firPat, localeId, newParamToPattern);

      else if (childName == CALL_MACRO)
	result = callMacro (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			    spaces, firPat, localeId, newParamToPattern);

      else if (childName == LET)
	let (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
	     localeId, newParamToPattern);

      else if (childName == MODIFY_CASE)
	modifyCase (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		    firPat, localeId, newParamToPattern);

      else if (childName == APPEND)
	append (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces, firPat,
		localeId, newParamToPattern);

      output.insert (output.end (), result.begin (), result.end ());
    }

  return output;
}

vector<string>
RuleExecution::findAttrPart (vector<string> tokenTags, vector<vector<string> > attrTags)
{

  vector<string> matchedTags;
  for (unsigned i = 0; i < tokenTags.size (); i++)
    {
      for (unsigned j = 0; j < attrTags.size (); j++)
	{
	  if (tokenTags[i] == ("<" + attrTags[j][0] + ">"))
	    {
	      matchedTags.push_back (tokenTags[i]);
	      for (unsigned k = 1; k < attrTags[j].size () && (k + i) < tokenTags.size ();
		  k++)
		{

		  if (tokenTags[i + k] == ("<" + attrTags[j][k] + ">"))
		    matchedTags.push_back (tokenTags[i + k]);
		  else
		    break;
		}
	      if (matchedTags.size () == attrTags[j].size ())
		return matchedTags;
	      else
		matchedTags.clear ();
	    }
	}
    }
  return matchedTags;
}

bool
RuleExecution::equal (xml_node equal, vector<vector<string> >* slAnalysisTokens,
		      vector<vector<string> >* tlAnalysisTokens,
		      map<string, vector<vector<string> > > attrs,
		      map<string, string>* vars, vector<string> spaces, unsigned firPat,
		      string localeId, map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "equal" << endl;
  printNodeAttrs (equal);

  xml_node firstChild = equal.first_child ();
  vector<string> firstResult;

  string firstName = firstChild.name ();
  if (firstName == CLIP)
    {
      firstResult = clip (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			  spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == CONCAT)
    {
      firstResult = concat (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			    spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == LIT_TAG)
    {
      firstResult = litTag (firstChild);
    }
  else if (firstName == LIT)
    {
      firstResult.push_back (lit (firstChild));
    }
  else if (firstName == B)
    {
      firstResult.push_back (b (firstChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == CASE_OF)
    {
      firstResult.push_back (
	  caseOf (firstChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (firstName == GET_CASE_FROM)
    {
      firstResult.push_back (
	  getCaseFrom (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == VAR)
    {
      firstResult.push_back (var (firstChild, vars));
    }

  xml_node secondChild = firstChild.next_sibling ();
  vector<string> secondResult;

  string secondName = secondChild.name ();
  if (secondName == CLIP)
    {
      secondResult = clip (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			   spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == CONCAT)
    {
      secondResult = concat (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == LIT_TAG)
    {
      secondResult = litTag (secondChild);
    }
  else if (secondName == LIT)
    {
      secondResult.push_back (lit (secondChild));
//      secondResult.push_back (secondChild.attribute (V).value ());
    }
  else if (secondName == B)
    {
      secondResult.push_back (b (secondChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == CASE_OF)
    {
      secondResult.push_back (
	  caseOf (secondChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (secondName == GET_CASE_FROM)
    {
      secondResult.push_back (
	  getCaseFrom (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == VAR)
    {
      secondResult.push_back (var (secondChild, vars));
    }

  string firstStr, secondStr;
  for (unsigned i = 0; i < firstResult.size (); i++)
    {
      firstStr += firstResult[i];
    }
  for (unsigned i = 0; i < secondResult.size (); i++)
    {
      secondStr += secondResult[i];
    }

  // remove "<>"
//  string temp;
//  for (unsigned i = 0; i < firstStr.size (); i++)
//    if (firstStr[i] != '<' && firstStr[i] != '>')
//      temp += firstStr[i];
//  firstStr = temp;
//  temp.clear ();
//  for (unsigned i = 0; i < secondStr.size (); i++)
//    if (secondStr[i] != '<' && secondStr[i] != '>')
//      temp += secondStr[i];
//  secondStr = temp;
//  cout << "firstStr=" << firstStr << " , secondStr=" << secondStr << endl;

  xml_attribute caseless = equal.attribute (CASE_LESS);
  if (string (caseless.value ()) == "yes")
    {
      return !(CLExec::compareCaseless (firstStr, secondStr, localeId));
    }
  else
    {
      return !(CLExec::compare (firstStr, secondStr));
    }
}

vector<string>
RuleExecution::choose (xml_node chooseNode, vector<vector<string> >* slAnalysisTokens,
		       vector<vector<string> >* tlAnalysisTokens,
		       map<string, vector<vector<string> > > attrs,
		       map<string, vector<string> > lists, map<string, string>* vars,
		       vector<string> spaces, unsigned firPat, string localeId,
		       map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "choose" << endl;
  printNodeAttrs (chooseNode);

  vector<string> output;

  for (xml_node child = chooseNode.first_child (); child; child = child.next_sibling ())
    {
      bool condition = false;

      string childName = child.name ();
      if (childName == WHEN)
	{
	  xml_node testNode = child.child (TEST);

	  condition = test (testNode, slAnalysisTokens, tlAnalysisTokens, attrs, lists,
			    vars, spaces, firPat, localeId, paramToPattern);
	}
      else
	{
	  // otherwise
	  condition = true;
	}

//      cout << "condition=" << condition << endl;
      if (condition)
	{
	  for (xml_node inst = child.first_child (); inst; inst = inst.next_sibling ())
	    {
	      vector<string> result;

	      string instName = inst.name ();
	      if (instName == CHOOSE)
		result = choose (inst, slAnalysisTokens, tlAnalysisTokens, attrs, lists,
				 vars, spaces, firPat, localeId, paramToPattern);

	      else if (instName == OUT)
		result = out (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			      spaces, firPat, localeId, paramToPattern);

	      else if (instName == CALL_MACRO)
		result = callMacro (inst, slAnalysisTokens, tlAnalysisTokens, attrs,
				    lists, vars, spaces, firPat, localeId,
				    paramToPattern);

	      else if (instName == LET)
		let (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
		     firPat, localeId, paramToPattern);

	      else if (instName == MODIFY_CASE)
		modifyCase (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			    firPat, localeId, paramToPattern);

	      else if (instName == APPEND)
		append (inst, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			firPat, localeId, paramToPattern);

	      output.insert (output.end (), result.begin (), result.end ());
	    }
	  break;
	}
    }

  return output;
}

bool
RuleExecution::test (xml_node test, vector<vector<string> >* slAnalysisTokens,
		     vector<vector<string> >* tlAnalysisTokens,
		     map<string, vector<vector<string> > > attrs,
		     map<string, vector<string> > lists, map<string, string>* vars,
		     vector<string> spaces, unsigned firPat, string localeId,
		     map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "test" << endl;

  printNodeAttrs (test);

  xml_node child = test.first_child ();
  string childName = child.name ();

  bool condition = false;

  if (childName == EQUAL)
    {
      condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern);
    }
  else if (childName == AND)
    {
      condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == OR)
    {
      condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == NOT)
    {
      condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == IN)
    {
      condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }

  return condition;
}

bool
RuleExecution::And (xml_node andNode, vector<vector<string> >* slAnalysisTokens,
		    vector<vector<string> >* tlAnalysisTokens,
		    map<string, vector<vector<string> > > attrs,
		    map<string, vector<string> > lists, map<string, string>* vars,
		    vector<string> spaces, unsigned firPat, string localeId,
		    map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "and" << endl;
  printNodeAttrs (andNode);

  bool condition = false;

  for (xml_node child = andNode.first_child (); child; child = child.next_sibling ())
    {
      string childName = child.name ();
      if (childName == EQUAL)
	{
	  condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == AND)
	{
	  condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == OR)
	{
	  condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == NOT)
	{
	  condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == IN)
	{
	  condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}

      if (!condition)
	break;
    }

  return condition;
}

bool
RuleExecution::Or (xml_node orNode, vector<vector<string> >* slAnalysisTokens,
		   vector<vector<string> >* tlAnalysisTokens,
		   map<string, vector<vector<string> > > attrs,
		   map<string, vector<string> > lists, map<string, string>* vars,
		   vector<string> spaces, unsigned firPat, string localeId,
		   map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "or" << endl;
  printNodeAttrs (orNode);

  bool condition = false;

  for (xml_node child = orNode.first_child (); child; child = child.next_sibling ())
    {
      string childName = child.name ();
      if (childName == EQUAL)
	{
	  condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == AND)
	{
	  condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == OR)
	{
	  condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == NOT)
	{
	  condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			   spaces, firPat, localeId, paramToPattern);
	}
      else if (childName == IN)
	{
	  condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
			  spaces, firPat, localeId, paramToPattern);
	}

      if (condition)
	break;
    }

  return condition;
}

bool
RuleExecution::in (xml_node inNode, vector<vector<string> >* slAnalysisTokens,
		   vector<vector<string> >* tlAnalysisTokens,
		   map<string, vector<vector<string> > > attrs,
		   map<string, vector<string> > lists, map<string, string>* vars,
		   vector<string> spaces, unsigned firPat, string localeId,
		   map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "in" << endl;
  printNodeAttrs (inNode);

  xml_node firstChild = inNode.first_child ();
  vector<string> firstResult;

  string firstName = firstChild.name ();
  if (firstName == CLIP)
    {
      firstResult = clip (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			  spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == CONCAT)
    {
      firstResult = concat (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			    spaces, firPat, localeId, paramToPattern);
    }
  else if (firstName == LIT_TAG)
    {
      firstResult = litTag (firstChild);
    }
  else if (firstName == LIT)
    {
      firstResult.push_back (lit (firstChild));
    }
  else if (firstName == B)
    {
      firstResult.push_back (b (firstChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == CASE_OF)
    {
      firstResult.push_back (
	  caseOf (firstChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (firstName == GET_CASE_FROM)
    {
      firstResult.push_back (
	  getCaseFrom (firstChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (firstName == VAR)
    {
      firstResult.push_back (var (firstChild, vars));
    }

  string firstStr;
  for (unsigned i = 0; i < firstResult.size (); i++)
    {
      firstStr += firstResult[i];
    }

  xml_node listNode = firstChild.next_sibling ();

  string listName = listNode.attribute (N).value ();
  vector<string> list = lists[listName];

  xml_attribute caseless = inNode.attribute (CASE_LESS);
  if (string (caseless.value ()) == "yes")
    {
      for (unsigned i = 0; i < list.size (); i++)
	{
	  if (!CLExec::compareCaseless (firstStr, list[i], localeId))
	    return true;
	}
    }
  else
    {
      for (unsigned i = 0; i < list.size (); i++)
	{
	  if (!CLExec::compare (firstStr, list[i]))
	    return true;
	}
    }

  return false;
}

bool
RuleExecution::Not (xml_node NotNode, vector<vector<string> >* slAnalysisTokens,
		    vector<vector<string> >* tlAnalysisTokens,
		    map<string, vector<vector<string> > > attrs,
		    map<string, vector<string> > lists, map<string, string>* vars,
		    vector<string> spaces, unsigned firPat, string localeId,
		    map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "not" << endl;
  printNodeAttrs (NotNode);

  xml_node child = NotNode.first_child ();
  string childName = child.name ();

  bool condition = false;

  if (childName == EQUAL)
    {
      condition = equal (child, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern);
    }
  else if (childName == AND)
    {
      condition = And (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == OR)
    {
      condition = Or (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == NOT)
    {
      condition = Not (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		       spaces, firPat, localeId, paramToPattern);
    }
  else if (childName == IN)
    {
      condition = in (child, slAnalysisTokens, tlAnalysisTokens, attrs, lists, vars,
		      spaces, firPat, localeId, paramToPattern);
    }

  return !condition;
}

vector<string>
RuleExecution::litTag (xml_node litTag)
{
  //cout << "Inside  " << "litTag" << endl;
  printNodeAttrs (litTag);

  // splitting tags by '.'
  string tagsString = litTag.attribute (V).value ();
  char tagsChars[tagsString.size ()];
  strcpy (tagsChars, tagsString.c_str ());

  vector<string> tags;

  char * tag;
  tag = strtok (tagsChars, ".");
  while (tag != NULL)
    {
      tags.push_back ("<" + string (tag) + ">");
      tag = strtok (NULL, ".");
    }

  return tags;
}

string
RuleExecution::lit (xml_node lit)
{
  //cout << "Inside  " << "lit" << endl;
  printNodeAttrs (lit);

  string litValue = lit.attribute (V).value ();
  return litValue;
}

string
RuleExecution::var (xml_node var, map<string, string>* vars)
{
  //cout << "Inside  " << "var" << endl;
  printNodeAttrs (var);

  string varName = var.attribute (N).value ();
  string varValue = (*vars)[varName];
//  cout << "varname=" << varName << " , value=" << (*vars)[varName] << endl;
  return varValue;
}

void
RuleExecution::let (xml_node let, vector<vector<string> >* slAnalysisTokens,
		    vector<vector<string> >* tlAnalysisTokens,
		    map<string, vector<vector<string> > > attrs,
		    map<string, string>* vars, vector<string> spaces, unsigned firPat,
		    string localeId, map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "let" << endl;
  printNodeAttrs (let);

  xml_node firstChild = let.first_child ();
  xml_node secondChild = firstChild.next_sibling ();

  string secondName = secondChild.name ();

  vector<string> secondResult;
  if (secondName == CLIP)
    {
      secondResult = clip (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			   spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == CONCAT)
    {
      secondResult = concat (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
			     spaces, firPat, localeId, paramToPattern);
    }
  else if (secondName == LIT_TAG)
    {
      secondResult = litTag (secondChild);
    }
  else if (secondName == LIT)
    {
      secondResult.push_back (lit (secondChild));
    }
  else if (secondName == B)
    {
      secondResult.push_back (b (secondChild, spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == CASE_OF)
    {
      secondResult.push_back (
	  caseOf (secondChild, slAnalysisTokens, tlAnalysisTokens, localeId,
		  paramToPattern));
    }
  else if (secondName == GET_CASE_FROM)
    {
      secondResult.push_back (
	  getCaseFrom (secondChild, slAnalysisTokens, tlAnalysisTokens, attrs, vars,
		       spaces, firPat, localeId, paramToPattern));
    }
  else if (secondName == VAR)
    {
      secondResult.push_back (var (secondChild, vars));
    }

  string firstName = firstChild.name ();
  if (firstName == VAR)
    {
      string resultStr;
      for (unsigned i = 0; i < secondResult.size (); i++)
	resultStr += secondResult[i];

      string varName = firstChild.attribute (N).value ();
//      cout << "varname=" << varName << " , value=" << resultStr << endl;
      (*vars)[varName] = resultStr;
//      cout << "varname=" << varName << " , value=" << (*vars)[varName] << endl;
    }
  else if (firstName == CLIP)
    {
      vector<string> firstResult = clip (firstChild, slAnalysisTokens, tlAnalysisTokens,
					 attrs, vars, spaces, firPat, localeId,
					 paramToPattern);
      if (firstResult.empty ())
	return;

      unsigned pos = firstChild.attribute (POS).as_uint ();
      if (paramToPattern.size ())
	pos = paramToPattern[pos];
      pos--;

      vector<vector<string> >* analysisTokens = slAnalysisTokens;
      string side = firstChild.attribute (SIDE).value ();
      if (side == TL)
	analysisTokens = tlAnalysisTokens;

      vector<string>* analysisToken = &((*analysisTokens)[pos]);
      // exchange the first part with the second part
      for (unsigned i = 0; i < analysisToken->size (); i++)
	{
	  if ((*analysisToken)[i] == firstResult[0])
	    {
	      analysisToken->erase (analysisToken->begin () + i,
				    analysisToken->begin () + i + firstResult.size ());
	      analysisToken->insert (analysisToken->begin () + i, secondResult.begin (),
				     secondResult.end ());
	      break;
	    }
	}
    }
}

// put the token and its tags in one vector and put tags between "<" , ">"
// the analysis will be done on this vector , "<>" to differ between tags and non-tags
// and the token for the lemma
vector<string>
RuleExecution::formatTokenTags (string token, vector<string> tags)
{

  vector<string> analysisToken;
  analysisToken.push_back (token);

  for (unsigned i = 0; i < tags.size (); i++)
    {
      analysisToken.push_back ("<" + tags[i] + ">");
    }

  return analysisToken;
}

vector<string>
RuleExecution::clip (xml_node clip, vector<vector<string> >* slAnalysisTokens,
		     vector<vector<string> >* tlAnalysisTokens,
		     map<string, vector<vector<string> > > attrs,
		     map<string, string>* vars, vector<string> spaces, unsigned firPat,
		     string localeId, map<unsigned, unsigned> paramToPattern,
		     vector<vector<string> > tags)
{
  //cout << "Inside  " << "clip" << endl;
  printNodeAttrs (clip);

  vector<string> result;

  unsigned pos = clip.attribute (POS).as_uint ();
  if (paramToPattern.size ())
    pos = paramToPattern[pos];
  pos--;

  string part = clip.attribute (PART).value ();

  xml_attribute linkTo = clip.attribute (LINK_TO);
  if (string (linkTo.name ()) == LINK_TO)
    {
      pos = linkTo.as_uint () - 1;
      result = tags[pos];

//      for (unsigned i = 0; i < result.size (); i++)
//	result[i] = "<" + result[i] + ">";

      return result;
    }

  string side = clip.attribute (SIDE).value ();

//  cout << pos << "  " << slAnalysisTokens->size () << "  " << tlAnalysisTokens->size ()
//      << endl;
  vector<string> analysisToken = (*slAnalysisTokens)[pos];

  if (side == TL)
    analysisToken = (*tlAnalysisTokens)[pos];

//  cout << "analysisToken = ";
//  for (unsigned i = 0; i < analysisToken.size (); i++)
//    cout << analysisToken[i] << " ";
//  cout << endl;

  if (part == WHOLE)
    {
      result = analysisToken;
    }
  else if (part == LEM)
    {
      result.push_back (analysisToken[0]);
    }
  else if (part == LEMH || part == LEMQ)
    {
      string lem = analysisToken[0];

//      // remove #
//      string newLem;
//      for (unsigned j = 0; j < lem.size (); j++)
//	{
//	  if (lem[j] != '#')
//	    newLem += lem[j];
//	}
//      lem = newLem;
      size_t spaceInd = lem.find ('#');
      if (spaceInd == string::npos)
	spaceInd = lem.find (' ');

      if (spaceInd == string::npos)
	{
	  if (part == LEMH)
	    result.push_back (lem);
	  else
	    result.push_back ("");
	}
      else
	{
	  string lemh = lem.substr (0, spaceInd);
	  string lemq = lem.substr (spaceInd);

	  if (part == LEMH)
	    result.push_back (lemh);
	  else
	    result.push_back (lemq);
	}
    }
  else if (part == TAGS)
    {
      result.insert (result.end (), analysisToken.begin () + 1, analysisToken.end ());
    }
  // part == "attr"
  else
    {
      result = RuleExecution::findAttrPart (analysisToken, attrs[part]);
    }

  return result;
}

vector<string>
RuleExecution::concat (xml_node concat, vector<vector<string> >* slAnalysisTokens,
		       vector<vector<string> >* tlAnalysisTokens,
		       map<string, vector<vector<string> > > attrs,
		       map<string, string>* vars, vector<string> spaces, unsigned firPat,
		       string localeId, map<unsigned, unsigned> paramToPattern,
		       vector<vector<string> > tags)
{
  //cout << "Inside  " << "concat" << endl;
  printNodeAttrs (concat);

  vector<string> concatResult;

  for (xml_node node = concat.first_child (); node; node = node.next_sibling ())
    {
      vector<string> result;

      string nodeName = node.name ();
      if (nodeName == CLIP)
	{
	  result = clip (node, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			 firPat, localeId, paramToPattern, tags);
	}
      else if (nodeName == LIT_TAG)
	{
	  result = litTag (node);
	}
      else if (nodeName == LIT)
	{
	  result.push_back (lit (node));
	}
      else if (nodeName == GET_CASE_FROM)
	{
	  result.push_back (
	      getCaseFrom (node, slAnalysisTokens, tlAnalysisTokens, attrs, vars, spaces,
			   firPat, localeId, paramToPattern));
	}
      else if (nodeName == CASE_OF)
	{
	  result.push_back (
	      caseOf (node, slAnalysisTokens, tlAnalysisTokens, localeId,
		      paramToPattern));
	}
      else if (nodeName == B)
	{
	  result.push_back (b (node, spaces, firPat, localeId, paramToPattern));
	}
      else if (nodeName == VAR)
	{
	  result.push_back (var (node, vars));
	}

      concatResult.insert (concatResult.end (), result.begin (), result.end ());
    }

  return concatResult;
}

void
RuleExecution::append (xml_node append, vector<vector<string> >* slAnalysisTokens,
		       vector<vector<string> >* tlAnalysisTokens,
		       map<string, vector<vector<string> > > attrs,
		       map<string, string>* vars, vector<string> spaces, unsigned firPat,
		       string localeId, map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "append" << endl;
  printNodeAttrs (append);

  string varName = append.attribute (NAME).value ();

  vector<string> result;

  for (xml_node child = append.first_child (); child; child = child.next_sibling ())
    {
      string childName = child.name ();
      if (childName == CLIP)
	{
	  vector<string> clipResult = clip (child, slAnalysisTokens, tlAnalysisTokens,
					    attrs, vars, spaces, firPat, localeId,
					    paramToPattern);
	  result.insert (result.end (), clipResult.begin (), clipResult.end ());
	}
      else if (childName == LIT_TAG)
	{
	  vector<string> litTagResult = litTag (child);
	  result.insert (result.end (), litTagResult.begin (), litTagResult.end ());
	}
      else if (childName == LIT)
	{
	  string litResult = lit (child);
	  result.push_back (litResult);
	}
      else if (childName == VAR)
	{
	  string varResult = var (child, vars);
	  result.push_back (varResult);
	}
      else if (childName == CONCAT)
	{
	  vector<string> concatResult = concat (child, slAnalysisTokens, tlAnalysisTokens,
						attrs, vars, spaces, firPat, localeId,
						paramToPattern);
	  result.insert (result.end (), concatResult.begin (), concatResult.end ());
	}
      else if (childName == B)
	{
	  string bResult = b (child, spaces, firPat, localeId, paramToPattern);
	  result.push_back (bResult);
	}
      else if (childName == GET_CASE_FROM)
	{
	  string getCaseFromResult = getCaseFrom (child, slAnalysisTokens,
						  tlAnalysisTokens, attrs, vars, spaces,
						  firPat, localeId, paramToPattern);
	  result.push_back (getCaseFromResult);
	}
      else if (childName == CASE_OF)
	{
	  string caseOfResult = caseOf (child, slAnalysisTokens, tlAnalysisTokens,
					localeId, paramToPattern);
	  result.push_back (caseOfResult);
	}

    }

  string newVarValue = (*vars)[varName];
  for (unsigned i = 0; i < result.size (); i++)
    {
      newVarValue += result[i];
    }
  (*vars)[varName] = newVarValue;
}

string
RuleExecution::b (xml_node b, vector<string> spaces, unsigned firPat, string localeId,
		  map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "b" << endl;
  printNodeAttrs (b);

  string blank;
  xml_attribute posAtt = b.attribute (POS);
  if (string (posAtt.name ()) == POS)
    {
      unsigned pos = posAtt.as_uint ();
      if (paramToPattern.size ())
	pos = paramToPattern[pos];
      pos--;

      unsigned spacePos = firPat + (pos);
      blank = spaces[spacePos];
    }
  else
    {
      blank = " ";
    }
  return blank;
}

string
RuleExecution::caseOf (xml_node caseOf, vector<vector<string> >* slAnalysisTokens,
		       vector<vector<string> >* tlAnalysisTokens, string localeId,
		       map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "caseOf" << endl;
  printNodeAttrs (caseOf);

  string Case;

  unsigned pos = caseOf.attribute (POS).as_uint ();
  if (paramToPattern.size ())
    pos = paramToPattern[pos];
  pos--;

  string part = caseOf.attribute (PART).value ();

  if (part == LEM)
    {
      string side = caseOf.attribute (SIDE).value ();

      string token;
      if (side == SL)
	token = (*slAnalysisTokens)[pos][0];
      else
	token = (*tlAnalysisTokens)[pos][0];

      if (token == CLExec::toLowerCase (token, localeId))
	Case = aa;
      else if (token == CLExec::toUpperCase (token, localeId))
	Case = AA;
      else
	Case = Aa;
    }

  return Case;
}

string
RuleExecution::getCaseFrom (xml_node getCaseFrom,
			    vector<vector<string> >* slAnalysisTokens,
			    vector<vector<string> >* tlAnalysisTokens,
			    map<string, vector<vector<string> > > attrs,
			    map<string, string>* vars, vector<string> spaces,
			    unsigned firPat, string localeId,
			    map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "getCaseFrom" << endl;
  printNodeAttrs (getCaseFrom);

  string result;

  unsigned pos = getCaseFrom.attribute (POS).as_uint ();
  if (paramToPattern.size ())
    pos = paramToPattern[pos];
  pos--;

  xml_node child = getCaseFrom.first_child ();
  string childName = child.name ();

  if (childName == LIT)
    {
      result = lit (child);
    }
  else if (childName == VAR)
    {
      result = var (child, vars);
    }
  else if (childName == CLIP)
    {
      vector<string> clipResult = clip (child, slAnalysisTokens, tlAnalysisTokens, attrs,
					vars, spaces, firPat, localeId, paramToPattern);

      for (unsigned i = 0; i < clipResult.size (); i++)
	{
	  result += clipResult[i];
	}
    }

  string slToken = (*slAnalysisTokens)[pos][0];

  if (slToken == CLExec::toLowerCase (slToken, localeId))
    result = CLExec::toLowerCase (result, localeId);
  else if (slToken == CLExec::toUpperCase (slToken, localeId))
    result = CLExec::toUpperCase (result, localeId);
  else
    result = CLExec::FirLetUpperCase (result, localeId);

  return result;
}

void
RuleExecution::modifyCase (xml_node modifyCase, vector<vector<string> >* slAnalysisTokens,
			   vector<vector<string> >* tlAnalysisTokens,
			   map<string, vector<vector<string> > > attrs,
			   map<string, string>* vars, vector<string> spaces,
			   unsigned firPat, string localeId,
			   map<unsigned, unsigned> paramToPattern)
{
  //cout << "Inside  " << "modifyCase" << endl;
  printNodeAttrs (modifyCase);

  xml_node firstChild = modifyCase.first_child ();
  xml_node secondChild = modifyCase.next_sibling ();

  string childName = secondChild.name ();

  string Case;
  if (childName == LIT)
    {
      Case = lit (secondChild);
    }
  else if (childName == VAR)
    {
      Case = var (secondChild, vars);
    }

  childName = firstChild.name ();
  if (childName == VAR)
    {
      string varName = firstChild.attribute (N).value ();

      if (Case == aa)
	(*vars)[varName] = CLExec::toLowerCase ((*vars)[varName], localeId);
      else if (Case == AA)
	(*vars)[varName] = CLExec::toUpperCase ((*vars)[varName], localeId);
      else if (Case == Aa)
	(*vars)[varName] = CLExec::FirLetUpperCase ((*vars)[varName], localeId);

    }
  else if (childName == CLIP)
    {
      unsigned pos = firstChild.attribute (POS).as_uint ();
      if (paramToPattern.size ())
	pos = paramToPattern[pos];
      pos--;

      string side = firstChild.attribute (SIDE).value ();

      vector<vector<string> >* analysisTokens;
      if (side == SL)
	analysisTokens = slAnalysisTokens;
      else
	analysisTokens = tlAnalysisTokens;

      string part = firstChild.attribute (PART).value ();

      if (part == LEM)
	{
	  if (Case == aa)
	    (*analysisTokens)[pos][0] = CLExec::toLowerCase ((*analysisTokens)[pos][0],
							     localeId);
	  else if (Case == AA)
	    (*analysisTokens)[pos][0] = CLExec::toUpperCase ((*analysisTokens)[pos][0],
							     localeId);
	  else if (Case == Aa)
	    (*analysisTokens)[pos][0] = CLExec::FirLetUpperCase (
		(*analysisTokens)[pos][0], localeId);
	}
      else if (part == LEMH || part == LEMQ)
	{
	  string lem = (*analysisTokens)[pos][0];

//	  // remove #
//	  string newLem;
//	  for (unsigned j = 0; j < lem.size (); j++)
//	    {
//	      if (lem[j] != '#')
//		newLem += lem[j];
//	    }
//	  lem = newLem;

	  size_t spaceInd = lem.find ('#');
	  if (spaceInd == string::npos)
	    spaceInd = lem.find (' ');
	  if (spaceInd == string::npos)
	    {
	      if (Case == aa)
		lem = CLExec::toLowerCase (lem, localeId);
	      else if (Case == AA)
		lem = CLExec::toUpperCase (lem, localeId);
	      else if (Case == Aa)
		lem = CLExec::FirLetUpperCase (lem, localeId);
	    }
	  else
	    {
	      string lemh = lem.substr (0, spaceInd);
	      string lemq = lem.substr (spaceInd);

	      if (part == LEMH)
		{
		  if (Case == aa)
		    lemh = CLExec::toLowerCase (lemh, localeId);
		  else if (Case == AA)
		    lemh = CLExec::toUpperCase (lemh, localeId);
		  else if (Case == Aa)
		    lemh = CLExec::FirLetUpperCase (lemh, localeId);
		}
	      else
		{
		  if (Case == aa)
		    lemq = CLExec::toLowerCase (lemq, localeId);
		  else if (Case == AA)
		    lemq = CLExec::toUpperCase (lemq, localeId);
		  else if (Case == Aa)
		    lemq = CLExec::FirLetUpperCase (lemq, localeId);
		}

	      lem = lemh + lemq;
	    }
	}

    }

}
