/*
 * BeamSearch.h
 *
 *  Created on: Mar 10, 2019
 *      Author: aboelhamd
 */

#ifndef SRC_AMBIGUOUS_TRANSFER_H_
#define SRC_AMBIGUOUS_TRANSFER_H_

#include <iostream>
#include "ambiguous_chunker.h"

using namespace std;

class AmbiguousTransfer
{
public:
  static void
  transfer (string transferFilePath, string modelsFileDest, string k,
	    FILE* lextorFileFile, FILE* outFile);
};

void
beamSearch (vector<pair<vector<AmbiguousChunker::Node>, float> > *beamTree, unsigned beam,
	    vector<string> slTokens, vector<AmbiguousChunker::AmbigInfo> ambigInfo,
	    map<string, map<string, vector<float> > > classesWeights);

map<string, map<string, vector<float> > >
loadYasmetModels (string modelsDest);

#endif /* SRC_AMBIGUOUS_TRANSFER_H_ */
