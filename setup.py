#!/usr/bin/env python

from distutils.core import setup, Extension

libs = [ "boost_python" ]

setup(name         = "Arabic",
      version      = "1.0",
      description  = "Arabic Transliteration Tool",
      author       = "John Wiegley",
      author_email = "johnw@newartisans.com",
      url          = "http://johnwiegley.com/",
      ext_modules  = [
    Extension("arabic", ["arabic.cc"],
              define_macros = [('PYTHON_MODULE', 1), ('USE_BOOST_PYTHON', 1)],
              libraries     = libs)])
