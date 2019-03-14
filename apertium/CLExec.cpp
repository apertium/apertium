/*
 * CLExec.cpp
 *
 *  Created on: Jun 21, 2018
 *      Author: aboelhamd
 */

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <algorithm>
#include <cfloat>
#include <unicode/unistr.h>
#include <unicode/ustream.h>
#include <unicode/locid.h>
#include <sstream>

#include "CLExec.h"
#include "TranElemLiterals.h"
#include "pugixml.hpp"

using namespace std;
using namespace pugi;
using namespace elem;

string exec(string cmd) {
	string data;
	FILE * stream;
	const int max_buffer = 256;
	char buffer[max_buffer];

	stream = popen(cmd.c_str(), "r");
	if (stream) {
		while (!feof(stream))
			if (fgets(buffer, max_buffer, stream) != NULL)
				data.append(buffer);
		pclose(stream);
	}
	return data;
}

void CLExec::segmenter(string inFilePath, string outFilePath) {
	// clear file before writing again
	ofstream ofs;
	ofs.open(outFilePath.c_str(), ofstream::out | ofstream::trunc);
	exec(
			string("ruby2.3 kazSentenceTokenizer.rb ") + inFilePath
					+ string(" ") + outFilePath);
}

void CLExec::biltrans(string inFilePath, string outFilePath) {
	// clear file before writing again
	ofstream ofs;
	ofs.open(outFilePath.c_str(), ofstream::out | ofstream::trunc);
	exec(
			string("apertium -d $HOME/apertium-kaz-tur kaz-tur-biltrans ")
					+ inFilePath + string(" ") + outFilePath);
}

void CLExec::lextor(string inFilePath, string outFilePath) {
	// clear file before writing again
	ofstream ofs;
	ofs.open(outFilePath.c_str(), ofstream::out | ofstream::trunc);
	exec(
			string("lrx-proc -m $HOME/apertium-kaz-tur/kaz-tur.autolex.bin ")
					+ inFilePath + string(" >") + outFilePath);
}

void CLExec::interchunk(string inFilePath, string outFilePath) {
	exec(
			string("apertium-interchunk")
					+ string(
							" $HOME/apertium-kaz-tur/apertium-kaz-tur.kaz-tur.t2x")
					+ string(" $HOME/apertium-kaz-tur/kaz-tur.t2x.bin ")
					+ inFilePath + string(" ") + outFilePath);
}

void CLExec::postchunk(string inFilePath, string outFilePath) {
	exec(
			string("apertium-postchunk")
					+ string(
							" $HOME/apertium-kaz-tur/apertium-kaz-tur.kaz-tur.t3x")
					+ string(" $HOME/apertium-kaz-tur/kaz-tur.t3x.bin ")
					+ inFilePath + string(" ") + outFilePath);
}

void CLExec::transfer(string inFilePath, string outFilePath) {
	exec(
			string("apertium-transfer -n")
					+ string(
							" $HOME/apertium-kaz-tur/apertium-kaz-tur.kaz-tur.t4x")
					+ string(" $HOME/apertium-kaz-tur/kaz-tur.t4x.bin ")
					+ inFilePath
					+ string(
							" | lt-proc -g $HOME/apertium-kaz-tur/kaz-tur.autogen.bin")
					+ string(
							" | lt-proc -p $HOME/apertium-kaz-tur/kaz-tur.autopgen.bin")
					+ string(" >") + outFilePath);
}

void CLExec::assignWeights(string inFilePath, string outFilePath) {
	exec(
			(string("python3 $HOME/NormaliseK/exampleken.py <")
					+ string(inFilePath) + string(">") + string(outFilePath)).c_str());
}

vector<string> CLExec::getFilesInDir(string dir) {
	vector<string> files;

	DIR *pDIR;
	struct dirent *entry;
	if ((pDIR = opendir((string("./") + dir).c_str()))) {
		while ((entry = readdir(pDIR))) {
			if (strcmp(entry->d_name, ".") != 0
					&& strcmp(entry->d_name, "..") != 0) {
				files.push_back(entry->d_name);
			}
		}
		closedir(pDIR);
	}

	return files;
}

//void
//CLExec::runYasmet ()
//{
//  vector<string> datasets = getFilesInDir (DATASETS);
//
//  for (unsigned i = 0; i < datasets.size (); i++)
//    {
//      string dataset = datasets[i];
//
//      exec (
//	  (string ("./yasmet <") + DATASETS + string ("/") + dataset + string (">")
//	      + MODELS + string ("/") + dataset + string (".model")).c_str ());
//    }
//}

map<string, map<string, vector<float> > > CLExec::loadYasmetModels(
		string modelsFilePath, string *localeid) {
	// map with key yasmet model name and the value is
	// another map with key word name and the value is
	// vector of weights in order
	map<string, map<string, vector<float> > > classWeights;

	ifstream modelsFile((modelsFilePath).c_str());

	if (modelsFile.is_open()) {
		string line, model, token, weight;

		// localeid
		getline(modelsFile, line);
		*localeid = line;

		while (getline(modelsFile, line)) {
			// 0=>word , 1=>rule_num & 2=>wieght
			// we don't need rule number , because
			// the weights are already sorted

			char lineChar[line.size()];
			strcpy(lineChar, line.c_str());

			token = strtok(lineChar, ": ");
			if (token == "file") {
				model = strtok(NULL, ": ");
				continue;
			}
			// skip rule_num
			strtok(NULL, ": ");

			weight = strtok(NULL, ": ");

			float w = strtof(weight.c_str(), NULL);

			classWeights[model][token].push_back(w);
		}
	} else {
		cout << "error in opening models file" << endl;
	}

	return classWeights;
}

string CLExec::toLowerCase(string word, string localeId) {
	icu::UnicodeString uString(word.c_str());
	string lowWord;
	uString.toLower(localeId.c_str()).toUTF8String(lowWord);
	return lowWord;
}

string CLExec::toUpperCase(string word, string localeId) {
	icu::UnicodeString uString(word.c_str());
	string upWord;
	uString.toUpper(localeId.c_str()).toUTF8String(upWord);
	return upWord;
}

string CLExec::FirLetUpperCase(string word, string localeId) {
	icu::UnicodeString uString(word.c_str());
	uString.toLower(localeId.c_str());
	uString.setCharAt(0,
			icu::UnicodeString(uString.charAt(0)).toUpper(localeId.c_str()).charAt(
					0));

	string upWord;
	uString.toUTF8String(upWord);
	return upWord;
}

// The result of bitwise character comparison: 0 if this contains
// the same characters as text, -1 if the characters in this are
// bitwise less than the characters in text, +1 if the characters
// in this are bitwise greater than the characters in text.
int CLExec::compare(string word1, string word2) {
	icu::UnicodeString uString1(word1.c_str());
	icu::UnicodeString uString2(word2.c_str());

	return uString1.compare(uString2);
}

int CLExec::compareCaseless(string word1, string word2, string localeId) {
	icu::UnicodeString uString1(word1.c_str());
	uString1.toLower(localeId.c_str());
	icu::UnicodeString uString2(word2.c_str());
	uString2.toLower(localeId.c_str());

	return uString1.compare(uString2);
}

// to sort translations from best to worth by their weight
bool sortParameter(pair<vector<RuleExecution::Node>, float> a,
		pair<vector<RuleExecution::Node>, float> b) {
	return (a.second > b.second);
}

void CLExec::beamSearch(
		vector<pair<vector<RuleExecution::Node>, float> > *beamTree,
		unsigned beam, vector<string> slTokens,
		vector<RuleExecution::AmbigInfo> ambigInfo,
		map<string, map<string, vector<float> > > classesWeights,
		string localeId) {
	// Initialization
	(*beamTree).push_back(pair<vector<RuleExecution::Node>, float>());

	for (unsigned i = 0; i < ambigInfo.size(); i++) {
//      for (unsigned x = 0; x < beamTree->size (); x++)
//	{
//	  cout << "weight = " << (*beamTree)[x].second << endl;
//	  for (unsigned j = 0; j < (*beamTree)[x].first.size (); j++)
//	    {
//	      cout << (*beamTree)[x].first[j].tokenId << "  "
//		  << (*beamTree)[x].first[j].ruleId << "  "
//		  << (*beamTree)[x].first[j].patNum << endl;
//	    }
//	}

		RuleExecution::AmbigInfo ambig = ambigInfo[i];
//      pair<pair<unsigned, unsigned>, pair<unsigned, vector<vector<unsigned> > > > p =
//	  ambigInfo[i];
//      pair<unsigned, unsigned> wordInd = p.first;
//      vector<vector<unsigned> > ambigRules = p.second.second;
		unsigned ambigRulesSize = ambig.combinations.size();

		// name of the file is the concatenation of rules ids
		string rulesNums;
		for (unsigned x = 0; x < ambigRulesSize; x++) {
			// avoid dummy node
			for (unsigned y = 1; y < ambig.combinations[x].size(); y++) {
				stringstream ss;
				ss << ambig.combinations[x][y].ruleId;
				rulesNums += ss.str();

				if (y + 1 < ambig.combinations[x].size())
					rulesNums += "_";
			}
			rulesNums += "+";
		}

//      cout << rulesNums << endl;

		map<string, vector<float> > classWeights = classesWeights[(rulesNums
				+ ".model")];

		// build new tree for the new words
		vector<pair<vector<RuleExecution::Node>, float> > newTree;

		// initialize the new tree
		for (unsigned x = 0; x < ambigRulesSize; x++) {
			newTree.push_back(
					pair<vector<RuleExecution::Node>, float>(
							vector<RuleExecution::Node>(), 0));
		}
		// put rules
		for (unsigned z = 0; z < ambigRulesSize; z++) {
			for (unsigned y = 0; y < ambig.combinations[z].size(); y++) {
				newTree[z].first.push_back(ambig.combinations[z][y]);
			}
		}

		for (unsigned x = ambig.firTokId; x < ambig.firTokId + ambig.maxPat;
				x++) {
			// word key is the word and it's order in the rule
			stringstream ss;
			ss << x - ambig.firTokId;
			string num = "_" + ss.str();

			// handle the case of two lemmas separated by a space
			for (unsigned t = 0; t < slTokens[x].size(); t++)
				if (slTokens[x][t] == ' ')
					slTokens[x].replace(t, 1, "_");

			string word = toLowerCase(slTokens[x], localeId) + num;
			vector<float> wordWeights = classWeights[word];

			// put weights
			if (wordWeights.empty()) {
				for (unsigned z = 0; z < ambigRulesSize; z++)
					newTree[z].second += 1;
				cout << "word : " << word << "  is not found in dataset : "
						<< rulesNums << endl;
			}

			else
				for (unsigned z = 0; z < ambigRulesSize; z++)
					newTree[z].second += wordWeights[z];

		}

		// expand beamTree
		unsigned initSize = beamTree->size();
		for (unsigned z = 0; z < ambigRulesSize - 1; z++) {
			for (unsigned x = 0; x < initSize; x++) {
				beamTree->push_back(
						pair<vector<RuleExecution::Node>, float>(
								(*beamTree)[x]));
			}
		}

		// merge the two trees
		for (unsigned z = 0; z < ambigRulesSize; z++) {
			for (unsigned x = initSize * z; x < initSize * (z + 1); x++) {
				// put the new rules with the old
				(*beamTree)[x].first.insert((*beamTree)[x].first.end(),
						newTree[z].first.begin(), newTree[z].first.end());

				// add their wiehgts
				(*beamTree)[x].second += newTree[z].second;
			}
		}

		// sort beam tree
		sort(beamTree->begin(), beamTree->end(), sortParameter);

		// remove elements more than (beam)
		if (beamTree->size() > beam)
			beamTree->erase(beamTree->begin() + beam, beamTree->end());
	}
}

void CLExec::getTransInds(vector<pair<unsigned, float> > *transInds,
		vector<pair<vector<unsigned>, float> > beamTree,
		vector<vector<pair<unsigned, unsigned> > > rulesIds) {
	for (unsigned i = 0; i < beamTree.size(); i++) {
		vector<unsigned> transInd = beamTree[i].first;
		for (unsigned j = 0; j < rulesIds.size(); j++) {
			vector<pair<unsigned, unsigned> > weigInd = rulesIds[j];

			unsigned count = 0;
			for (unsigned x = 0; x < weigInd.size() && count < transInd.size();
					x++) {
				if (transInd[count] == weigInd[x].first)
					count++;
			}

			if (count == transInd.size()) {
				transInds->push_back(
						pair<unsigned, float>(j, beamTree[i].second));
				break;
			}
		}
	}
}
