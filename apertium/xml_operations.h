/*
 * xmloperations.h
 *
 *  Created on: Mar 28, 2019
 *      Author: aboelhamd
 */

#ifndef SRC_XML_OPERATIONS_H_
#define SRC_XML_OPERATIONS_H_

#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

class xml_operations
{

public:

  static xmlNode*
  getRoot (xmlNode* node);

  static xmlNode*
  getFirstNext (xmlNode* node);

  static xmlNode*
  getNext (xmlNode* node, string nextName);

  static xmlNode*
  getFirstChild (xmlNode* parent);

  static xmlNode*
  getChild (xmlNode* parent, string childName);

  static string
  getAttVal (xmlNode* node, string attrName);

  static unsigned
  getAttValUnsg (xmlNode* node, string attrName);

  static string
  getName (xmlNode* node);
};

#endif /* SRC_XML_OPERATIONS_H_ */
