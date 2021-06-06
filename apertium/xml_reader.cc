#include <apertium/xml_reader.h>


XMLReader::XmlTextReaderResource::XmlTextReaderResource(
    string const &filename,
    xmlTextReaderPtr &reader) : reader(reader)
{
  reader = xmlReaderForFile(filename.c_str(), NULL, 0);
  if (reader == NULL) {
    cerr << "Error: Cannot open '" << filename << "'." << endl;
    exit(EXIT_FAILURE);
  }
}

XMLReader::XmlTextReaderResource::~XmlTextReaderResource()
{
  if (reader != NULL) {
    xmlFreeTextReader(reader);
    xmlCleanupParser();
  }
}

XMLReader::XMLReader() : reader(0), type(0) {}

void
XMLReader::stepToTag()
{
  while (name == "#text" || name == "#comment") {
    step();
  }
}

void
XMLReader::step()
{
  int retval = xmlTextReaderRead(reader);
  if (retval != 1)
  {
    parseError("unexpected EOF");
  }
  name = XMLParseUtil::readName(reader);
  type = xmlTextReaderNodeType(reader);
  //std::cerr << name << ": " << type << "\n";
}

void
XMLReader::stepPastSelfClosingTag(UString const &tag)
{
  // libxml2 expands <foo /> to <foo></foo> inside entities.
  // This method exists to work around this difference.
  step();
  if (name == tag && type ==  XML_READER_TYPE_END_ELEMENT) {
    step();
  }
  stepToTag();
}

void
XMLReader::stepToNextTag()
{
  stepToTag();
  step();
  stepToTag();
}

UString
XMLReader::attrib(UString const &name)
{
  return XMLParseUtil::attrib(reader, name);
}

string
XMLReader::attrib(string const &name)
{
  return UtfConverter::toUtf8(attrib(UtfConverter::fromUtf8(name)));
}

void
XMLReader::parseError(UString const &message)
{
  cerr << "Error at line " << xmlTextReaderGetParserLineNumber(reader)
        << ", column " << xmlTextReaderGetParserColumnNumber(reader)
        << ": " << message << "." << endl;
  exit(EXIT_FAILURE);
}

void
XMLReader::unexpectedTag()
{
  parseError("unexpected '<" + name + ">' tag");
}

void
XMLReader::read(string const &filename)
{
  path = filename;
  XmlTextReaderResource reader_resource(filename, reader);
  parse();
}
