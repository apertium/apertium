SWIG_INTERFACE = apertium_core.i

BUILT_SOURCES = %_wrap.cpp

%_wrap.cpp: $(SWIG_INTERFACE) setup.py
	$(PYTHON) setup.py build

install-exec-local:
	$(PYTHON) setup.py install --prefix=$(DESTDIR)$(prefix)
