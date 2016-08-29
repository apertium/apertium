#ifndef __DUALMORPHOSTREAM_H
#define __DUALMORPHOSTREAM_H

#include <apertium/tagger_word.h>
#include <apertium/file_morpho_stream.h>

using namespace std;

/** Class DualMorphoStream.  
 *  This class combines two MorphoStreams, picking the 
 */
class DualMorphoStream : public MorphoStream {
private:
  FileMorphoStream cg_stream;
  FileMorphoStream untagged_stream;
public:
   /** Constructor 
    *  @param is the input stream.
    */
   DualMorphoStream(FILE *fcg, FILE *funtagged, bool d, TaggerData *t);

   /** See interface */
   TaggerWord* get_next_word();  
   void setNullFlush(bool nf);
   bool getEndOfFile(void);
   void setEndOfFile(bool eof);
   void rewind();
};

#endif
