#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <algorithm>
#include <string.h>

#include "pugixml.hpp"
#include "TranElemLiterals.h"

using namespace std;
using namespace pugi;
using namespace elem;

#include "RuleParser.h"

void RuleParser::sentenceTokenizer(vector<string>* slTokens,
		vector<string>* tlTokens, vector<vector<string> >* slTags,
		vector<vector<string> >* tlTags, vector<string>* spaces,
		string tokenizedSentenceStr) {
	vector<string> taggedTokens;
	// from string to char*
	char tokenizedSentence[tokenizedSentenceStr.size()];
	strcpy(tokenizedSentence, tokenizedSentenceStr.c_str());

	char * taggedToken;
	taggedToken = strtok(tokenizedSentence, "^");
	while (taggedToken != NULL) {
		taggedTokens.push_back(taggedToken);
		taggedToken = strtok(NULL, "^");
	}

	size_t taggedTokensSize = taggedTokens.size();
	for (unsigned i = 0; i < taggedTokensSize; i++) {
		// take spaces after token
		size_t dolSignInd = taggedTokens[i].find("$");
		spaces->push_back(taggedTokens[i].substr(dolSignInd + 1));
		taggedTokens[i] = taggedTokens[i].substr(0, dolSignInd);

		// remove multiple translations and take only the first one
		size_t firSlashInd = taggedTokens[i].find("/");

		// if no translation , remove that word
		if (firSlashInd + 1 == taggedTokens[i].size()) {
			taggedTokens.erase(taggedTokens.begin() + i);
			spaces->erase(spaces->begin() + i);
			taggedTokensSize--;
			i--;
			continue;
		}

		size_t secSlashInd = taggedTokens[i].find("/", firSlashInd + 1);
		if (secSlashInd != string::npos)
			taggedTokens[i] = taggedTokens[i].substr(0, secSlashInd);

		// split source and target tokens
		string target = taggedTokens[i].substr(firSlashInd + 1);

		taggedTokens.push_back(target);

		taggedTokens[i] = taggedTokens[i].substr(0, firSlashInd);
	}

	for (unsigned i = 0; i < taggedTokens.size(); i++) {
		char taggedToken[taggedTokens[i].size()];
		strcpy(taggedToken, taggedTokens[i].c_str());
		char* split;

		string token;
		vector<string> tokTags;

//      cout << "taggedToken : " << taggedToken << endl;

		if (taggedToken[0] != '<') {
			split = strtok(taggedToken, "<>");
			token = split;
			split = strtok(NULL, "<>");
		} else {
			split = strtok(taggedToken, "<>");
		}

//      cout << "word : " << token << endl;

		while (split != NULL) {
			string tag = split;
			tokTags.push_back(tag);

			split = strtok(NULL, "<>");
		}

		if (i < taggedTokens.size() / 2) {
			slTokens->push_back(token);
			slTags->push_back(tokTags);
		} else {
			tlTokens->push_back(token);
			tlTags->push_back(tokTags);
		}
	}
}

void RuleParser::matchCats(map<unsigned, vector<string> >* catsApplied,
		vector<string> slTokens, vector<vector<string> > tags,
		xml_node transfer) {
	xml_node section_def_cats = transfer.child(SECTION_DEF_CATS);

//  cout << "here" << endl;

	for (xml_node def_cat = section_def_cats.child(DEF_CAT); def_cat; def_cat =
			def_cat.next_sibling()) {
		for (xml_node cat_item = def_cat.child(CAT_ITEM); cat_item; cat_item =
				cat_item.next_sibling()) {

			// separate tags from (t1.t2) format, for easy access
			string tagsString = cat_item.attribute(TAGS).value();

			char tagDotted[tagsString.size()];
			strcpy(tagDotted, tagsString.c_str());
			char* split;
			split = strtok(tagDotted, ".");

			vector<string> itemTags;

			while (split != NULL) {
				string tag = split;
				itemTags.push_back(tag);

				split = strtok(NULL, ".");
			}

			for (unsigned x = 0; x < slTokens.size(); x++) {
				// if cat-item have lemma
				if (!string(cat_item.attribute("lemma").value()).empty()) {
					if (string(cat_item.attribute("lemma").value())
							!= slTokens[x]) {
						continue;
					}
				}

				vector<string> tokTags = tags[x];

				unsigned i = 0, j = 0;
				for (; i < tokTags.size() && j < itemTags.size(); i++) {
					if (itemTags[j] == "*") {
						if (j + 1 < itemTags.size() && i + 1 < tokTags.size()
								&& itemTags[j + 1] == tokTags[i + 1]) {
							j += 2;
							i++;
						}
					} else if (itemTags[j] == tokTags[i]) {
						j++;
					} else {
						break;
					}
				}

				if (i == tokTags.size()
						&& (j == itemTags.size()
								|| (j + 1 == itemTags.size()
										&& itemTags[j] == "*"
										&& itemTags[j - 1] != tokTags[i - 1]))) {
//	    	  cout << N <<endl;
//					cout << def_cat.attribute(N).value() << endl;
//					(*catsApplied)[x];
//					(*catsApplied)[x].push_back("");
					string s = def_cat.attribute(N).value();
					(*catsApplied)[x].push_back(s);
				}
			}
		}
	}

}

void RuleParser::matchRules(
		map<xml_node, vector<pair<unsigned, unsigned> > >* rulesApplied,
		vector<string> slTokens, map<unsigned, vector<string> > catsApplied,
		xml_node transfer) {

	xml_node section_rules = transfer.child(SECTION_RULES);

	vector<unsigned> tokensApplied;

	for (xml_node rule = section_rules.child(RULE); rule;
			rule = rule.next_sibling()) {
		xml_node pattern = rule.child(PATTERN);

		// Put pattern items in vector for ease in processing
		vector<xml_node> pattern_items;
		for (xml_node pattern_item = pattern.child(PATTERN_ITEM); pattern_item;
				pattern_item = pattern_item.next_sibling()) {
			pattern_items.push_back(pattern_item);
		}

		for (unsigned i = 0;
				(slTokens.size() >= pattern_items.size())
						&& i <= slTokens.size() - pattern_items.size(); i++) {

			vector<unsigned> slMatchedTokens;
			for (unsigned j = 0; j < pattern_items.size(); j++) {

				// match cat-item with pattern-item
				string slToken = slTokens[i + j];
				vector<string> cats = catsApplied[i + j];

				for (unsigned k = 0; k < cats.size(); k++) {
					// if cat name equals pattern item name
					if (pattern_items[j].attribute(N).value() == cats[k]) {
						slMatchedTokens.push_back(i + j);
						break;
					}
				}
			}
			// if matched tokens' size = pattern items' size
			// then this rule is matched
			if (slMatchedTokens.size() == pattern_items.size()) {
				if (slMatchedTokens.size() == 1)
					tokensApplied.insert(tokensApplied.end(),
							slMatchedTokens.begin(), slMatchedTokens.end());
				(*rulesApplied)[rule].push_back(
						pair<unsigned, unsigned>(slMatchedTokens[0],
								slMatchedTokens.size()));
			}

		}

	}

	// set a default rule for tokens without rules applied
	vector<pair<unsigned, unsigned> > tokensNotApp;
	for (unsigned i = 0; i < slTokens.size(); i++) {
		bool found = false;
		for (unsigned j = 0; j < tokensApplied.size(); j++) {
			if (i == tokensApplied[j]) {
				found = true;
				break;
			}
		}
		if (!found) {
//	  vector<unsigned> tokenNotApp;
//	  tokenNotApp.push_back (i);
//	  tokensNotApp.push_back (tokenNotApp);
			tokensNotApp.push_back(pair<unsigned, unsigned>(i, 1));
		}
	}

	xml_node defaultRule;

	(*rulesApplied)[defaultRule] = tokensNotApp;
}

// to sort attribute tags descendingly
bool sortParameter(vector<string> a, vector<string> b) {
	return (a.size() > b.size());
}

map<string, vector<vector<string> > > RuleParser::getAttrs(xml_node transfer) {
	map<string, vector<vector<string> > > attrs;
	xml_node section_def_attrs = transfer.child(SECTION_DEF_ATTRS);

	for (xml_node def_attr = section_def_attrs.child(DEF_ATTR); def_attr;
			def_attr = def_attr.next_sibling()) {

		vector<vector<string> > allTags;
		for (xml_node attr_item = def_attr.child(ATTR_ITEM); attr_item;
				attr_item = attr_item.next_sibling()) {
			// splitting tags by '.'
			string tagsString = attr_item.attribute(TAGS).value();
			char tagsChars[tagsString.size()];
			strcpy(tagsChars, tagsString.c_str());

			vector<string> tags;

			char * tag;
			tag = strtok(tagsChars, ".");
			while (tag != NULL) {
				tags.push_back(tag);
				tag = strtok(NULL, ".");
			}

			allTags.push_back(tags);
		}
		// sort the tags , descendingly by their size
		sort(allTags.begin(), allTags.end(), sortParameter);
//      cout << def_attr.attribute (N).value () << endl;
		attrs[def_attr.attribute(N).value()] = allTags;
	}

	return attrs;
}

map<string, string> RuleParser::getVars(xml_node transfer) {
	map<string, string> vars;

	xml_node section_def_vars = transfer.child(SECTION_DEF_VARS);
	for (xml_node def_var = section_def_vars.child(DEF_VAR); def_var; def_var =
			def_var.next_sibling()) {
		vars[def_var.attribute(N).value()] = def_var.attribute(V).value();
	}

	return vars;
}

map<string, vector<string> > RuleParser::getLists(xml_node transfer) {
	map<string, vector<string> > lists;

	xml_node section_def_lists = transfer.child(SECTION_DEF_LISTS);
	for (xml_node def_list = section_def_lists.child(DEF_LIST); def_list;
			def_list = def_list.next_sibling()) {
		vector<string> list;
		for (xml_node list_item = def_list.child(LIST_ITEM); list_item;
				list_item = list_item.next_sibling()) {
			list.push_back(list_item.attribute(V).value());
		}
		lists[def_list.attribute(N).value()] = list;
	}

	return lists;
}
