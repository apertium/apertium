INCLUDE_DIRECTORIES (${ICONV_INCLUDE_DIR} ${LIBXML2_INCLUDE_DIR} ${LTTOOLBOX3_INCLUDE_DIR} ${LIBPCRE_INCLUDE_DIR})
ADD_DEFINITIONS (${LIBXML2_DEFINITIONS} ${LTTOOLBOX3_DEFINITIONS})

SET(APERTIUM_LIBS ${LIBXML2_LIBRARIES} ${LTTOOLBOX3_LIBRARIES} ${LIBPCRE_LIBRARIES} ${ICONV_LIBRARIES})

MACRO (FROM_XML OUTPUT XSL XML)
        XSLTPROC (${OUTPUT}.yy ${XSL} ${XML})
        FLEX (${OUTPUT} "-Cfer" ${OUTPUT}.yy)
ENDMACRO (FROM_XML)

#IF (WIN32)
#        SET (FLEX_LIB flex_support)
#
#        ADD_EXECUTABLE (runner_skeleton ${WIN32_DIR}/runner_skeleton.c)
#ENDIF (WIN32)

MACRO (ADD_FORMATTER SRC_NAME XSL XML)
        FROM_XML (${SRC_NAME}.cc ${XSL} ${XML})        # Create SRC_NAME using the stylesheet XSL and the xml file XML
        STRING (REPLACE "_" "-" EXE_NAME ${SRC_NAME})  # Convert underscores to dashes for the executable names

        ADD_EXECUTABLE (${EXE_NAME} ${SRC_NAME}.cc "${WIN32_DIR}/regex.c")    # Compile EXE_NAME using the source file ${SRC_NAME}.cc
        TARGET_LINK_LIBRARIES (${EXE_NAME} ${LIBXML2_LIBRARIES} ${ICONV_LIBRARIES} ${LTTOOLBOX3_LIBRARIES} ) # Link the lttoolbox library to EXE_NAME

        LIST (APPEND INSTALLABLES ${EXE_NAME})
ENDMACRO (ADD_FORMATTER)

ADD_FORMATTER (apertium_destxt        deformat.xsl txt-format.xml)
ADD_FORMATTER (apertium_retxt         reformat.xsl txt-format.xml)
ADD_FORMATTER (apertium_deshtml       deformat.xsl html-format.xml)
ADD_FORMATTER (apertium_rehtml        reformat.xsl html-format.xml)
ADD_FORMATTER (apertium_desodt        deformat.xsl odt-format.xml)
ADD_FORMATTER (apertium_reodt         reformat.xsl odt-format.xml)
ADD_FORMATTER (apertium_desrtf        deformat.xsl rtf-format.xml)
ADD_FORMATTER (apertium_rertf         reformat.xsl rtf-format.xml)
ADD_FORMATTER (apertium_desrtf_cp1250 deformat.xsl rtf-format-cp1250.xml)
ADD_FORMATTER (apertium_rertf_cp1250  reformat.xsl rtf-format-cp1250.xml)
ADD_FORMATTER (apertium_desrtf_cp1251 deformat.xsl rtf-format-cp1251.xml)
ADD_FORMATTER (apertium_rertf_cp1251  reformat.xsl rtf-format-cp1251.xml)

ADD_LIBRARY (apertium3 SHARED collection.cc constant_manager.cc hmm.cc lswpost.cc interchunk.cc
             interchunk_word.cc lextor.cc lextor_data.cc lextor_eval.cc
             lextor_word.cc morpho_stream.cc postchunk.cc string_utils.cc
             tagger.cc tagger_data.cc tagger_data_hmm.cc tagger_data_lsw.cc tagger_utils.cc tagger_word.cc
             transfer.cc transfer_data.cc transfer_instr.cc endian_double_util.cc
             transfer_token.cc transfer_word.cc transfer_word_list.cc
             trx_reader.cc tsx_reader.cc utf_converter.cc apertium_re.cc)

LIST (APPEND INSTALLABLES apertium3)

SET(EXECUTABLE_LIBS ${APERTIUM_LIBS} apertium3)

MACRO (ADD_APERTIUM_EXE EXE_NAME SOURCE_NAME)
        ADD_EXECUTABLE (${EXE_NAME} ${SOURCE_NAME})
        TARGET_LINK_LIBRARIES (${EXE_NAME} ${EXECUTABLE_LIBS})
        LIST (APPEND INSTALLABLES ${EXE_NAME})
ENDMACRO (ADD_APERTIUM_EXE)

ADD_APERTIUM_EXE (apertium-pretransfer                  apertium_pretransfer.cc)
ADD_APERTIUM_EXE (apertium-tagger                       apertium_tagger.cc)
ADD_APERTIUM_EXE (apertium-preprocess-transfer          transferpp.cc)
ADD_APERTIUM_EXE (apertium-filter-ambiguity             apertium_filter_ambiguity.cc)
ADD_APERTIUM_EXE (apertium-transfer                     apertium_transfer.cc)
ADD_APERTIUM_EXE (apertium-interchunk                   apertium_interchunk.cc)
ADD_APERTIUM_EXE (apertium-postchunk                    apertium_postchunk.cc)
#ADD_APERTIUM_EXE (apertium-lextor                       apertium_lextor.cc)
ADD_APERTIUM_EXE (apertium-tagger-apply-new-rules       apertium_tagger_apply_new_rules.cc)
ADD_APERTIUM_EXE (apertium-gen-wlist-lextor-translation apertium_gen_wlist_lextor_translation.cc)

MACRO (ADD_SCRIPT SCRIPT_NAME SOURCE)
	SET (OUTPUT_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${SCRIPT_NAME}")
	SET (SOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE}")

	FILE (READ ${SOURCE_FILE} SCRIPT)
	CONFIGURE_FILE (${SCRIPT_HEADER_FILE} ${OUTPUT_FILE})

	INSTALL (PROGRAMS    ${SCRIPT_NAME}
		 DESTINATION bin)

#	IF (WIN32)
#		INSTALL (PROGRAMS    "${CMAKE_CURRENT_SOURCE_DIR}/runner_skeleton.exe"
#		         DESTINATION bin
#			 RENAME      ${SCRIPT_NAME}.exe)
#       ENDIF (WIN32)
ENDMACRO (ADD_SCRIPT)

ADD_SCRIPT (apertium apertium-header.sh)

INSTALL (TARGETS ${INSTALLABLES}
         RUNTIME DESTINATION bin
         LIBRARY DESTINATION lib
         ARCHIVE DESTINATION lib)

FILE(GLOB HEADERS *.h)

INSTALL (FILES ${HEADERS}
         DESTINATION include/apertium-3.1/apertium)

IF (WIN32)
	INSTALL (FILES ${PROJECT_SOURCE_DIR}/etc/LEAVE_THIS_DIRECTORY_HERE.txt
		 DESTINATION etc)

	INSTALL (PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/sh.exe"
	                  "${CMAKE_CURRENT_SOURCE_DIR}/dirname.exe"
			  "${CMAKE_CURRENT_SOURCE_DIR}/basename.exe"
			  "${CMAKE_CURRENT_SOURCE_DIR}/msys-1.0.dll"
		 DESTINATION bin)
ENDIF (WIN32)
