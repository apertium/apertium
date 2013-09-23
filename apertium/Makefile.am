h_sources = collection.h constant_manager.h hmm.h lswpost.h interchunk.h \
            interchunk_word.h lextor_data.h lextor_eval.h lextor.h \
            lextor_word.h morpho_stream.h postchunk.h string_utils.h \
            tagger_data.h tagger_data_hmm.h tagger_data_lsw.h tagger.h tagger_utils.h tagger_word.h \
            transfer_data.h transfer.h transfer_instr.h transfer_token.h \
            transfer_word.h transfer_word_list.h trx_reader.h tsx_reader.h \
            ttag.h utf_converter.h apertium_re.h unlocked_cstdio.h \
            endian_double_util.h transfer_mult.h tmx_builder.h \
            tmx_alignment.h tmx_arguments_parser.h tmx_book_to_matrix.h \
            tmx_dictionary.h tmx_dic_tree.h tmx_quasi_diagonal.h \
            tmx_serialize_impl.h tmx_strings_and_streams.h \
            tmx_trail_postprocessors.h tmx_translate.h tmx_words.h \
            tmx_align_parameters.h tmx_aligner_tool.h latex_accentsmap.h

cc_sources = collection.cc constant_manager.cc hmm.cc lswpost.cc interchunk.cc \
             interchunk_word.cc lextor.cc lextor_data.cc lextor_eval.cc \
             lextor_word.cc morpho_stream.cc postchunk.cc string_utils.cc \
             tagger.cc tagger_data.cc tagger_data_hmm.cc tagger_data_lsw.cc tagger_utils.cc tagger_word.cc \
             transfer.cc transfer_data.cc transfer_instr.cc \
             transfer_token.cc transfer_word.cc transfer_word_list.cc \
             trx_reader.cc tsx_reader.cc utf_converter.cc apertium_re.cc \
             endian_double_util.cc transfer_mult.cc tmx_builder.cc \
             tmx_aligner_tool.cc tmx_alignment.cc tmx_arguments_parser.cc \
             tmx_book_to_matrix.cc tmx_dictionary.cc \
             tmx_strings_and_streams.cc tmx_trail_postprocessors.cc \
             tmx_translate.cc latex_accentsmap.cc
             
library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)
library_include_HEADERS = $(h_sources)

GENERATEDSCRIPTS = apertium-gen-deformat apertium-gen-reformat \
                   apertium-validate-tagger \
                   apertium-validate-transfer apertium-validate-dictionary \
                   apertium-validate-modes \
                   apertium-gen-lextorbil \
                   apertium-gen-lextormono apertium-gen-wlist-lextor \
                   apertium-preprocess-corpus-lextor \
                   apertium-gen-stopwords-lextor \
                   apertium-validate-interchunk \
                   apertium-validate-postchunk apertium apertium-unformat \
                   apertium-gen-modes apertium-validate-acx \
                   apertium-utils-fixlatex

lib_LTLIBRARIES = libapertium3.la
libapertium3_la_SOURCES = $(h_sources) $(cc_sources)
libapertium3_la_LDFLAGS = -version-info $(GENERIC_LIBRARY_VERSION) -release $(GENERIC_RELEASE)

bin_PROGRAMS = apertium-pretransfer apertium-destxt apertium-retxt \
               apertium-deshtml apertium-rehtml \
               apertium-rehtml-noent \
               apertium-desxpresstag apertium-rexpresstag \
               apertium-desodt apertium-reodt \
               apertium-desrtf apertium-rertf \
               apertium-deswxml apertium-rewxml \
               apertium-deslatex apertium-relatex \
               apertium-desxlsx apertium-rexlsx \
               apertium-despptx apertium-repptx \
               apertium-desmediawiki apertium-remediawiki \
               apertium-prelatex apertium-postlatex \
               apertium-postlatex-raw \
               apertium-tagger \
               apertium-preprocess-transfer apertium-transfer \
               apertium-filter-ambiguity apertium-lextor \
               apertium-gen-wlist-lextor-translation \
               apertium-interchunk apertium-postchunk \
               apertium-multiple-translations \
               apertium-tagger-apply-new-rules \
               apertium-tagger-readwords \
               apertium-tmxbuild
               ###pruebas-lextor apertium-lextor-search apertium-lextor-eval

bin_SCRIPTS =  $(GENERATEDSCRIPTS)

instdir = apertium

apertiumdir = $(prefix)/share/apertium
apertiuminclude = $(prefix)/include/apertium-$(GENERIC_API_VERSION)
apertiumlib = $(prefix)/lib 
apertiumsysconf = $(prefix)/etc/apertium

apertium_DATA = deformat.xsl reformat.xsl new2old.xsl lexchoice.xsl \
                lexchoicebil.xsl tagger.dtd interchunk.dtd format.dtd \
                transfer.dtd dix.dtd postchunk.dtd acx.rng modes.dtd modes2bash.xsl \
                apertium-createmodes.awk

apertium_pretransfer_SOURCES = apertium_pretransfer.cc
apertium_multiple_translations_SOURCES = apertium-multiple-translations.cc
apertium_multiple_translations_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)
apertium_destxt_SOURCES = apertium_destxt.cc
apertium_retxt_SOURCES = apertium_retxt.cc
apertium_deshtml_SOURCES = apertium_deshtml.cc
apertium_rehtml_SOURCES = apertium_rehtml.cc
apertium_rehtml_noent_SOURCES = apertium_rehtml_noent.cc
apertium_desxpresstag_SOURCES = apertium_desxpresstag.cc
apertium_rexpresstag_SOURCES = apertium_rexpresstag.cc
apertium_desodt_SOURCES = apertium_desodt.cc
apertium_reodt_SOURCES = apertium_reodt.cc
apertium_desrtf_SOURCES = apertium_desrtf.cc
apertium_rertf_SOURCES = apertium_rertf.cc
apertium_deswxml_SOURCES = apertium_deswxml.cc
apertium_rewxml_SOURCES = apertium_rewxml.cc
apertium_deslatex_SOURCES = apertium_deslatex.cc
apertium_relatex_SOURCES = apertium_relatex.cc
apertium_desxlsx_SOURCES = apertium_desxlsx.cc
apertium_rexlsx_SOURCES = apertium_rexlsx.cc
apertium_despptx_SOURCES = apertium_despptx.cc
apertium_repptx_SOURCES = apertium_repptx.cc
apertium_desmediawiki_SOURCES = apertium_desmediawiki.cc
apertium_remediawiki_SOURCES = apertium_remediawiki.cc
apertium_prelatex_SOURCES = apertium_prelatex.cc
apertium_prelatex_LDADD= $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)
apertium_postlatex_SOURCES = apertium_postlatex.cc
apertium_postlatex_LDADD= $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)
apertium_postlatex_raw_SOURCES = apertium_postlatex_raw.cc
apertium_postlatex_raw_LDADD= $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_tagger_SOURCES = apertium_tagger.cc
apertium_tagger_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_tmxbuild_SOURCES = apertium_tmxbuild.cc
apertium_tmxbuild_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_preprocess_transfer_SOURCES = transferpp.cc
apertium_preprocess_transfer_LDADD = $(APERTIUM_LIBS) \
                                     -lapertium$(GENERIC_MAJOR_VERSION)

apertium_filter_ambiguity_SOURCES = apertium_filter_ambiguity.cc
apertium_filter_ambiguity_LDADD = $(APERTIUM_LIBS) \
                                  -lapertium$(GENERIC_MAJOR_VERSION)

apertium_transfer_SOURCES = apertium_transfer.cc
apertium_transfer_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_interchunk_SOURCES = apertium_interchunk.cc
apertium_interchunk_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_postchunk_SOURCES = apertium_postchunk.cc
apertium_postchunk_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_lextor_SOURCES = apertium_lextor.cc
apertium_lextor_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

#apertium_lextor_eval_SOURCES = apertium-lextor-eval.C
#apertium_lextor_eval_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_tagger_apply_new_rules_SOURCES = apertium_tagger_apply_new_rules.cc
apertium_tagger_apply_new_rules_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_tagger_readwords_SOURCES = apertium_tagger_readwords.cc
apertium_tagger_readwords_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

###apertium_lextor_search_SOURCES = apertium-lextor-search.C
###apertium_lextor_search_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

###pruebas_lextor_SOURCES = pruebas-lextor.C
###pruebas_lextor_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)

apertium_gen_wlist_lextor_translation_SOURCES = apertium_gen_wlist_lextor_translation.cc
apertium_gen_wlist_lextor_translation_LDADD = $(APERTIUM_LIBS) -lapertium$(GENERIC_MAJOR_VERSION)


INCLUDES = -I$(top_srcdir) $(APERTIUM_CFLAGS)
CLEANFILES = *~ apertium_destxt.cc apertium_retxt.cc apertium_deshtml.cc \
             apertium_rehtml.cc apertium_desrtf.cc apertium_rertf.cc \
             apertium_rehtml_noent.cc \
             apertium_deswxml.cc apertium_rewxml.cc \
             apertium_deslatex.cc apertium_relatex.cc \
             apertium_desxlsx.cc apertium_rexlsx.cc \
             apertium_despptx.cc apertium_repptx.cc \
             apertium_desodt.cc apertium_reodt.cc \
	     apertium_desxpresstag.cc apertium_rexpresstag.cc \
             apertium_desmediawiki.cc apertium_remediawiki.cc \
             apertium_prelatex.cc apertium_postlatex.cc \
             $(GENERATEDSCRIPTS)

apertium_destxt.cc: txt-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl txt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desxpresstag.cc: xpresstag-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl xpresstag-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rexpresstag.cc: xpresstag-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl xpresstag-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp	

apertium_retxt.cc: txt-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl txt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deshtml.cc: html-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl html-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rehtml.cc: html-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl html-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rehtml_noent.cc: html-noent-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl html-noent-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desodt.cc: odt-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl odt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_reodt.cc: odt-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl odt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desrtf.cc: rtf-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl rtf-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rertf.cc: rtf-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl rtf-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deswxml.cc: wxml-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl wxml-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rewxml.cc: wxml-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl wxml-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deslatex.cc: latex-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl latex-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_relatex.cc: latex-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl latex-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp



apertium_desxlsx.cc: xlsx-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl xlsx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rexlsx.cc: xlsx-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl xlsx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_despptx.cc: pptx-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl pptx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_repptx.cc: pptx-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl pptx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desmediawiki.cc: mediawiki-format.xml Makefile.am deformat.xsl
	$(XSLTPROC) deformat.xsl mediawiki-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_remediawiki.cc: mediawiki-format.xml Makefile.am reformat.xsl
	$(XSLTPROC) reformat.xsl mediawiki-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp
	
apertium_prelatex.cc: apertium-prelatex.l
	$(FLEX) -Cfer -o$@ apertium-prelatex.l

apertium_postlatex.cc: apertium-postlatex.l
	$(FLEX) -Cfer -o$@ apertium-postlatex.l

apertium_postlatex_raw.cc: apertium-postlatex-raw.l
	$(FLEX) -Cfer -o$@ apertium-postlatex-raw.l
	
apertium-validate-tagger: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-tagger script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/tagger.dtd --noout \$$FILE1 && exit 0;" >>$@
	@echo "exit 1;" >> $@
	@chmod a+x $@

apertium-validate-transfer: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-transfer script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/transfer.dtd --noout \$$FILE1 && exit 0;" >>$@
	@echo "exit 1;" >> $@
	@chmod a+x $@

apertium-validate-interchunk: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-interchunk script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/interchunk.dtd --noout \$$FILE1 && exit 0;" >>$@
	@echo "exit 1;" >> $@
	@chmod a+x $@

apertium-validate-postchunk: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-postchunk script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/postchunk.dtd --noout \$$FILE1 && exit 0;" >>$@
	@echo "exit 1;" >> $@
	@chmod a+x $@

apertium-validate-acx: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-acx script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --relaxng $(apertiumdir)/acx.rng --noout \$$FILE1 && exit 0;" >>$@
	@echo "exit 1;" >> $@
	@chmod a+x $@

apertium-validate-modes: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-modes script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/modes.dtd --noout \$$FILE1 && exit 0;" >>$@
	@echo "exit 1;" >> $@
	@chmod a+x $@


apertium-validate-dictionary: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-dictionary script"
	@echo "#!$(BASH)" > $@
	@cat validate-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/dix.dtd --noout \$$FILE1 && exit 0;" >> $@
	@echo "exit 1;" >> $@
	@chmod a+x $@

apertium-gen-deformat: Makefile.am deformat-header.sh
	@echo "Creating apertium-gen-deformat script"
	@echo "#!$(BASH)" > $@
	@cat deformat-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/format.dtd --noout \$$FILE1 && \\" >> $@
	@if [ `basename $(XSLTPROC)` == xsltproc ]; \
	  then echo "$(XSLTPROC) --stringparam mode \$$MODE $(apertiumdir)/deformat.xsl \$$FILE1 >/tmp/\$$\$$.deformat.l && \\"; \
          else echo "$(XSLTPROC) $(apertiumdir)/deformat.xsl \$$FILE1 \"\\\$$mode=\$$MODE\" >/tmp/\$$\$$.deformat.l && \\"; \
          fi >> $@ 
	@echo "$(FLEX) \$$FLEXOPTS -o/tmp/\$$\$$.lex.cc /tmp/\$$\$$.deformat.l && \\" >> $@
	@echo "$(CXX) -DGENFORMAT $(CXXFLAGS) -w $(APERTIUM_CFLAGS) -I $(apertiuminclude) -o \$$FILE2 /tmp/\$$\$$.lex.cc $(APERTIUM_LIBS) 2>/dev/null && \\" >> $@
	@echo "rm /tmp/\$$\$$.deformat.l /tmp/\$$\$$.lex.cc" >> $@
	@chmod a+x $@

apertium-gen-reformat: Makefile.am gen-header.sh
	@echo "Creating apertium-gen-reformat script"
	@echo "#!$(BASH)" > $@
	@cat gen-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/format.dtd --noout \$$FILE1 && \\" >> $@
	@echo "$(XSLTPROC) $(apertiumdir)/reformat.xsl \$$FILE1 >/tmp/\$$\$$.reformat.l && \\" >> $@
	@echo "$(FLEX) \$$FLEXOPTS -o/tmp/\$$\$$.lex.cc /tmp/\$$\$$.reformat.l && \\" >> $@
	@echo "$(CXX) -DGENFORMAT $(CXXFLAGS) -w $(APERTIUM_CFLAGS) -I $(apertiuminclude) -o \$$FILE2 /tmp/\$$\$$.lex.cc $(APERTIUM_LIBS) 2>/dev/null &&\\" >> $@
	@echo "rm /tmp/\$$\$$.reformat.l /tmp/\$$\$$.lex.cc" >> $@
	@chmod a+x $@

apertium-gen-modes: Makefile.am modes-header.sh
	@echo "Creating apertium-gen-modes script"
	@echo "#!$(BASH)" > $@
	@echo "APERTIUMDIR="$(apertiumdir) >> $@
	@cat modes-header.sh >> $@
	@echo "$(XMLLINT) --dtdvalid $(apertiumdir)/modes.dtd --noout \$$FILE1 && \\" >> $@
	@if [ `basename $(XSLTPROC)` == xsltproc ]; \
	  then echo "$(XSLTPROC) --stringparam prefix $(prefix)/bin --stringparam dataprefix \$$FULLDIRNAME  $(apertiumdir)/modes2bash.xsl \$$FILE1 | awk -f $(apertiumdir)/apertium-createmodes.awk PARAM=\$$FULLDIRNAME"; \
          else echo "$(XSLTPROC) $(apertiumdir)/modes2bash.xsl \$$FILE1 \\\$$prefix=$(prefix)/bin \\\$$dataprefix=\$$FULLDIRNAME| awk -f $(apertiumdir)/apertium-createmodes.awk PARAM=\$$FULLDIRNAME"; \
          fi >> $@ 
	@chmod a+x $@

apertium-utils-fixlatex: Makefile.am utils-fixlatex-header.sh
	@echo "Creating apertium-utils-fixlatex script"
	@echo "#!$(BASH)" > $@
	@cat utils-fixlatex-header.sh >> $@
	@chmod a+x $@

apertium: Makefile.am apertium-header.sh
	@echo "Creating apertium script"
	@echo "#!$(BASH)" > $@
	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
	@echo "DEFAULT_DIRECTORY=\"$(prefix)/share/apertium\"" >>$@
	@cat apertium-header.sh >>$@
	@chmod a+x $@

apertium-unformat: Makefile.am apertium-unformat-header.sh
	@echo "Creating apertium-unformat script"
	@echo "#!$(BASH)" > $@
	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
	@echo "DEFAULT_DIRECTORY=\"$(prefix)/share/apertium\"" >>$@
	@cat apertium-unformat-header.sh >>$@
	@chmod a+x $@


#apertium-translator-lextor: Makefile.am trans-lextor-header.sh
#	@echo "Creating apertium-translator-lextor script"
#	@echo "#!$(BASH)" > $@
#	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
#	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
#	@cat trans-lextor-header.sh >>$@
#	@chmod a+x $@

#apertium-gen-oldbil: Makefile.am transformdicbil-header.sh
#	@echo "Creating apertium-gen-oldbil script"
#	@echo "#!$(BASH)" >$@
#	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
#	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
#	@echo "XSLTPROC_OPTIONS=\"\"">>$@
#	@echo "STYLESHEET=\"$(apertiumdir)/new2old.xsl\"">>$@
#	@cat transformdicbil-header.sh >>$@
#	@chmod a+x $@

apertium-gen-lextorbil: Makefile.am transformdic-header.sh
	@echo "Creating apertium-gen-lextorbil script"
	@echo "#!$(BASH)" >$@
	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
	@echo "XSLTPROC_OPTIONS_LR=\"\"">>$@
	@echo "XSLTPROC_OPTIONS_RL=\"--stringparam r2l yes\"">>$@
	@echo "STYLESHEET=\"$(apertiumdir)/lexchoicebil.xsl\"">>$@
	@cat transformdic-header.sh >>$@
	@chmod a+x $@

apertium-gen-lextormono: Makefile.am transformdic-header.sh
	@echo "Creating apertium-gen-lextormono script"
	@echo "#!$(BASH)" >$@
	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
	@echo "XSLTPROC_OPTIONS_LR=\"\"">>$@
	@echo "XSLTPROC_OPTIONS_RL=\"--stringparam r2l yes\"">>$@
	@echo "STYLESHEET=\"$(apertiumdir)/lexchoice.xsl\"">>$@
	@cat transformdic-header.sh >>$@
	@chmod a+x $@

apertium-gen-wlist-lextor: Makefile.am gen-wlist-lextor-header.sh
	@echo "Creating apertium-gen-wlist-lextor script"
	@echo "#!$(BASH)" >$@
	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
	@cat gen-wlist-lextor-header.sh >>$@
	@chmod a+x $@

apertium-preprocess-corpus-lextor: Makefile.am preprocess-corpus-lextor.sh
	@echo "Creating apertium-preprocess-corpus-lextor script"
	@echo "#!$(BASH)" >$@
	@echo "LTTOOLBOX_PATH=\"$(prefix)/bin\"" >>$@
	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
	@cat preprocess-corpus-lextor.sh >>$@
	@chmod a+x $@

apertium-gen-stopwords-lextor: Makefile.am gen-stopwords-lextor.sh
	@echo "Creating apertium-gen-stopwords-lextor script"
	@echo "#!$(BASH)" >$@
	@cat gen-stopwords-lextor.sh >>$@
	@chmod a+x $@

man_MANS=apertium.1 apertium-deshtml.1 apertium-desrtf.1 apertium-destxt.1 \
         apertium-desodt.1 apertium-reodt.1 \
         apertium-deswxml.1 apertium-rewxml.1 \
         apertium-deslatex.1 apertium-relatex.1 \
         apertium-prelatex.1 apertium-postlatex.1 apertium-postlatex-raw.1 \
         apertium-desxlsx.1 apertium-rexlsx.1 \
         apertium-despptx.1 apertium-repptx.1 \
         apertium-desmediawiki.1 apertium-remediawiki.1 \
         apertium-filter-ambiguity.1 apertium-gen-deformat.1 apertium-gen-lextorbil.1 \
         apertium-gen-lextormono.1 apertium-gen-reformat.1 apertium-gen-stopwords-lextor.1 \
         apertium-gen-wlist-lextor.1 apertium-gen-wlist-lextor-translation.1 \
         apertium-lextor.1 apertium-preprocess-corpus-lextor.1 \
         apertium-preprocess-transfer.1 apertium-pretransfer.1 apertium-rehtml.1 \
         apertium-rertf.1 apertium-retxt.1 apertium-tagger.1 apertium-transfer.1 \
         apertium-validate-dictionary.1 apertium-validate-tagger.1 \
         apertium-validate-transfer.1 apertium-gen-modes.1 apertium-interchunk.1 \
         apertium-postchunk.1 apertium-validate-interchunk.1 apertium-utils-fixlatex.1 \
         apertium-validate-postchunk.1 apertium-validate-modes.1 apertium-tagger-apply-new-rules.1 \
	 apertium-validate-acx.1 apertium-multiple-translations.1 \
	 apertium-unformat.1
         ###apertium-lextor-eval.1 

EXTRA_DIST = gen-header.sh deformat-header.sh \
             reformat.xsl deformat.xsl new2old.xsl lexchoice.xsl lexchoicebil.xsl \
             tagger.dtd transfer.dtd format.dtd dix.dtd txt-format.xml \
             html-format.xml odt-format.xml rtf-format.xml wxml-format.xml latex-format.xml\
             html-noent-format.xml \
             xlsx-format.xml pptx-format.xml mediawiki-format.xml trans-header.sh \
             apertium-header.sh apertium-unformat-header.sh $(man_MANS) \
             xpresstag-format.xml \
             validate-header.sh transformdic-header.sh transformdicbil-header.sh \
             gen-wlist-lextor-header.sh gen-stopwords-lextor.sh preprocess-corpus-lextor.sh \
             interchunk.dtd postchunk.dtd modes.dtd acx.rng \
             utils-fixlatex-header.sh \
             modes-header.sh apertium-createmodes.awk modes2bash.xsl ###trans-lextor-header.sh 
