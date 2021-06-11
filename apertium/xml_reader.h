#ifndef _XMLREADER_
#define _XMLREADER_

#include <apertium/constant_manager.h>
#include <apertium/string_utils.h>
#include <apertium/tagger_data.h>
#include <apertium/ttag.h>
#include <apertium/utf_converter.h>
#include <lttoolbox/pattern_list.h>
#include <lttoolbox/xml_parse_util.h>

#include <libxml/xmlreader.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

class XMLReader
{
private:
  class XmlTextReaderResource {
    friend class XMLReader;
    xmlTextReaderPtr &reader;
    XmlTextReaderResource(string const &filename,
                          xmlTextReaderPtr &reader);
    ~XmlTextReaderResource();
  };

protected:
  XMLReader();
  xmlTextReaderPtr reader;
  int type;
  UString name;
  UString attrib(UString const &name);
  string attrib_str(const UString& name);
  void parseError(UString const &message);
  void parseError(const string& message);
  void unexpectedTag();
  void stepToTag();
  void stepPastSelfClosingTag(UString const &tag);
  void stepToNextTag();
  void step();
  virtual void parse() = 0;

public:
  string path;
  void read(const string &filename);
};

#endif
