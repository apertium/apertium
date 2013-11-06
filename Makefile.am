SUBDIRS = $(GENERIC_LIBRARY_NAME)
DIST_SUBDIRS = $(GENERIC_LIBRARY_NAME)

modesdir=$(prefix)/share/apertium/modes

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = apertium.pc

aclocaldir = $(datadir)/aclocal
aclocal_DATA = apertium.m4

EXTRA_DIST=autogen.sh README-MODES

install-data-local:
	mkdir -p $(DESTDIR)$(modesdir)
	$(INSTALL_DATA) README-MODES  $(DESTDIR)$(modesdir)/README
