apertium-filter-dix: apertium-filter-dix.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

apertium-filter-rules: apertium-filter-rules.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

apertium-metalrx-to-lrx: apertium-metalrx-to-lrx.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

dist_bin_SCRIPTS = \
	apertium-filter-dix \
	apertium-filter-rules \
	apertium-genvdix \
	apertium-genvldix \
	apertium-genvrdix \
	apertium-metalrx \
	apertium-metalrx-to-lrx

apertiumdir = $(prefix)/share/apertium
dist_apertium_DATA = \
	filter-dix.xslt \
	filter-rules.xslt \
	metalrx-to-lrx.xslt
