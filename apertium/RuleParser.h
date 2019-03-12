/*
 * ruleParser.h
 *
 *  Created on: Apr 25, 2018
 *      Author: aboelhamd
 */

#ifndef SRC_RULEPARSER_H_
#define SRC_RULEPARSER_H_

#include "pugixml.hpp"

using namespace std;
using namespace pugi;

class RuleParser
{
public:
  static void
  sentenceTokenizer (vector<string>* slTokens, vector<string>* tlTokens,
		     vector<vector<string> >* slTags, vector<vector<string> >* tlTags,
		     vector<string>* spaces, string tokenizedSentenceStr);

  static void
  matchCats (map<unsigned, vector<string> >* catsApplied, vector<string> slTokens,
	     vector<vector<string> > tags, xml_node transfer);

  static void
  matchRules (map<xml_node, vector<pair<unsigned, unsigned> > >* rulesApplied,
	      vector<string> slTokens, map<unsigned, vector<string> > catsApplied,
	      xml_node transfer);

  static map<string, vector<vector<string> > >
  getAttrs (xml_node transfer);

  static map<string, string>
  getVars (xml_node transfer);

  static map<string, vector<string> >
  getLists (xml_node transfer);
};

#endif /* SRC_RULEPARSER_H_ */
