AUTOMAKE_OPTIONS = subdir-objects

h_sources = a.h \
	    align.h \
	    analysis.h \
	    apertium_re.h \
	    collection.h \
	    constant_manager.h \
	    deserialiser.h \
	    endian_double_util.h \
	    exception.h \
	    exception_type.h \
	    feature_vec.h \
	    feature_vec_averager.h \
	    file_tagger.h \
	    getopt_long.h \
	    hmm.h \
	    i.h \
	    interchunk.h \
	    interchunk_word.h \
	    latex_accentsmap.h \
	    lemma.h \
	    lexical_unit.h \
	    linebreak.h \
	    lswpost.h \
	    morpheme.h \
	    morpho_stream.h \
	    mtx_reader.h \
	    file_morpho_stream.h \
	    optional.h \
	    perceptron_spec.h \
	    perceptron_tagger.h \
	    pretransfer.h \
	    postchunk.h \
	    sentence_stream.h \
	    serialiser.h \
	    stream.h \
	    stream_tagger.h \
	    streamed_type.h \
	    string_to_wostream.h \
	    string_utils.h \
	    shell_utils.h \
	    tag.h \
	    tagger.h \
	    tagger_data.h \
	    tagger_data_hmm.h \
	    tagger_data_lsw.h \
	    tagger_data_percep_coarse_tags.h \
	    tagger_flags.h \
	    tagger_utils.h \
	    tagger_word.h \
	    tmx_aligner_tool.h \
	    tmx_alignment.h \
	    tmx_align_parameters.h \
	    tmx_arguments_parser.h \
	    tmx_book_to_matrix.h \
	    tmx_builder.h \
	    tmx_dictionary.h \
	    tmx_dic_tree.h \
	    tmx_quasi_diagonal.h \
	    tmx_serialize_impl.h \
	    tmx_strings_and_streams.h \
	    tmx_trail_postprocessors.h \
	    tmx_translate.h \
	    tmx_words.h \
	    transfer_data.h \
	    transfer.h \
	    transfer_instr.h \
	    transfer_mult.h \
	    transfer_token.h \
	    transfer_word.h \
	    transfer_word_list.h \
	    trx_reader.h \
	    tsx_reader.h \
	    ttag.h \
	    unigram_tagger.h \
	    unlocked_cstdio.h \
	    utf_converter.h \
	    utils.h \
	    xml_reader.h

cc_sources = a.cc \
	     align.cc \
	     analysis.cc \
	     apertium_re.cc \
	     collection.cc \
	     constant_manager.cc \
	     endian_double_util.cc \
	     exception_type.cc \
	     feature_vec.cc \
	     feature_vec_averager.cc \
	     file_tagger.cc \
	     hmm.cc \
	     i.cc \
	     interchunk.cc \
	     interchunk_word.cc \
	     latex_accentsmap.cc \
	     lemma.cc \
	     linebreak.cc \
	     lswpost.cc \
	     morpheme.cc \
	     morpho_stream.cc \
	     mtx_reader.cc \
	     file_morpho_stream.cc \
	     perceptron_spec.cc \
	     perceptron_tagger.cc \
	     pretransfer.cc \
	     postchunk.cc \
	     sentence_stream.cc \
	     stream.cc \
	     stream_tagger.cc \
	     string_utils.cc \
	     shell_utils.cc \
	     tag.cc \
	     tagger.cc \
	     tagger_data.cc \
	     tagger_data_hmm.cc \
	     tagger_data_lsw.cc \
	     tagger_data_percep_coarse_tags.cc \
	     tagger_flags.cc \
	     tagger_utils.cc \
	     tagger_word.cc \
	     tmx_aligner_tool.cc \
	     tmx_alignment.cc \
	     tmx_arguments_parser.cc \
	     tmx_book_to_matrix.cc \
	     tmx_builder.cc \
	     tmx_dictionary.cc \
	     tmx_strings_and_streams.cc \
	     tmx_trail_postprocessors.cc \
	     tmx_translate.cc \
	     transfer.cc \
	     transfer_data.cc \
	     transfer_instr.cc \
	     transfer_mult.cc \
	     transfer_token.cc \
	     transfer_word.cc \
	     transfer_word_list.cc \
	     trx_reader.cc \
	     tsx_reader.cc \
	     unigram_tagger.cc \
	     utf_converter.cc \
	     xml_reader.cc

library_includedir = $(includedir)/$(PACKAGE_NAME)-$(VERSION_API)/$(PACKAGE_NAME)
library_include_HEADERS = $(h_sources)

GENERATEDSCRIPTS = apertium-gen-deformat apertium-gen-reformat \
                   apertium-validate-tagger \
                   apertium-validate-transfer apertium-validate-dictionary \
                   apertium-validate-modes \
                   apertium-validate-interchunk \
                   apertium-validate-postchunk apertium apertium-unformat \
                   apertium-gen-modes apertium-validate-acx \
                   apertium-utils-fixlatex

lib_LTLIBRARIES = libapertium3.la
libapertium3_la_SOURCES = $(h_sources) $(cc_sources)
libapertium3_la_LDFLAGS = -version-info $(SOVERSION) -release $(VERSION_API)

bin_PROGRAMS = apertium-cleanstream \
	       apertium-deshtml \
	       apertium-deshtml-alt \
	       apertium-deshtml-noent \
	       apertium-deslatex \
	       apertium-desmediawiki \
	       apertium-desodt \
	       apertium-despptx \
	       apertium-desrtf \
	       apertium-destxt \
	       apertium-deswxml \
	       apertium-desxlsx \
	       apertium-desxpresstag \
	       apertium-filter-ambiguity \
	       apertium-interchunk \
	       apertium-multiple-translations \
	       apertium-perceptron-trace \
	       apertium-postchunk \
	       apertium-postlatex \
	       apertium-postlatex-raw \
	       apertium-prelatex \
	       apertium-preprocess-transfer \
	       apertium-pretransfer \
	       apertium-posttransfer \
	       apertium-rehtml \
	       apertium-rehtml-noent \
	       apertium-rehtml-alt \
	       apertium-relatex \
	       apertium-remediawiki \
	       apertium-reodt \
	       apertium-repptx \
	       apertium-rertf \
	       apertium-retxt \
	       apertium-rewxml \
	       apertium-rexlsx \
	       apertium-rexpresstag \
	       apertium-tagger \
	       apertium-tagger-apply-new-rules \
	       apertium-tagger-readwords \
	       apertium-perceptron-trace \
	       apertium-tmxbuild \
	       apertium-transfer \
	       apertium-wblank-attach \
	       apertium-wblank-detach \
	       apertium-wblank-mode \
	       apertium-adapt-docx

bin_SCRIPTS = $(GENERATEDSCRIPTS)

instdir = apertium

apertiumdir = $(prefix)/share/apertium
apertiuminclude = $(prefix)/include/apertium-$(VERSION_API)
apertiumlib = $(prefix)/lib
apertiumsysconf = $(prefix)/etc/apertium

apertium_DATA = deformat.xsl reformat.xsl new2old.xsl lexchoice.xsl \
                lexchoicebil.xsl \
				tagger.dtd interchunk.dtd format.dtd  transfer.dtd postchunk.dtd modes.dtd \
				tagger.rnc interchunk.rnc format.rnc  transfer.rnc postchunk.rnc modes.rnc \
				modes2bash.xsl modes2debugmodes.xsl \
                apertium-createmodes.awk

apertium_cleanstream_SOURCES = apertium_cleanstream.cc
apertium_pretransfer_SOURCES = apertium_pretransfer.cc
apertium_pretransfer_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)
apertium_posttransfer_SOURCES = apertium_posttransfer.cc
apertium_posttransfer_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)
apertium_multiple_translations_SOURCES = apertium-multiple-translations.cc
apertium_multiple_translations_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)
apertium_destxt_SOURCES = apertium_destxt.cc
apertium_retxt_SOURCES = apertium_retxt.cc
apertium_deshtml_SOURCES = apertium_deshtml.cc
apertium_deshtml_alt_SOURCES = apertium_deshtml_alt.cc
apertium_deshtml_noent_SOURCES = apertium_deshtml_noent.cc
apertium_rehtml_SOURCES = apertium_rehtml.cc
apertium_rehtml_noent_SOURCES = apertium_rehtml_noent.cc
apertium_rehtml_alt_SOURCES = apertium_rehtml_alt.cc
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
apertium_prelatex_LDADD= -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)
apertium_postlatex_SOURCES = apertium_postlatex.cc
apertium_postlatex_LDADD= -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)
apertium_postlatex_raw_SOURCES = apertium_postlatex_raw.cc
apertium_postlatex_raw_LDADD= -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_tagger_SOURCES = apertium_tagger.cc
apertium_tagger_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_perceptron_trace_SOURCES = apertium_perceptron_trace.cc
apertium_perceptron_trace_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_tmxbuild_SOURCES = apertium_tmxbuild.cc
apertium_tmxbuild_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_preprocess_transfer_SOURCES = transferpp.cc
apertium_preprocess_transfer_LDADD = \
                                     -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_filter_ambiguity_SOURCES = apertium_filter_ambiguity.cc
apertium_filter_ambiguity_LDADD = \
                                  -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_transfer_SOURCES = apertium_transfer.cc
apertium_transfer_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_interchunk_SOURCES = apertium_interchunk.cc
apertium_interchunk_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_postchunk_SOURCES = apertium_postchunk.cc
apertium_postchunk_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_tagger_apply_new_rules_SOURCES = apertium_tagger_apply_new_rules.cc
apertium_tagger_apply_new_rules_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_tagger_readwords_SOURCES = apertium_tagger_readwords.cc
apertium_tagger_readwords_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES)

apertium_wblank_attach_SOURCES = wblank-attach.cc
apertium_wblank_detach_SOURCES = wblank-detach.cc
apertium_wblank_mode_SOURCES = wblank-mode.cc

apertium_adapt_docx_SOURCES = adapt_docx.cc
apertium_adapt_docx_LDADD = -lapertium$(VERSION_MAJOR) $(lib_LTLIBRARIES) $(ICU_LIBS)


if WINDOWS
AM_CPPFLAGS = -I$(top_srcdir)/utf8 -I$(top_srcdir)/apertium/win32 -I$(top_srcdir) $(APERTIUM_CFLAGS) $(ICU_CFLAGS)
else
AM_CPPFLAGS = -I$(top_srcdir) $(APERTIUM_CFLAGS) $(ICU_CFLAGS)
endif
CLEANFILES = *~ apertium_destxt.cc apertium_retxt.cc apertium_deshtml.cc \
             apertium_rehtml.cc apertium_desrtf.cc apertium_rertf.cc \
             apertium_rehtml_noent.cc \
             apertium_rehtml_alt.cc \
             apertium_deshtml_alt.cc \
             apertium_deshtml_noent.cc \
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
	xsltproc deformat.xsl txt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desxpresstag.cc: xpresstag-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl xpresstag-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rexpresstag.cc: xpresstag-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl xpresstag-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_retxt.cc: txt-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl txt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deshtml.cc: html-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl html-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deshtml_alt.cc: html-alt-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl $< >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deshtml_noent.cc: html-noent-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl $< >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rehtml.cc: html-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl html-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rehtml_noent.cc: html-noent-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl html-noent-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rehtml_alt.cc: html-alt-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl html-alt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desodt.cc: odt-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl odt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_reodt.cc: odt-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl odt-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desrtf.cc: rtf-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl rtf-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rertf.cc: rtf-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl rtf-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deswxml.cc: wxml-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl wxml-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rewxml.cc: wxml-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl wxml-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_deslatex.cc: latex-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl latex-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_relatex.cc: latex-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl latex-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp



apertium_desxlsx.cc: xlsx-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl xlsx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_rexlsx.cc: xlsx-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl xlsx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_despptx.cc: pptx-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl pptx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_repptx.cc: pptx-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl pptx-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_desmediawiki.cc: mediawiki-format.xml Makefile.am deformat.xsl
	xsltproc deformat.xsl mediawiki-format.xml >$@tmp
	$(FLEX) -Cfer -o$@ $@tmp
	rm $@tmp

apertium_remediawiki.cc: mediawiki-format.xml Makefile.am reformat.xsl
	xsltproc reformat.xsl mediawiki-format.xml >$@tmp
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
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/tagger.dtd --noout \"\$$FILE1\"" >>$@
	@chmod a+x $@

apertium-validate-transfer: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-transfer script"
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/transfer.dtd --noout \"\$$FILE1\"" >>$@
	@chmod a+x $@

apertium-validate-interchunk: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-interchunk script"
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/interchunk.dtd --noout \"\$$FILE1\"" >>$@
	@chmod a+x $@

apertium-validate-postchunk: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-postchunk script"
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/postchunk.dtd --noout \"\$$FILE1\"" >>$@
	@chmod a+x $@

apertium-validate-acx: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-acx script"
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "xmllint --relaxng \"$(prefix)\"/share/lttoolbox/acx.rng --schema \"$(prefix)\"/share/lttoolbox/acx.xsd --noout \"\$$FILE1\"" >>$@
	@chmod a+x $@

apertium-validate-modes: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-modes script"
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/modes.dtd --noout \"\$$FILE1\"" >>$@
	@chmod a+x $@


apertium-validate-dictionary: Makefile.am validate-header.sh
	@echo "Creating apertium-validate-dictionary script"
	@echo "#!/usr/bin/env bash" > $@
	@cat validate-header.sh >> $@
	@echo "# xsd is a non-final command, so just treated as a warning when compiling:" >> $@
	@echo "xmllint --schema \"$(prefix)\"/share/lttoolbox/dix.xsd --noout \"\$$FILE1\" 2>&1 | grep -ve ' validates$$' -e ' fails to validate$$'" >> $@
	@echo "xmllint --dtdvalid \"$(prefix)\"/share/lttoolbox/dix.dtd --noout \"\$$FILE1\"" >> $@
	@chmod a+x $@

apertium-gen-deformat: Makefile.am deformat-header.sh
	@echo "Creating apertium-gen-deformat script"
	@echo "#!/usr/bin/env bash" > $@
	@cat deformat-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/format.dtd --noout \"\$$FILE1\" && \\" >> $@
	@if [ `basename xsltproc` == xsltproc ]; \
	  then echo "xsltproc --stringparam mode \$$MODE \"$(apertiumdir)\"/deformat.xsl \"\$$FILE1\" >/tmp/\$$\$$.deformat.l && \\"; \
          else echo "xsltproc \"$(apertiumdir)\"/deformat.xsl \"\$$FILE1\" \"\\\$$mode=\$$MODE\" >/tmp/\$$\$$.deformat.l && \\"; \
          fi >> $@
	@echo "$(FLEX) \$$FLEXOPTS -o/tmp/\$$\$$.lex.cc /tmp/\$$\$$.deformat.l && \\" >> $@
	@echo "$(CXX) -DGENFORMAT $(CXXFLAGS) -w $(APERTIUM_CFLAGS) -I $(apertiuminclude) -o \"\$$FILE2\" /tmp/\$$\$$.lex.cc 2>/dev/null && \\" >> $@
	@echo "rm /tmp/\$$\$$.deformat.l /tmp/\$$\$$.lex.cc" >> $@
	@chmod a+x $@

apertium-gen-reformat: Makefile.am gen-header.sh
	@echo "Creating apertium-gen-reformat script"
	@echo "#!/usr/bin/env bash" > $@
	@cat gen-header.sh >> $@
	@echo "xmllint --dtdvalid \"$(apertiumdir)\"/format.dtd --noout \"\$$FILE1\" && \\" >> $@
	@echo "xsltproc \"$(apertiumdir)\"/reformat.xsl \"\$$FILE1\" >/tmp/\$$\$$.reformat.l && \\" >> $@
	@echo "$(FLEX) \$$FLEXOPTS -o/tmp/\$$\$$.lex.cc /tmp/\$$\$$.reformat.l && \\" >> $@
	@echo "$(CXX) -DGENFORMAT $(CXXFLAGS) -w $(APERTIUM_CFLAGS) -I $(apertiuminclude) -o \"\$$FILE2\" /tmp/\$$\$$.lex.cc 2>/dev/null &&\\" >> $@
	@echo "rm /tmp/\$$\$$.reformat.l /tmp/\$$\$$.lex.cc" >> $@
	@chmod a+x $@

apertium-gen-modes: apertium-gen-modes.in Makefile.am
	@echo "#!/usr/bin/env bash" > $@
	@echo "APERTIUMDIR=$(apertiumdir)" >> $@
	@echo "XMLLINT=xmllint" >> $@
	@echo "XSLTPROC=xsltproc" >> $@
	@cat $< >> $@
	@chmod a+x $@

apertium-utils-fixlatex: Makefile.am utils-fixlatex-header.sh
	@echo "Creating apertium-utils-fixlatex script"
	@echo "#!/usr/bin/env bash" > $@
	@cat utils-fixlatex-header.sh >> $@
	@chmod a+x $@

apertium: Makefile.am apertium-header.sh
	@echo 'Creating apertium script'
	@echo '#!/usr/bin/env bash' > $@
	@echo 'APERTIUM_PATH="$${APERTIUM_PATH:-$(prefix)/bin}"' >>$@
	@echo 'APERTIUM_DATADIR="$${APERTIUM_DATADIR:-$(prefix)/share/apertium}"' >>$@
	@cat apertium-header.sh >>$@
	@chmod a+x $@

apertium-unformat: Makefile.am apertium-unformat-header.sh
	@echo "Creating apertium-unformat script"
	@echo "#!/usr/bin/env bash" > $@
	@echo "APERTIUM_PATH=\"$(prefix)/bin\"" >>$@
	@cat apertium-unformat-header.sh >>$@
	@chmod a+x $@

man_MANS=apertium.1 apertium-deshtml.1 apertium-desrtf.1 apertium-destxt.1 \
         apertium-deshtml-alt.1 \
         apertium-desodt.1 apertium-reodt.1 \
         apertium-deswxml.1 apertium-rewxml.1 \
         apertium-deslatex.1 apertium-relatex.1 \
         apertium-prelatex.1 apertium-postlatex.1 apertium-postlatex-raw.1 \
         apertium-desxlsx.1 apertium-rexlsx.1 \
         apertium-despptx.1 apertium-repptx.1 \
         apertium-desmediawiki.1 apertium-remediawiki.1 \
         apertium-filter-ambiguity.1 apertium-gen-deformat.1 \
         apertium-gen-reformat.1 \
         apertium-preprocess-transfer.1 apertium-pretransfer.1 apertium-rehtml.1 \
         apertium-rertf.1 apertium-retxt.1 apertium-tagger.1 apertium-transfer.1 \
         apertium-validate-dictionary.1 apertium-validate-tagger.1 \
         apertium-validate-transfer.1 apertium-gen-modes.1 apertium-interchunk.1 \
         apertium-postchunk.1 apertium-validate-interchunk.1 apertium-utils-fixlatex.1 \
         apertium-validate-postchunk.1 apertium-validate-modes.1 apertium-tagger-apply-new-rules.1 \
	 apertium-validate-acx.1 apertium-multiple-translations.1 \
	 apertium-unformat.1

EXTRA_DIST = gen-header.sh deformat-header.sh \
             reformat.xsl deformat.xsl new2old.xsl lexchoice.xsl lexchoicebil.xsl \
             txt-format.xml \
             html-format.xml odt-format.xml rtf-format.xml wxml-format.xml latex-format.xml\
             html-noent-format.xml \
             xlsx-format.xml pptx-format.xml mediawiki-format.xml trans-header.sh \
             apertium-postlatex.l  apertium-postlatex-raw.l  apertium-prelatex.l \
             apertium-header.sh apertium-unformat-header.sh $(man_MANS) \
             xpresstag-format.xml \
             validate-header.sh transformdic-header.sh transformdicbil-header.sh \
			 tagger.dtd interchunk.dtd format.dtd  transfer.dtd postchunk.dtd modes.dtd \
			 tagger.rnc interchunk.rnc format.rnc  transfer.rnc postchunk.rnc modes.rnc \
             utils-fixlatex-header.sh \
             apertium-gen-modes.in apertium-createmodes.awk modes2bash.xsl modes2debugmodes.xsl
