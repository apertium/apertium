#include <string.h>

#include "libgen.h"

// http://www.opengroup.org/onlinepubs/007908775/xsh/basename.html

char* basename(char *path) {
	if (path != NULL) {
		// Find the last position of the \ in the path name
		char* pos = strrchr(path, '\\');

		if (pos != NULL) { // If a \ char was found...
			if (pos + 1 != NULL) // If it is not the last character in the string...
				return pos + 1; // then return a pointer to the first character after \.
			else
				return pos; // else return a pointer to \

		} else { // If a \ char was NOT found
			return path; // return the pointer passed to basename (this is probably non-conformant)
		}

	} else { // If path == NULL, return "."
		return ".";
	}
}
