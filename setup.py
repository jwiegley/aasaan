#!/usr/bin/env python

from distutils.core import setup, Extension

setup(name         = "Arabic",
      version      = "1.0",
      description  = "Arabic Transliteration Tool",
      author       = "John Wiegley",
      author_email = "johnw@newartisans.com",
      url          = "http://www.newartisans.com/johnw/",
      ext_modules  = [
    Extension("arabic", ["arabic.cpp"],
              define_macros = [('PYTHON_MODULE', None)])])
