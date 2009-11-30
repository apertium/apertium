/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/
#include <apertium/tmx_strings_and_streams.h>

namespace TMXAligner
{

void split( const std::string line, std::vector<std::string>& words, char delim /*='\t'*/ )
{
  words.clear();

  std::string current;

  for (size_t i=0; i<line.size(); ++i )
  {
    if (line[i]==delim)
    {
      words.push_back(current);
      current = "";
    }
    else
    {
      current += line[i];
    }
  }
  words.push_back(current);
}

} // namespace TMXAligner
