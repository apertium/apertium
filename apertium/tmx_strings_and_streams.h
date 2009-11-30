#ifndef __TMXALIGNER_INCLUDE_STRINGSANDSTREAMS_H
#define __TMXALIGNER_INCLUDE_STRINGSANDSTREAMS_H

#include <string>
#include <vector>

namespace TMXAligner
{

void split( const std::string line, std::vector<std::string>& words, char delim='\t' );

} // namespace TMXAligner

#endif // #define __TMXALIGNER_INCLUDE_STRINGSANDSTREAMS_H
