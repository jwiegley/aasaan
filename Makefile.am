#lib_LTLIBRARIES = libarabic.la
#libarabic_la_SOURCES = arabic.cc
#libarabic_la_LDFLAGS = -version-info 2:0

######################################################################

bin_PROGRAMS = arabic
arabic_CXXFLAGS = -DSTANDALONE
if HAVE_BOOST_PYTHON
arabic_CXXFLAGS += -DUSE_BOOST_PYTHON=1
endif
arabic_SOURCES = arabic.cc

######################################################################

if HAVE_BOOST_PYTHON

bin_PROGRAMS += arabic.so

arabic.so: arabic.cc
	CFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS) -L. -L.libs" \
	    python setup.py build --build-lib=.

install-exec-hook:
	CFLAGS="$(CPPFLAGS)" LDFLAGS="$(LDFLAGS) -L. -L.libs" \
	    python setup.py install --prefix=$(DESTDIR)

endif

aasaan.pdf: aasaan.tex
	cat aasaan.tex | python convert.py > temp.tex
	pdflatex temp.tex
	pdflatex temp.tex
	pdflatex temp.tex
	mv temp.pdf $@

all-clean: maintainer-clean
	rm -f	*~	       \
		.*~	       \
		.\#*	       \
		.gdb_history   \
		gmon.out       \
		h	       \
		out
	rm -f	*.o	       \
		*.a	       \
		*.lo	       \
		*.la	       \
		*.aux	       \
		*.cp	       \
		*.elc	       \
		*.fn	       \
		*.html	       \
		*.info	       \
		*.ky	       \
		*.log	       \
		*.pdf	       \
		*.pg	       \
		*.toc	       \
		*.tp	       \
		*.vr	       \
		*.so	       \
		TAGS	       \
		ledger	       \
		valexpr
	rm -fr	.deps	       \
		.libs          \
		build          \
		AUTHORS        \
		ChangeLog      \
		COPYING        \
		INSTALL        \
		Makefile       \
		Makefile.in    \
		acconf.h       \
		acconf.h.in    \
		aclocal.m4     \
		autom4te.cache \
		config.guess   \
		config.status  \
		config.sub     \
		configure      \
		depcomp        \
		install-sh     \
		libtool        \
		ltconfig       \
		ltmain.sh      \
		missing        \
		stamp-h1       \
		texinfo.tex
