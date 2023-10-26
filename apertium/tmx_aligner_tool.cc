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
#include <apertium/tmx_aligner_tool.h>
#include <lttoolbox/string_utils.h>
#include <lttoolbox/i18n.h>
#include <unicode/ustream.h>

namespace TMXAligner
{

extern std::string hunglishDictionaryHome;
extern std::string hunglishExperimentsHome;

void readTrailOrBisentenceList( std::istream& is, Trail& trail )
{
  trail.clear();
  while ( is.peek() != -1 )
  {
    int huPos, enPos;

    is >> huPos;
    if (is.peek()!=' ')
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81150", {}, {}, false);
      throw I18n(APR_I18N_DATA, "apertium").format("APR81220");
    }
    is.ignore();

    is >> enPos;
    if (is.peek()!='\n')
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81160", {}, {}, false);
      throw I18n(APR_I18N_DATA, "apertium").format("APR81220");
    }
    is.ignore();

    trail.push_back(std::make_pair(huPos,enPos));
  }
}

void scoreBisentenceListByFile( const BisentenceList& bisentenceList, const std::string& handAlignFile )
{
  Trail trailHand;
  std::ifstream is( handAlignFile.c_str() );
  readTrailOrBisentenceList( is, trailHand );

  scoreBisentenceList( bisentenceList, trailHand );
}

void scoreTrailByFile( const Trail& bestTrail, const std::string& handAlignFile )
{
  Trail trailHand;
  std::ifstream is( handAlignFile.c_str() );
  readTrailOrBisentenceList( is, trailHand );

  scoreTrail( bestTrail, trailHand );
}

// TEMP TEMP
void logLexiconCoverageOfBicorpus( SentenceList& huSentenceList, SentenceList& enSentenceList,
                                   const DictionaryItems& dictionaryItems );


// The <p> scores should not be counted. This causes some complications.
// Otherwise, this is just the average score of segments.
// Currently this does not like segment lengths of more than two.
double globalScoreOfTrail( const Trail& trail, const AlignMatrix& dynMatrix,
                         const SentenceList& huSentenceListGarbled, const SentenceList& enSentenceListGarbled )
{
  TrailScoresInterval trailScoresInterval( trail, dynMatrix, huSentenceListGarbled, enSentenceListGarbled );

  return trailScoresInterval(0,trail.size()-1);
}


void collectBisentences( const Trail& bestTrail, const AlignMatrix& dynMatrix,
                         const SentenceList& huSentenceListPretty, const SentenceList& enSentenceListPretty,
                         SentenceList& huBisentences, SentenceList& enBisentences,
                         double qualityThreshold )
{
  huBisentences.clear();
  enBisentences.clear();

  BisentenceList bisentenceList;

  TrailScores trailScores( bestTrail, dynMatrix );
  trailToBisentenceList( bestTrail, trailScores, qualityThreshold, bisentenceList );

  for (size_t i=0; i<bisentenceList.size(); ++i )
  {
    huBisentences.push_back( huSentenceListPretty[ bisentenceList[i].first  ] );
    enBisentences.push_back( enSentenceListPretty[ bisentenceList[i].second ] );
  }

//  std::cerr << huBisentences.size() << " bisentences collected." << std::endl;

}


void temporaryDumpOfAlignMatrix( std::ostream& os, const AlignMatrix& alignMatrix )
{
  for ( int huPos=0; huPos<alignMatrix.size(); ++huPos )
  {
    int rowStart = alignMatrix.rowStart(huPos);
    int rowEnd   = alignMatrix.rowEnd(huPos);
    for ( int enPos=rowStart; enPos<rowEnd; ++enPos )
    {
      bool numeric = true;
      if (numeric)
      {
        os << alignMatrix[huPos][enPos] << "\t" ;
      }
      else
      {
        if (alignMatrix[huPos][enPos]<0)
        {
          os << ". " ;
        }
        else if (alignMatrix[huPos][enPos]<10)
        {
          os << alignMatrix[huPos][enPos] << " " ;
        }
        else
        {
          os << "X " ;
        }
      }
    }
    os << std::endl;
  }
}


double alignerToolWithObjects( const DictionaryItems& dictionary,
                 SentenceList& huSentenceListPretty,
                 SentenceList& enSentenceList,
                 const AlignParameters& alignParameters,
                 std::ostream& os )
{
  int huBookSize = huSentenceListPretty.size();
  int enBookSize = enSentenceList.size();

  SentenceValues huLength,enLength;
  setSentenceValues( huSentenceListPretty, huLength, alignParameters.utfCharCountingMode ); // Here we use the most originalest Hungarian text.
  setSentenceValues( enSentenceList,       enLength, alignParameters.utfCharCountingMode );

  bool quasiglobal_stopwordRemoval = false;
//  std::cerr << "quasiglobal_stopwordRemoval is set to " << quasiglobal_stopwordRemoval << std::endl;
  if (quasiglobal_stopwordRemoval)
  {
    removeStopwords( huSentenceListPretty, enSentenceList );
//    std::cerr << "Stopwords removed." << std::endl;
  }

  SentenceList huSentenceListGarbled, enSentenceListGarbled;

  normalizeTextsForIdentity( dictionary,
                             huSentenceListPretty,  enSentenceList,
                             huSentenceListGarbled, enSentenceListGarbled );

  const int minimalThickness = 500;

  const double quasiglobal_maximalSizeInMegabytes = 4000;

  const int maximalThickness = (int) (
    quasiglobal_maximalSizeInMegabytes
    * 1024*1024 /*bytes*/
    / ( 2*sizeof(double)+sizeof(char) ) /* for the similarity, dynprog and trelli matrices */
    / (double)( huBookSize ) /* the memory consumption of alignMatrix( huBookSize, enBookSize, thickness ) is huBookSize*thickness. */
    / 2.4 /* unexplained empirically observed factor. linux top is weird. :) */
    ) ;

  // Note that thickness is not a radius, it's a diameter.
  const double thicknessRatio = 10.0;

  int thickness = (int) ( (double)( huBookSize>enBookSize ? huBookSize : enBookSize ) / thicknessRatio ) ;

  thickness = ( thickness>minimalThickness ? thickness : minimalThickness ) ;

  if (thickness>maximalThickness)
  {
//    std::cerr << "WARNING: Downgrading planned thickness " << thickness << " to " << maximalThickness ;
//    std::cerr << " to obey memory constraint of " << quasiglobal_maximalSizeInMegabytes << " megabytes " << std::endl;
//    std::cerr << "You should recompile if you have much more physical RAM than that. People of the near-future, forgive me for the inconvenience." << std::endl;

    thickness = maximalThickness;
  }

  AlignMatrix similarityMatrix( huBookSize, enBookSize, thickness, outsideOfRadiusValue );

  sentenceListsToAlignMatrixIdentity( huSentenceListGarbled, enSentenceListGarbled, similarityMatrix );
//  std::cerr << std::endl;
//  std::cerr << "Rough translation-based similarity matrix ready." << std::endl;

  Trail bestTrail;
  AlignMatrix dynMatrix( huBookSize+1, enBookSize+1, thickness, 1e30 );

  align( similarityMatrix, huLength, enLength, bestTrail, dynMatrix );
//  std::cerr << "Align ready." << std::endl;

  double globalQuality;
  globalQuality = globalScoreOfTrail( bestTrail, dynMatrix,
                                      huSentenceListGarbled, enSentenceListGarbled );

  //  std::cerr << "Global quality of unfiltered align " << globalQuality << std::endl;

  if (alignParameters.realignType==AlignParameters::NoRealign)
  {
  }
  else
  {
    AlignMatrix similarityMatrixDetailed( huBookSize, enBookSize, thickness, outsideOfRadiusValue );

    bool success = borderDetailedAlignMatrix( similarityMatrixDetailed, bestTrail, 5/*radius*/ );

    if (!success)
    {
//      std::cerr << "Realign zone too close to quasidiagonal border. Abandoning realign. The align itself is suspicious." << std::endl;
    }
    else
    {
//      std::cerr << "Border of realign zone determined." << std::endl;

      switch (alignParameters.realignType)
      {
      case AlignParameters::ModelOneRealign:
        {
          IBMModelOne modelOne;

          SentenceList huBisentences,enBisentences;

          throw I18n(APR_I18N_DATA, "apertium").format("APR81230");
//          std::cerr << "Plausible bisentences filtered." << std::endl;

          modelOne.build(huBisentences,enBisentences);
//          std::cerr << "IBM Model I ready." << std::endl;

          sentenceListsToAlignMatrixIBMModelOne( huSentenceListPretty, enSentenceList, modelOne, similarityMatrixDetailed );
//          std::cerr << "IBM Model I based similarity matrix ready." << std::endl;
          break;
        }
      case AlignParameters::FineTranslationRealign:
        {
          TransLex transLex;
          transLex.build(dictionary);
//          std::cerr << "Hashtable for dictionary ready." << std::endl;

          sentenceListsToAlignMatrixTranslation( huSentenceListPretty, enSentenceList, transLex, similarityMatrixDetailed );

//          std::cerr << "Fine translation-based similarity matrix ready." << std::endl;
          break;
        }

      case AlignParameters::NoRealign:
      default:
	{
	  break;
	}
      }

      Trail bestTrailDetailed;
      AlignMatrix dynMatrixDetailed( huBookSize+1, enBookSize+1, thickness, 1e30 );
      align( similarityMatrixDetailed, huLength, enLength, bestTrailDetailed, dynMatrixDetailed );
//      std::cerr << "Detail realign ready." << std::endl;

      bestTrail = bestTrailDetailed;
      dynMatrix = dynMatrixDetailed;

      globalQuality = globalScoreOfTrail( bestTrail, dynMatrix,
                                          huSentenceListGarbled, enSentenceListGarbled );

      //      std::cerr << "Global quality of unfiltered align after realign " << globalQuality << std::endl;
    }
  }

  TrailScoresInterval trailScoresInterval( bestTrail, dynMatrix, huSentenceListGarbled, enSentenceListGarbled );

  if ( alignParameters.postprocessTrailQualityThreshold != -1 )
  {
    postprocessTrail( bestTrail, trailScoresInterval, alignParameters.postprocessTrailQualityThreshold );
//    std::cerr << "Trail start and end postprocessed by score." << std::endl;
  }

  if ( alignParameters.postprocessTrailStartAndEndQualityThreshold != -1 )
  {
    postprocessTrailStartAndEnd( bestTrail, trailScoresInterval, alignParameters.postprocessTrailStartAndEndQualityThreshold );
//    std::cerr << "Trail start and end postprocessed by score." << std::endl;
  }

  if ( alignParameters.postprocessTrailByTopologyQualityThreshold != -1 )
  {
    postprocessTrailByTopology( bestTrail, alignParameters.postprocessTrailByTopologyQualityThreshold );
//    std::cerr << "Trail postprocessed by topology." << std::endl;
  }

  bool quasiglobal_spaceOutBySentenceLength = true;
//  std::cerr << "quasiglobal_spaceOutBySentenceLength is set to " << quasiglobal_spaceOutBySentenceLength << std::endl;
  if (quasiglobal_spaceOutBySentenceLength)
  {
    spaceOutBySentenceLength( bestTrail, huSentenceListPretty, enSentenceList, alignParameters.utfCharCountingMode );
//    std::cerr << "Trail spaced out by sentence length." << std::endl;
  }

  // In cautious mode, auto-aligned rundles are thrown away if
  // their left or right neighbour holes are not one-to-one.
  if (alignParameters.cautiousMode)
  {
    cautiouslyFilterTrail( bestTrail );
//    std::cerr << "Trail filtered by topology." << std::endl;
  }

  globalQuality = globalScoreOfTrail( bestTrail, dynMatrix,
                                    huSentenceListGarbled, enSentenceListGarbled );

  //  std::cerr << "Global quality of unfiltered align after realign " << globalQuality << std::endl;

  bool textual = ! alignParameters.justSentenceIds ;

  if (alignParameters.justBisentences)
  {
    BisentenceList bisentenceList;
    trailToBisentenceList( bestTrail, bisentenceList );

    filterBisentenceListByQuality( bisentenceList, dynMatrix, alignParameters.qualityThreshold );

    BisentenceListScores bisentenceListScores(bisentenceList, dynMatrix);

    for ( size_t i=0; i<bisentenceList.size(); ++i )
    {
      int huPos = bisentenceList[i].first;
      int enPos = bisentenceList[i].second;

      if (textual)
      {
        os << huSentenceListPretty[huPos].words;
      }
      else
      {
        os << huPos ;
      }

      os << "\t" ;

      if (textual)
      {
        os << enSentenceList[enPos].words;
      }
      else
      {
        os << enPos ;
      }

      os << "\t" << bisentenceListScores(i);

      os << std::endl;
    }

    if (! alignParameters.handAlignFilename.empty())
    {
      scoreBisentenceListByFile( bisentenceList, alignParameters.handAlignFilename );
    }
  }
  else
  {
    filterTrailByQuality( bestTrail, trailScoresInterval, alignParameters.qualityThreshold );

    for ( size_t i=0; i<bestTrail.size()-1; ++i )
    {
      // The [huPos, nexthuPos) interval corresponds to the [enPos, nextenPos) interval.
      int huPos = bestTrail[i].first;
      int enPos = bestTrail[i].second;
      int nexthuPos = bestTrail[i+1].first;
      int nextenPos = bestTrail[i+1].second;

      if (textual)
      {
        int j;
        for ( j=huPos; j<nexthuPos; ++j )
        {
            os << huSentenceListPretty[j].words;

            if (j+1<nexthuPos)
              os << " "; // os << " ~~~ ";
        }

        os << "\t" ;

        for ( j=enPos; j<nextenPos; ++j )
        {
          os << enSentenceList[j].words;
          if (j+1<nextenPos)
          {
            os << " "; // os << " ~~~ ";
          }
        }
      }
      else // (!textual)
      {
        os << huPos << "\t" << enPos ;
      }

      os << "\t" << trailScoresInterval(i);

      os << std::endl;
    }

    if (! alignParameters.handAlignFilename.empty())
    {
      scoreTrailByFile( bestTrail, alignParameters.handAlignFilename );
    }
  }

  return globalQuality;
}


void alignerToolWithFilenames( const DictionaryItems& dictionary,
                 const std::string& huFilename, const std::string& enFilename,
                 const AlignParameters& alignParameters,
                 const std::string& outputFilename)
{
  std::ifstream hus(huFilename.c_str());
  SentenceList huSentenceListPretty;
  huSentenceListPretty.readNoIds( hus );
//  std::cerr << huSentenceListPretty.size() << " hungarian sentences read." << std::endl;

  std::ifstream ens(enFilename.c_str());
  SentenceList enSentenceList;
  enSentenceList.readNoIds( ens );
//  std::cerr << enSentenceList.size() << " english sentences read." << std::endl;

  if ( (enSentenceList.      size() < huSentenceListPretty.size()/5) ||
       (huSentenceListPretty.size() < enSentenceList.      size()/5) )
  {
//    std::cerr << "Sizes differing too much. Ignoring files to avoid a rare loop bug." << std::endl;
    return;
  }

  if (outputFilename.empty())
  {
    /* double globalQuality = */alignerToolWithObjects
     ( dictionary, huSentenceListPretty, enSentenceList, alignParameters, std::cout );

//    std::cerr << "Quality " << globalQuality << std::endl ;

  }
  else
  {
    std::ofstream os(outputFilename.c_str());
    /*double globalQuality = */ alignerToolWithObjects
     ( dictionary, huSentenceListPretty, enSentenceList, alignParameters, os );

    // If you want to collect global quality information in batch mode, grep "^Quality" of stderr must do.
//    std::cerr << "Quality\t" << outputFilename << "\t" << globalQuality << std::endl ;
  }

}

void fillPercentParameter( Arguments& args, const std::string& argName, double& value )
{
  int valueInt;
  if ( args.getNumericParam(argName, valueInt))
  {
    value = 1.0 * valueInt / 100 ;
  }
}

void main_alignerToolUsage()
{
  std::cerr << I18n(APR_I18N_DATA, "apertium").format("tmx_aligner_tool_desc");
}

int main_alignerTool(int argC, char* argV[])
{
#ifndef _DEBUG
  try
#endif
  {
    if (argC<4)
    {
      main_alignerToolUsage();
      throw icu::UnicodeString("");
    }

    Arguments args;
    std::vector<const char*> remains;
    args.read( argC, argV, remains );

    AlignParameters alignParameters;

    if (args.getSwitchCompact("text"))
    {
      alignParameters.justSentenceIds = false;
    }

    if (args.getSwitchCompact("bisent"))
    {
      alignParameters.justBisentences = true;
    }

    if (args.getSwitchCompact("cautious"))
    {
      alignParameters.cautiousMode = true;
    }

    alignParameters.utfCharCountingMode = args.getSwitchCompact("utf");

    fillPercentParameter( args, "thresh", alignParameters.qualityThreshold );

    fillPercentParameter( args, "ppthresh", alignParameters.postprocessTrailQualityThreshold );

    fillPercentParameter( args, "headerthresh", alignParameters.postprocessTrailStartAndEndQualityThreshold );

    fillPercentParameter( args, "topothresh", alignParameters.postprocessTrailByTopologyQualityThreshold );

    bool batchMode = args.getSwitchCompact("batch") ;

    if (batchMode && (remains.size()!=2) )
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81170", {}, {}, false);
      std::cerr << std::endl;

      main_alignerToolUsage();
      throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
    }

    std::string handArgumentname = "hand";
    if (args.find(handArgumentname)!=args.end())
    {
      if (batchMode)
      {
      I18n(APR_I18N_DATA, "apertium").error("APR81180", {"arg"}, {handArgumentname.c_str()}, false);
        throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
      }
      else
      {
        alignParameters.handAlignFilename = args[handArgumentname].dString ;
        args.erase(handArgumentname);

        if (alignParameters.handAlignFilename.empty())
        {
          I18n(APR_I18N_DATA, "apertium").error("APR81190", {"arg"}, {handArgumentname.c_str()}, false);
          throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
        }
      }
    }

    std::string autoDictDumpArgumentname = "autodict";
    if (args.find(autoDictDumpArgumentname)!=args.end())
    {
      if (batchMode)
      {
        I18n(APR_I18N_DATA, "apertium").error("APR81180", {"arg"}, {autoDictDumpArgumentname.c_str()}, false);
        throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
      }
      else
      {
        alignParameters.autoDictionaryDumpFilename = args[autoDictDumpArgumentname].dString ;
        args.erase(autoDictDumpArgumentname);

        if (alignParameters.autoDictionaryDumpFilename.empty())
        {
          I18n(APR_I18N_DATA, "apertium").error("APR81190", {"arg"}, {autoDictDumpArgumentname.c_str()}, false);
          throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
        }
      }
    }

    if (!batchMode && (remains.size()!=3) )
    {
      I18n(APR_I18N_DATA, "apertium").error("APR81200", {}, {}, false);
      std::cerr << std::endl;

      main_alignerToolUsage();
      throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
    }

    try
    {
      args.checkEmptyArgs();
    }
    catch (...)
    {
      std::cerr << std::endl;

      main_alignerToolUsage();
      throw I18n(APR_I18N_DATA, "apertium").format("APR81240");
    }

//    std::cerr << "Reading dictionary..." << std::endl;
    const char* dicFilename = remains[0] ;
    DictionaryItems dictionary;
    std::ifstream dis(dicFilename);
    dictionary.read(dis);

    if (batchMode)
    {
      const char* batchFilename = remains[1] ;
      std::ifstream bis(batchFilename);

      while (bis.good()&&!bis.eof())
      {
        std::string line;
        std::getline(bis,line);

        std::vector<std::string> words;
        split( line, words, '\t' );

        if (words.size()!=3)
        {
          I18n(APR_I18N_DATA, "apertium").error("APR81210", {}, {}, false);
          throw I18n(APR_I18N_DATA, "apertium").format("APR81220");
        }

        std::string huFilename, enFilename, outFilename;
        huFilename  = words[0];
        enFilename  = words[1];
        outFilename = words[2];

//        std::cerr << "Processing " << outFilename << std::endl;
        bool failed = false;
        try
        {
          alignerToolWithFilenames( dictionary, huFilename, enFilename, alignParameters, outFilename );
        }
        catch ( icu::UnicodeString errorType )
        {
          std::cerr << errorType << std::endl;
          failed = true;
        }
        catch ( std::exception& e )
        {
          I18n(APR_I18N_DATA, "apertium").error("some_failed_assertion", {"what"}, {e.what()}, false);
          failed = true;
        }
        catch ( ... )
        {
          I18n(APR_I18N_DATA, "apertium").error("APR81680", {}, {}, false);
          failed = true;
        }

        if (failed)
        {
          I18n(APR_I18N_DATA, "apertium").error("APR81690", {"file"}, {outFilename.c_str()}, false);
        }
      }
    }
    else
    {
      const char* huFilename  = remains[1] ;
      const char* enFilename  = remains[2] ;

      alignerToolWithFilenames( dictionary, huFilename, enFilename, alignParameters );
    }
  }
#ifndef _DEBUG
  catch ( icu::UnicodeString errorType )
  {
    std::cerr << errorType << std::endl;
    return -1;
  }
  catch ( std::exception& e )
  {
    I18n(APR_I18N_DATA, "apertium").error("some_failed_assertion", {"what"}, {e.what()}, false);
    return -1;
  }
  catch ( ... )
  {
    I18n(APR_I18N_DATA, "apertium").error("APR81680", {}, {}, false);
    return -1;
  }
#endif
  return 0;
}

}
