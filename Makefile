CFLAGS = -g -DSTANDALONE

all: arabic arabic.so

arabic: arabic.cpp
	g++ $(CFLAGS) -o $@ $<

arabic.so: arabic.cpp
	python setup.py build_ext
	cp build/lib*/arabic.so $@

dist: asbaab.zip

asbaab.zip:
	zip -r asbaab.zip setup.py \
		Makefile arabic.cpp arabic.h \
		chaap.py \
		haftvadi.xml \
		$(HOME)/doc/books/steingass.txt
