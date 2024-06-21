#include <apertium/xml_reader.h>
#include <lttoolbox/i18n.h>


XMLReader::XmlTextReaderResource::XmlTextReaderResource(
    string const &filename,
    xmlTextReaderPtr &reader) : reader(reader)
{
  reader = xmlReaderForFile(filename.c_str(), NULL, 0);
  if (reader == NULL) {
		I18n(APR_I18N_DATA, "apertium").error("APR80000", {"file_name"}, {filename.c_str()}, true);
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
  while (name == "#text"_u || name == "#comment"_u) {
    step();
  }
}

void
XMLReader::step()
{
  int retval = xmlTextReaderRead(reader);
  if (retval != 1)
  {
    I18n(APR_I18N_DATA, "apertium").error("APR81570", {"line", "column"},
      {xmlTextReaderGetParserLineNumber(reader),
       xmlTextReaderGetParserColumnNumber(reader)}, true);
  }
  type = xmlTextReaderNodeType(reader);
  if (type == XML_READER_TYPE_DOCUMENT_TYPE) {
    // Some users have DOCTYPE declarations for the benefit of particular editors
    // but we always want to skip those
    step();
    return;
  }
  name = XMLParseUtil::readName(reader);
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

std::string
XMLReader::attrib_str(const UString& name)
{
  return XMLParseUtil::attrib_str(reader, name);
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
XMLReader::parseError(const std::string& message)
{
  cerr << "Error at line " << xmlTextReaderGetParserLineNumber(reader)
       << ", column " << xmlTextReaderGetParserColumnNumber(reader)
       << ": " << message << "." << endl;
  exit(EXIT_FAILURE);
}

void
XMLReader::warnAtLoc()
{
  cerr << "Warning at line " << xmlTextReaderGetParserLineNumber(reader)
       << ", column " << xmlTextReaderGetParserColumnNumber(reader) << ": ";
}

void
XMLReader::unexpectedTag()
{
  I18n(APR_I18N_DATA, "apertium").error("APR81460", {"line", "column", "tag"},
    {xmlTextReaderGetParserLineNumber(reader),
     xmlTextReaderGetParserColumnNumber(reader), icu::UnicodeString(name.data())}, true);
}

void
XMLReader::read(string const &filename)
{
  path = filename;
  XmlTextReaderResource reader_resource(filename, reader);
  parse();
}
