/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*  From hunalign; for license see ../AUTHORS and ../COPYING.hunalign     *
*                                                                        *
*************************************************************************/
#ifndef __TMXALIGNER_ALIGN_PARAMETERS_H
#define __TMXALIGNER_ALIGN_PARAMETERS_H


class AlignParameters
{
public:
  enum RealignType { NoRealign, ModelOneRealign, FineTranslationRealign };

  bool justSentenceIds;
  bool justBisentences;

  bool cautiousMode;
  RealignType realignType;
  double qualityThreshold;

  double postprocessTrailQualityThreshold;
  double postprocessTrailStartAndEndQualityThreshold;
  double postprocessTrailByTopologyQualityThreshold;

  std::string handAlignFilename;

  bool utfCharCountingMode;
  
  std::string autoDictionaryDumpFilename; // Empty string means do not dump.

AlignParameters() : justSentenceIds(true), 
    justBisentences(false), cautiousMode(false),
    realignType(NoRealign),
    qualityThreshold(-100000),
    postprocessTrailQualityThreshold(-1),
    postprocessTrailStartAndEndQualityThreshold(-1),
    postprocessTrailByTopologyQualityThreshold(-1),
    utfCharCountingMode(false)
      {}


};

#endif
