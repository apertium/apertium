#include <map>
#include <iostream>
#include <cwchar>
#include <string>
#include <cstring>
#include <locale>
#include <lttoolbox/ltstr.h>

using namespace std;

/*struct Ltstr // Already in lttoolbox/ltstr.h
{
  bool operator()(wstring const &s1, wstring const &s2) const
  {
    return wcscmp(s1.c_str(), s2.c_str()) < 0;
  }
};
*/

class AccentsMap {
	typedef std::map<wstring, wstring, Ltstr> acmap;
	private:
		acmap           map; // Accent to character
		acmap::iterator it;  // Iterator for searching

		void init_acmap();
		void init_camap();
	public:
		AccentsMap(bool char2accent); // the direction
		~AccentsMap();

		// Optionally
		void init_locale(); 

		// The getter for both directions depending on init.
		wstring get(wstring input);
};

