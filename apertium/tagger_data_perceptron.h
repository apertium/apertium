#ifndef _TAGGERDATAPERCEP_H
#define _TAGGERDATAPERCEP_H

#include <map>
#include <string>
#include <vector>
#include <apertium/feature_vec.h>

namespace Apertium {
class TaggerDataPerceptron
{
public:
  TaggerDataPerceptron();
  virtual ~TaggerDataPerceptron();
 
  virtual void read(FILE *in);
  virtual void write(FILE *out);
  FeatureVec weights;
};
}

#endif
