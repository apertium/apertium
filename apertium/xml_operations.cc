/*
 * xmloperations.cpp
 *
 *  Created on: Mar 28, 2019
 *      Author: aboelhamd
 */

#include "xml_operations.h"
#include <iostream>

xmlNode*
xml_operations::getRoot (xmlNode* node)
{
  xmlNode* parent = NULL;
  for (xmlNode *i = node->parent; i; i = i->parent)
    if (i->type == XML_ELEMENT_NODE)
      parent = i;
  return parent;
}

xmlNode*
xml_operations::getFirstNext (xmlNode* node)
{
  for (xmlNode *i = node->next; i; i = i->next)
    if (i->type == XML_ELEMENT_NODE)
      return i;
  return NULL;
}

xmlNode*
xml_operations::getNext (xmlNode* node, string nextName)
{
  for (xmlNode *i = node->next; i; i = i->next)
    if (i->type == XML_ELEMENT_NODE
	&& !xmlStrcmp (i->name, (const xmlChar *) nextName.c_str ()))
      return i;
  return NULL;
}

xmlNode*
xml_operations::getFirstChild (xmlNode* parent)
{
  for (xmlNode *i = parent->children; i; i = i->next)
    if (i->type == XML_ELEMENT_NODE)
      return i;
  return NULL;
}

xmlNode*
xml_operations::getChild (xmlNode* parent, string childName)
{
  for (xmlNode *i = parent->children; i; i = i->next)
    if (i->type == XML_ELEMENT_NODE
	&& !xmlStrcmp (i->name, (const xmlChar *) childName.c_str ()))
      return i;
  return NULL;
}

string
xml_operations::getAttVal (xmlNode* node, string attrName)
{
  if (node->type == XML_ELEMENT_NODE
      && xmlHasProp (node, (const xmlChar *) attrName.c_str ()))
    return (const char *) xmlGetProp (node, (const xmlChar *) attrName.c_str ());
  return "";
}

unsigned
xml_operations::getAttValUnsg (xmlNode* node, string attrName)
{
  return atoi (getAttVal (node, attrName).c_str ());
}

string
xml_operations::getName (xmlNode* node)
{
  if (node->type == XML_ELEMENT_NODE)
    return (const char *) node->name;
  return "";
}
