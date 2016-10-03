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
#ifndef _ALIGNER_TOOL_H_
#define _ALIGNER_TOOL_H_

#include <apertium/tmx_alignment.h>

#include <apertium/tmx_words.h>
#include <apertium/tmx_book_to_matrix.h>
#include <apertium/tmx_translate.h>
#include <apertium/tmx_trail_postprocessors.h>

#include <apertium/tmx_arguments_parser.h>
#include <apertium/tmx_strings_and_streams.h>
#include <apertium/tmx_serialize_impl.h>
#include <apertium/tmx_align_parameters.h>


#include <fstream>
#include <iostream>

#include <cmath>

namespace TMXAligner{

void alignerToolWithFilenames(const DictionaryItems& dictionary,
			      const std::string& huFilename, 
			      const std::string& enFilename,
			      const AlignParameters& alignParameters,
			      const std::string& outputFilename = "" );
}
#endif
