all: arabic.so

arabic.so: arabic.cpp
	python setup.py build_ext
	cp build/lib*/arabic.so $@

dist: asbaab.zip

asbaab.zip:
	zip -r asbaab.zip setup.py \
		Makefile arabic.cpp arabic.h \
		chaap.py \
		steingass.txt
