library_includedir = $(includedir)/$(GENERIC_LIBRARY_NAME)-$(GENERIC_API_VERSION)/$(GENERIC_LIBRARY_NAME)

bin_PROGRAMS = test-find-similar-ambiguity-class
bin_SCRIPTS =  $(GENERATEDSCRIPTS)

AM_CPPFLAGS = -I$(top_srcdir)

apertiumdir = $(prefix)/share/apertium
apertiuminclude = $(prefix)/include/apertium-$(GENERIC_API_VERSION)
apertiumlib = $(prefix)/lib
apertiumsysconf = $(prefix)/etc/apertium

test_find_similar_ambiguity_class_SOURCES = test_find_similar_ambiguity_classes.cc
test_find_similar_ambiguity_class_LDADD = -L$(top_srcdir)/$(GENERIC_LIBRARY_NAME)/.libs/ $(APERTIUM_LIBS) -l$(GENERIC_LIBRARY_NAME)$(GENERIC_MAJOR_VERSION)
