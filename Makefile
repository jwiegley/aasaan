arabic.so: arabic.cpp
	python setup.py build_ext
	cp build/lib*/arabic.so $@
