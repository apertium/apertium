/*
 * BeamSearch.h
 *
 *  Created on: Mar 10, 2019
 *      Author: aboelhamd
 */

#ifndef SRC_BEAMSEARCH_H_
#define SRC_BEAMSEARCH_H_

#include <iostream>

using namespace std;

class BeamSearch {
public:
	static void transfer(string transferFilePath, string modelsFileDest,
			string k, FILE* lextorFileFile, FILE* outFile);
};
#endif /* SRC_BEAMSEARCH_H_ */
