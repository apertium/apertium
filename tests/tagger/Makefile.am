# This is expected to be built before the parent dir, which depends on
# tagger/test-find-similar-ambiguity-class for run_tests.py

library_includedir = $(includedir)/$(PACKAGE_NAME)-$(VERSION_API)/$(PACKAGE_NAME)

check_PROGRAMS = test-find-similar-ambiguity-class
check_SCRIPTS =  $(GENERATEDSCRIPTS)

AM_CPPFLAGS = -I$(top_srcdir)

apertiumdir = $(prefix)/share/apertium
apertiuminclude = $(prefix)/include/apertium-$(VERSION_API)
apertiumlib = $(prefix)/lib
apertiumsysconf = $(prefix)/etc/apertium

test_find_similar_ambiguity_class_SOURCES = test_find_similar_ambiguity_classes.cc
test_find_similar_ambiguity_class_LDADD = -L$(top_srcdir)/$(PACKAGE_NAME)/.libs/ -l$(PACKAGE_NAME)$(VERSION_MAJOR)
