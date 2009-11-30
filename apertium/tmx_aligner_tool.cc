/*************************************************************************
*                                                                        *
*  (C) Copyright 2004. Media Research Centre at the                      *
*  Sociology and Communications Department of the                        *
*  Budapest University of Technology and Economics.                      *
*                                                                        *
*  Developed by Daniel Varga.                                            *
*                                                                        *
*************************************************************************/
#include <apertium/tmx_aligner_tool.h>

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
      std::cerr << "no space in line" << std::endl;
      throw "data error";
    }
    is.ignore();

    is >> enPos;
    if (is.peek()!='\n')
    {
      std::cerr << "too much data in line" << std::endl;
      throw "data error";
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

          throw "unimplemented";
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
  std::cerr << "Usage (either):\n\
    alignerTool [ common_arguments ] [ -hand=hand_align_file ] dictionary_file source_text target_text\n\
\n\
or:\n\
    alignerTool [ common_arguments ] -batch dictionary_file batch_file\n\
\n\
where\n\
common_arguments ::= [ -text ] [ -bisent ] [ -utf ] [ -cautious ] [ -realign [ -autodict=filename ] ]\n\
    [ -thresh=n ] [ -ppthresh=n ] [ -headerthresh=n ] [ -topothresh=n ]\n\
\n\
Arguments:\n\
\n\
-text\n\
	The output should be in text format, rather than the default (numeric) ladder format.\n\
\n\
-bisent\n\
	Only bisentences (one-to-one alignment segments) are printed. In non-text mode, their\n\
	starting rung is printed.\n\
\n\
-cautious\n\
	In -bisent mode, only bisentences for which both the preceeding and the following\n\
	segments are one-to-one are printed. In the default non-bisent mode, only rungs\n\
	for which both the preceeding and the following segments are one-to-one are printed.\n\
\n\
-hand=file\n\
	When this argument is given, the precision and recall of the alignment is calculated\n\
	based on the manually built ladder file. Information like the following is written\n\
	on the standard error: \n\
	53 misaligned out of 6446 correct items, 6035 bets.\n\
	Precision: 0.991218, Recall: 0.928017\n\
	\n\
        Note that by default, 'item' means rung. The switch -bisent also changes the semantics\n\
	of the scoring from rung-based to bisentence-based and in this case 'item' means bisentences.\n\
	See File formats about the format of this input align file.\n\
\n\
-autodict=filename\n\
	The dictionary built during realign is saved to this file. By default, it is not saved.\n\
\n\
-utf\n\
	The system uses the character counts of the sentences as information for the\n\
	pairing of sentences. By default, it assumes one-byte character encoding such\n\
	as ISO Latin-1 when calculating these counts. If our text is in UTF-8 format,\n\
	byte counts and character counts are different, and we must use the -utf switch\n\
	to force the system to properly calculate character counts.\n\
	Note: UTF-16 input is not supported.\n\
\n\
Postfiltering options:\n\
There are various postprocessors which remove implausible rungs based on various heuristics.\n\
\n\
-thresh=n\n\
	Don't print out segments with score lower than n/100.\n\
\n\
-ppthresh=n\n\
	Filter rungs with less than n/100 average score in their vicinity.\n\
\n\
-headerthresh=n\n\
	Filter all rungs at the start and end of texts until finding a reliably\n\
	plausible region.\n\
\n\
-topothresh=n\n\
	Filter rungs with less than n percent of one-to-one segments in their vicinity.\n\
\n\
";
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
      throw "";
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
      std::cerr << "Batch mode requires exactly two file arguments." << std::endl;
      std::cerr << std::endl;

      main_alignerToolUsage();
      throw "argument error";
    }

    std::string handArgumentname = "hand";
    if (args.find(handArgumentname)!=args.end())
    {
      if (batchMode)
      {
        std::cerr << "-batch and -" << handArgumentname << " are incompatible switches." << std::endl;
        throw "argument error";
      }
      else
      {
        alignParameters.handAlignFilename = args[handArgumentname].dString ;
        args.erase(handArgumentname);

        if (alignParameters.handAlignFilename.empty())
        {
          std::cerr << "-" << handArgumentname << " switch requires a filename value." << std::endl;
          throw "argument error";
        }
      }
    }

    std::string autoDictDumpArgumentname = "autodict";
    if (args.find(autoDictDumpArgumentname)!=args.end())
    {
      if (batchMode)
      {
        std::cerr << "-batch and -" << autoDictDumpArgumentname << " are incompatible switches." << std::endl;
        throw "argument error";
      }
      else
      {
        alignParameters.autoDictionaryDumpFilename = args[autoDictDumpArgumentname].dString ;
        args.erase(autoDictDumpArgumentname);

        if (alignParameters.autoDictionaryDumpFilename.empty())
        {
          std::cerr << "-" << autoDictDumpArgumentname << " switch requires a filename value." << std::endl;
          throw "argument error";
        }
      }
    }

    if (!batchMode && (remains.size()!=3) )
    {
      std::cerr << "Nonbatch mode requires exactly three file arguments." << std::endl;
      std::cerr << std::endl;

      main_alignerToolUsage();
      throw "argument error";
    }

    try
    {
      args.checkEmptyArgs();
    }
    catch (...)
    {
      std::cerr << std::endl;

      main_alignerToolUsage();
      throw "argument error";
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
          std::cerr << "Batch file has incorrect format." << std::endl;
          throw "data error";
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
        catch ( const char* errorType )
        {
          std::cerr << errorType << std::endl;
          failed = true;
        }
        catch ( std::exception& e )
        {
          std::cerr << "some failed assertion:" << e.what() << std::endl;
          failed = true;
        }
        catch ( ... )
        {
          std::cerr << "some unknown failed assertion..." << std::endl;
          failed = true;
        }

        if (failed)
        {
          std::cerr << "Align failed for " << outFilename << std::endl;
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
  catch ( const char* errorType )
  {
    std::cerr << errorType << std::endl;
    return -1;
  }
  catch ( std::exception& e )
  {
    std::cerr << "some failed assertion:" << e.what() << std::endl;
    return -1;
  }
  catch ( ... )
  {
    std::cerr << "some unknown failed assertion..." << std::endl;
    return -1;
  }
#endif
  return 0;
}

}
