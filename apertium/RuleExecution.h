/*
 * RuleExecution.h
 *
 *  Created on: May 5, 2018
 *      Author: aboelhamd
 */

#ifndef SRC_RULEEXECUTION_H_
#define SRC_RULEEXECUTION_H_

#include "pugixml.hpp"

using namespace std;
using namespace pugi;

class RuleExecution
{
public:
  class Node
  {
  public:
    unsigned tokenId;
    unsigned ruleId;
    unsigned patNum;
    vector<Node> neighbors;
    Node (unsigned tokenId, unsigned ruleId, unsigned patNum)
    {
      this->tokenId = tokenId;
      this->ruleId = ruleId;
      this->patNum = patNum;
    }
    Node ()
    {
      this->tokenId = 0;
      this->ruleId = 0;
      this->patNum = 0;
    }
  };

  class AmbigInfo
  {
  public:
    unsigned firTokId;
    unsigned maxPat;
    vector<vector<Node> > combinations;
    AmbigInfo (unsigned firTokId, unsigned maxPat)
    {
      this->firTokId = firTokId;
      this->maxPat = maxPat;
    }
    AmbigInfo ()
    {
      this->firTokId = 0;
      this->maxPat = 0;
    }
  };

  static vector<vector<float> >
  normaliseWeights (vector<vector<float> > vweights);

  static void
  normaliseWeights (vector<float>* weights);

  static void
  normaliseWeights (vector<vector<float> >* vweights,
		    vector<vector<RuleExecution::AmbigInfo> >* vambigInfo);

  static void
  normaliseWeights (vector<float>* weights, vector<RuleExecution::AmbigInfo> ambigInfo);

  static void
  getOuts (vector<string>* finalOuts, vector<vector<Node> >* finalCombNodes,
	   vector<pair<vector<RuleExecution::Node>, float> > beamTree,
	   map<unsigned, vector<RuleExecution::Node> > nodesPool,
	   map<unsigned, map<unsigned, string> > ruleOutputs, vector<string> spaces);

  static void
  getOuts (vector<string>* finalOuts, vector<vector<Node> >* combNodes,
	   vector<RuleExecution::AmbigInfo> ambigInfo,
	   map<unsigned, vector<RuleExecution::Node> > nodesPool,
	   map<unsigned, map<unsigned, string> > ruleOutputs, vector<string> spaces);

  static map<unsigned, vector<RuleExecution::Node> >
  getNodesPool (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules);

  static string
  noRuleOut (vector<string> analysis);

//  static void
//  getCombinations (Node root, vector<pair<unsigned, unsigned> > path,
//		   vector<vector<pair<unsigned, unsigned> > >* ambigRules);

  static void
  getCombinations (Node root, vector<Node> path, vector<vector<Node> >* ambigRules);

  static Node
  ambiguousGraph (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		  map<unsigned, vector<Node> > nodesPool, unsigned firTok,
		  unsigned maxPat);

  static Node
  ambiguousGraph (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		  map<unsigned, vector<Node> > nodesPool);

  static void
  getAmbigInfo (map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
		map<unsigned, vector<RuleExecution::Node> > nodesPool,
		vector<RuleExecution::AmbigInfo>* ambigInfo, unsigned* combNum);

//  static void
//  getAmbigInfo (
//      vector<vector<vector<unsigned> > >* ambigRulIdsCombs,
//      map<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > >* ambigInfo,
//      vector<pair<unsigned, unsigned> > maxPatts,
//      vector<vector<RuleExecution::Node*> > tokRulIdsCombs);

  static bool
  outputs (
      vector<string>* outs,
      vector<vector<pair<unsigned, unsigned> > >* rulesIds,
      vector<vector<vector<unsigned> > >* outsRules,
      vector<pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > > *ambigInfo,
      vector<string> tlTokens, vector<vector<string> > tags,
      map<unsigned, map<unsigned, string> > ruleOuts,
      map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules,
      vector<string> spaces);

  static void
  weightIndices (
      vector<vector<unsigned> >* weigInds,
      vector<pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > > ambigInfo,
      vector<vector<vector<unsigned> > > outsRules);

  static void
  ruleOuts (map<unsigned, map<unsigned, string> >* ruleOuts,
	    map<unsigned, vector<pair<unsigned, unsigned> > >* tokenRules,
	    vector<string> slTokens, vector<vector<string> > slTags,
	    vector<string> tlTokens, vector<vector<string> > tlTags,
	    map<xml_node, vector<pair<unsigned, unsigned> > > rulesApplied,
	    map<string, vector<vector<string> > > attrs,
	    map<string, vector<string> > lists, map<string, string>* vars,
	    vector<string> spaces, string localeId);

  static vector<string>
  ruleExe (xml_node rule, vector<vector<string> >* slAnalysisTokens,
	   vector<vector<string> >* tlAnalysisTokens,
	   map<string, vector<vector<string> > > attrs,
	   map<string, vector<string> > lists, map<string, string>* vars,
	   vector<string> spaces, unsigned firPat, string localeId);

  static vector<string>
  choose (xml_node choose, vector<vector<string> >* slAnalysisTokens,
	  vector<vector<string> >* tlAnalysisTokens,
	  map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
	  map<string, string>* vars, vector<string> spaces, unsigned firPat,
	  string localeId, map<unsigned, unsigned> paramToPattern);

  static void
  let (xml_node let, vector<vector<string> >* slAnalysisTokens,
       vector<vector<string> >* tlAnalysisTokens,
       map<string, vector<vector<string> > > attrs, map<string, string>* vars,
       vector<string> spaces, unsigned firPat, string localeId,
       map<unsigned, unsigned> paramToPattern);

  static vector<string>
  callMacro (xml_node callMacro, vector<vector<string> >* slAnalysisTokens,
	     vector<vector<string> >* tlAnalysisTokens,
	     map<string, vector<vector<string> > > attrs,
	     map<string, vector<string> > lists, map<string, string>* vars,
	     vector<string> spaces, unsigned firPat, string localeId,
	     map<unsigned, unsigned> paramToPattern);

  static vector<string>
  out (xml_node out, vector<vector<string> >* slAnalysisTokens,
       vector<vector<string> >* tlAnalysisTokens,
       map<string, vector<vector<string> > > attrs, map<string, string>* vars,
       vector<string> spaces, unsigned firPat, string localeId,
       map<unsigned, unsigned> paramToPattern);

  static vector<string>
  chunk (xml_node chunkNode, vector<vector<string> >* slAnalysisTokens,
	 vector<vector<string> >* tlAnalysisTokens,
	 map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	 vector<string> spaces, unsigned firPat, string localeId,
	 map<unsigned, unsigned> paramToPattern);

  static vector<string>
  formatTokenTags (string token, vector<string> tags);

  static vector<string>
  findAttrPart (vector<string> tokenTags, vector<vector<string> > attrTags);

  static vector<string>
  clip (xml_node clip, vector<vector<string> >* slAnalysisTokens,
	vector<vector<string> >* tlAnalysisTokens,
	map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	vector<string> spaces, unsigned firPat, string localeId,
	map<unsigned, unsigned> paramToPattern,
	vector<vector<string> > tags = vector<vector<string> > ());

  static vector<string>
  concat (xml_node concat, vector<vector<string> >* slAnalysisTokens,
	  vector<vector<string> >* tlAnalysisTokens,
	  map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	  vector<string> spaces, unsigned firPat, string localeId,
	  map<unsigned, unsigned> paramToPattern,
	  vector<vector<string> > tags = vector<vector<string> > ());

  static bool
  equal (xml_node equal, vector<vector<string> >* slAnalysisTokens,
	 vector<vector<string> >* tlAnalysisTokens,
	 map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	 vector<string> spaces, unsigned firPat, string localeId,
	 map<unsigned, unsigned> paramToPattern);

  static bool
  test (xml_node test, vector<vector<string> >* slAnalysisTokens,
	vector<vector<string> >* tlAnalysisTokens,
	map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
	map<string, string>* vars, vector<string> spaces, unsigned firPat,
	string localeId, map<unsigned, unsigned> paramToPattern);

  static bool
  And (xml_node And, vector<vector<string> >* slAnalysisTokens,
       vector<vector<string> >* tlAnalysisTokens,
       map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
       map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
       map<unsigned, unsigned> paramToPattern);

  static bool
  Or (xml_node Or, vector<vector<string> >* slAnalysisTokens,
      vector<vector<string> >* tlAnalysisTokens,
      map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
      map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
      map<unsigned, unsigned> paramToPattern);

  static bool
  in (xml_node in, vector<vector<string> >* slAnalysisTokens,
      vector<vector<string> >* tlAnalysisTokens,
      map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
      map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
      map<unsigned, unsigned> paramToPattern);

  static bool
  Not (xml_node Not, vector<vector<string> >* slAnalysisTokens,
       vector<vector<string> >* tlAnalysisTokens,
       map<string, vector<vector<string> > > attrs, map<string, vector<string> > lists,
       map<string, string>* vars, vector<string> spaces, unsigned firPat, string localeId,
       map<unsigned, unsigned> paramToPattern);

  static vector<string>
  litTag (xml_node litTag);

  static string
  lit (xml_node lit);

  static string
  var (xml_node var, map<string, string>* vars);

  static void
  append (xml_node append, vector<vector<string> >* slAnalysisTokens,
	  vector<vector<string> >* tlAnalysisTokens,
	  map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	  vector<string> spaces, unsigned firPat, string localeId,
	  map<unsigned, unsigned> paramToPattern);

  static string
  b (xml_node b, vector<string> spaces, unsigned firPat, string localeId,
     map<unsigned, unsigned> paramToPattern);

  static string
  caseOf (xml_node caseOf, vector<vector<string> >* slAnalysisTokens,
	  vector<vector<string> >* tlAnalysisTokens, string localeId,
	  map<unsigned, unsigned> paramToPattern);

  static string
  getCaseFrom (xml_node getCaseFrom, vector<vector<string> >* slAnalysisTokens,
	       vector<vector<string> >* tlAnalysisTokens,
	       map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	       vector<string> spaces, unsigned firPat, string localeId,
	       map<unsigned, unsigned> paramToPattern);

  static void
  modifyCase (xml_node modifyCase, vector<vector<string> >* slAnalysisTokens,
	      vector<vector<string> >* tlAnalysisTokens,
	      map<string, vector<vector<string> > > attrs, map<string, string>* vars,
	      vector<string> spaces, unsigned firPat, string localeId,
	      map<unsigned, unsigned> paramToPattern);
};

#endif /* SRC_RULEEXECUTION_H_ */
