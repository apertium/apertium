#include <apertium/xml_reader.h>


XMLReader::XmlTextReaderResource::XmlTextReaderResource(
    string const &filename,
    xmlTextReaderPtr &reader) : reader(reader)
{
  reader = xmlReaderForFile(filename.c_str(), NULL, 0);
  if (reader == NULL) {
    wcerr << L"Error: Cannot open '" << filename << L"'." << endl;
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
  while (name == L"#text" || name == L"#comment") {
    step();
  }
}

void
XMLReader::step()
{
  int retval = xmlTextReaderRead(reader);
  if (retval != 1)
  {
    parseError(L"unexpected EOF");
  }
  name = XMLParseUtil::towstring(xmlTextReaderConstName(reader));
  type = xmlTextReaderNodeType(reader);
  std::wcerr << name << L": " << type << "\n";
}

wstring
XMLReader::attrib(wstring const &name)
{
  return XMLParseUtil::attrib(reader, name);
}

string
XMLReader::attrib(string const &name)
{
  return UtfConverter::toUtf8(attrib(UtfConverter::fromUtf8(name)));
}

void
XMLReader::parseError(wstring const &message)
{
  wcerr << L"Error: (" << xmlTextReaderGetParserLineNumber(reader);
  wcerr << L"): " << message << L"." << endl;
  exit(EXIT_FAILURE);
}

void
XMLReader::unexpectedTag()
{
  parseError(L"unexpected '<" + name + L">' tag");
}

void
XMLReader::read(string const &filename)
{
  XmlTextReaderResource reader_resource(filename, reader);
  parse();
}
