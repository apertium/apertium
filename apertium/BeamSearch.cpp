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

#include "pugixml.hpp"
#include "RuleParser.h"
#include "RuleExecution.h"
#include "TranElemLiterals.h"
#include "CLExec.h"
#include "BeamSearch.h"

#include <omp.h>

using namespace std;
using namespace pugi;
using namespace elem;

void BeamSearch::transfer(string transferFilePath, string localeId,
		string modelsDest, string k, FILE* lextorFileFile, FILE* outFile) {

	// load transfer file in an xml document object
	xml_document transferDoc;
	xml_parse_result result = transferDoc.load_file(transferFilePath.c_str());
	if (string(result.description()) != "No error") {
		cout << "ERROR : " << result.description() << endl;
		exit(EXIT_FAILURE);
	}

	// xml node of the parent node (transfer) in the transfer file
	xml_node transfer = transferDoc.child("transfer");

	map<string, vector<vector<string> > > attrs = RuleParser::getAttrs(
			transfer);
	map<string, string> vars = RuleParser::getVars(transfer);
	map<string, vector<string> > lists = RuleParser::getLists(transfer);

	map<string, map<string, vector<float> > > classesWeights =
			CLExec::loadYasmetModels(modelsDest);

	int beam;
	stringstream buffer(k);
	buffer >> beam;

	char buff[10240];
	string tokenizedSentence;
	while (fgets(buff, 10240, lextorFileFile)) {
		tokenizedSentence = buff;

		// spaces after each token
		vector<string> spaces;

		// tokens in the sentence order
		vector<string> slTokens, tlTokens;

		// tags of tokens in order
		vector<vector<string> > slTags, tlTags;

		RuleParser::sentenceTokenizer(&slTokens, &tlTokens, &slTags, &tlTags,
				&spaces, tokenizedSentence);

		// map of tokens ids and their matched categories
		map<unsigned, vector<string> > catsApplied;

		RuleParser::matchCats(&catsApplied, slTokens, slTags, transfer);

		// map of matched rules and a pair of first token id and patterns number
		map<xml_node, vector<pair<unsigned, unsigned> > > rulesApplied;

		RuleParser::matchRules(&rulesApplied, slTokens, catsApplied, transfer);

		// rule and (target) token map to specific output
		// if rule has many patterns we will choose the first token only
		map<unsigned, map<unsigned, string> > ruleOutputs;

		// map (target) token to all matched rules ids and the number of pattern items of each rule
		map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;

		RuleExecution::ruleOuts(&ruleOutputs, &tokenRules, slTokens, slTags,
				tlTokens, tlTags, rulesApplied, attrs, lists, &vars, spaces,
				localeId);

		// final outputs
		vector<string> outs;
		// number of generated combinations
		unsigned compNum;
		// nodes for every token and rule
		map<unsigned, vector<RuleExecution::Node> > nodesPool;
		// ambiguous informations
		vector<RuleExecution::AmbigInfo> ambigInfo;
		// beam tree
		vector<pair<vector<RuleExecution::Node>, float> > beamTree;
		// rules combinations
		vector<vector<RuleExecution::Node> > combNodes;

		nodesPool = RuleExecution::getNodesPool(tokenRules);

		RuleExecution::getAmbigInfo(tokenRules, nodesPool, &ambigInfo,
				&compNum);

		vector<RuleExecution::AmbigInfo> newAmbigInfo;
		for (unsigned j = 0; j < ambigInfo.size(); j++)
			if (ambigInfo[j].combinations.size() > 1)
				newAmbigInfo.push_back(ambigInfo[j]);

		CLExec::beamSearch(&beamTree, beam, slTokens, newAmbigInfo,
				classesWeights, localeId);

		RuleExecution::getOuts(&outs, &combNodes, beamTree, nodesPool,
				ruleOutputs, spaces);

		// write the outs
		for (unsigned j = 0; j < outs.size(); j++) {
			fputs(outs[j].c_str(), outFile);
		}
	}

}

//FILE * open_input(string const &filename) {
//	FILE *input = fopen(filename.c_str(), "r");
//	if (!input) {
//		wcerr << "Error: can't open input file '";
//		wcerr << filename.c_str() << "'." << endl;
//		exit(EXIT_FAILURE);
//	}
//
//	return input;
//}
//
//FILE * open_output(string const &filename) {
//	FILE *output = fopen(filename.c_str(), "w");
//	if (!output) {
//		wcerr << "Error: can't open output file '";
//		wcerr << filename.c_str() << "'." << endl;
//		exit(EXIT_FAILURE);
//	}
//	return output;
//}

//int main(int argc, char **argv) {
//	string sentenceFilePath, lextorFilePath, interInFilePath, localeId,
//			transferFilePath, modelsDest, k;
//
//	if (argc == 8) {
//		localeId = argv[1];
//		transferFilePath = argv[2];
//		sentenceFilePath = argv[3];
//		lextorFilePath = argv[4];
//		interInFilePath = argv[5];
//		modelsDest = argv[6];
//		k = argv[7];
//	} else {
//		localeId = "es_ES";
//		transferFilePath = "apertium-eng-spa.spa-eng.t1x";
//		sentenceFilePath = "sentences.txt";
//		lextorFilePath = "lextor.txt";
//		interInFilePath = "beaminter.txt";
//		modelsDest = "/home/aboelhamd/Downloads/models";
//		k = "8";
//
////      localeId = "kk_KZ";
////      transferFilePath = "apertium-kaz-tur.kaz-tur.t1x";
////      sentenceFilePath = "src.txt";
////      lextorFilePath = "lextor.txt";
////      interInFilePath = "beam-inter.txt";
////      modelsDest = "./UntitledFolder/models";
////      k = "8";
//
//		cout << "Error in parameters !" << endl;
//		cout
//				<< "Parameters are : localeId transferFilePath sentenceFilePath lextorFilePath interInFilePath modelsDest beamSize"
//				<< endl;
//		cout
//				<< "localeId : ICU locale ID for the source language. For Kazakh => kk-KZ"
//				<< endl;
//		cout
//				<< "transferFilePath : Apertium transfer file of the language pair used."
//				<< endl;
//		cout << "sentenceFilePath : Source language sentences file." << endl;
//		cout
//				<< "lextorFilePath : Apertium lextor file for the source language sentences."
//				<< endl;
//		cout
//				<< "interInFilePath : Output file of this program which is the input for apertium interchunk."
//				<< endl;
//		cout << "modelsDest : Yasmet models destination." << endl;
//		cout << "beamSize : The size of beam in beam search algorithm." << endl;
////      return -1;
//	}
//
//	ifstream lextorFile(lextorFilePath.c_str());
//	ifstream inSentenceFile(sentenceFilePath.c_str());
//	if (lextorFile.is_open() && inSentenceFile.is_open()) {
//		// load transfer file in an xml document object
//		xml_document transferDoc;
//		xml_parse_result result = transferDoc.load_file(
//				transferFilePath.c_str());
//		if (string(result.description()) != "No error") {
//			cout << "ERROR : " << result.description() << endl;
//			return -1;
//		}
//
//		// xml node of the parent node (transfer) in the transfer file
//		xml_node transfer = transferDoc.child("transfer");
//
//		vector<string> sourceSentences, tokenizedSentences;
//
//		string tokenizedSentence;
//		while (getline(lextorFile, tokenizedSentence)) {
//			string sourceSentence;
//			if (!getline(inSentenceFile, sourceSentence))
//				sourceSentence = "No more sentences";
//
//			sourceSentences.push_back(sourceSentence);
//			tokenizedSentences.push_back(tokenizedSentence);
//		}
//		lextorFile.close();
//		inSentenceFile.close();
//
//		map<string, vector<vector<string> > > attrs = RuleParser::getAttrs(
//				transfer);
//		map<string, string> vars = RuleParser::getVars(transfer);
//		map<string, vector<string> > lists = RuleParser::getLists(transfer);
//
//		map<string, map<string, vector<float> > > classesWeights =
//				CLExec::loadYasmetModels(modelsDest);
//
//		vector<vector<string> > vouts;
//
//		int beam;
//		stringstream buffer(k);
//		buffer >> beam;
//		for (unsigned i = 0; i < sourceSentences.size(); i++) {
//			cout << i << endl;
//
//			string sourceSentence, tokenizedSentence;
//			sourceSentence = sourceSentences[i];
//			tokenizedSentence = tokenizedSentences[i];
//
//			// spaces after each token
//			vector<string> spaces;
//
//			// tokens in the sentence order
//			vector<string> slTokens, tlTokens;
//
//			// tags of tokens in order
//			vector<vector<string> > slTags, tlTags;
//
//			RuleParser::sentenceTokenizer(&slTokens, &tlTokens, &slTags,
//					&tlTags, &spaces, tokenizedSentence);
//
//			// map of tokens ids and their matched categories
//			map<unsigned, vector<string> > catsApplied;
//
//			RuleParser::matchCats(&catsApplied, slTokens, slTags, transfer);
//
//			// map of matched rules and a pair of first token id and patterns number
//			map<xml_node, vector<pair<unsigned, unsigned> > > rulesApplied;
//
//			RuleParser::matchRules(&rulesApplied, slTokens, catsApplied,
//					transfer);
//
//			// rule and (target) token map to specific output
//			// if rule has many patterns we will choose the first token only
//			map<unsigned, map<unsigned, string> > ruleOutputs;
//
//			// map (target) token to all matched rules ids and the number of pattern items of each rule
//			map<unsigned, vector<pair<unsigned, unsigned> > > tokenRules;
//
//			RuleExecution::ruleOuts(&ruleOutputs, &tokenRules, slTokens, slTags,
//					tlTokens, tlTags, rulesApplied, attrs, lists, &vars, spaces,
//					localeId);
//
//			// final outputs
//			vector<string> outs;
//			// number of generated combinations
//			unsigned compNum;
//			// nodes for every token and rule
//			map<unsigned, vector<RuleExecution::Node> > nodesPool;
//			// ambiguous informations
//			vector<RuleExecution::AmbigInfo> ambigInfo;
//			// beam tree
//			vector<pair<vector<RuleExecution::Node>, float> > beamTree;
//			// rules combinations
//			vector<vector<RuleExecution::Node> > combNodes;
//
//			nodesPool = RuleExecution::getNodesPool(tokenRules);
//
//			RuleExecution::getAmbigInfo(tokenRules, nodesPool, &ambigInfo,
//					&compNum);
//
//			vector<RuleExecution::AmbigInfo> newAmbigInfo;
//			for (unsigned j = 0; j < ambigInfo.size(); j++)
//				if (ambigInfo[j].combinations.size() > 1)
//					newAmbigInfo.push_back(ambigInfo[j]);
//
//			CLExec::beamSearch(&beamTree, beam, slTokens, newAmbigInfo,
//					classesWeights, localeId);
//
//			RuleExecution::getOuts(&outs, &combNodes, beamTree, nodesPool,
//					ruleOutputs, spaces);
//
//			vouts.push_back(outs);
//		}
//
//		// write the outs
//		ofstream interInFile(interInFilePath.c_str());
//		if (interInFile.is_open())
//			for (unsigned i = 0; i < vouts.size(); i++) {
//				for (unsigned j = 0; j < vouts[i].size(); j++)
//					interInFile << vouts[i][j] << endl;
//			}
//		else
//			cout << "ERROR in opening files!" << endl;
//		interInFile.close();
//
//	} else {
//		cout << "ERROR in opening files!" << endl;
//	}
//	return 0;
//}

/*int main(int argc, char *argv[]) {

	string transferFilePath, localeId, modelsDest, k;
	FILE *input = stdin, *output = stdout;

	if (argc == 7) {
		output = open_output(argv[argc - 1]);
		input = open_input(argv[argc - 2]);
		k = argv[argc - 3];
		modelsDest = argv[argc - 4];
		localeId = argv[argc - 5];
		transferFilePath = argv[argc - 6];
	} else if (argc == 6) {
		input = open_input(argv[argc - 1]);
		k = argv[argc - 2];
		modelsDest = argv[argc - 3];
		localeId = argv[argc - 4];
		transferFilePath = argv[argc - 5];
	} else if (argc == 5) {
		k = argv[argc - 1];
		modelsDest = argv[argc - 2];
		localeId = argv[argc - 3];
		transferFilePath = argv[argc - 4];
	}

	BeamSearch::transfer(transferFilePath, localeId, modelsDest, k, input,
			output);
}*/
