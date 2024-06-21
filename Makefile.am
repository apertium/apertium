ACLOCAL_AMFLAGS=-I m4

SUBDIRS = $(PACKAGE_NAME) scripts tests locales
DIST_SUBDIRS = $(PACKAGE_NAME) scripts tests

modesdir=$(prefix)/share/apertium/modes

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = apertium.pc

aclocaldir = $(datadir)/aclocal
aclocal_DATA = apertium.m4

if HAVE_PYTHON_BINDINGS
SUBDIRS += python
endif

EXTRA_DIST=autogen.sh README-MODES apertium.m4 tests

install-data-local:
	mkdir -p $(DESTDIR)$(modesdir)
	$(INSTALL_DATA) README-MODES  $(DESTDIR)$(modesdir)/README

# just a synonym:
test: check


export ALT_I18N_DATA=$(datadir)/lttoolbox/lttoolbox.dat
export LOCALES_DIR=$(datadir)/$(PACKAGE_NAME)
export APR_I18N_DATA=$(LOCALES_DIR)/apertium.dat
