SUBDIRS = $(GENERIC_LIBRARY_NAME)
DIST_SUBDIRS = $(GENERIC_LIBRARY_NAME)

modesdir=$(prefix)/share/apertium/modes/

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = apertium-$(GENERIC_RELEASE).pc

EXTRA_DIST=autogen.sh

install-data-local:
	mkdir -p $(modesdir)
	$(INSTALL_DATA) README-MODES  $(modesdir)/README
