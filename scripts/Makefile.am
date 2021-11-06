apertium-filter-dix: apertium-filter-dix.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

apertium-filter-rules: apertium-filter-rules.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

apertium-metalrx-to-lrx: apertium-metalrx-to-lrx.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

apertium-translate-to-default-equivalent: apertium-translate-to-default-equivalent.in
	$(SED) -e 's|[@]datarootdir@|$(datarootdir)|g' < "$<" > "$@" && chmod +x "$@"

dist_bin_SCRIPTS = \
	apertium-editdist \
	apertium-filter-dix \
	apertium-filter-rules \
	apertium-filter-xml \
	apertium-genvdix \
	apertium-genvldix \
	apertium-genvrdix \
	apertium-metalrx \
	apertium-metalrx-to-lrx \
	apertium-translate-to-default-equivalent

apertiumdir = $(prefix)/share/apertium
dist_apertium_DATA = \
	filter-dix.xslt \
	filter-rules.xslt \
	metalrx-to-lrx.xslt \
	translate-to-default-equivalent.xslt
