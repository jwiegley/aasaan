#!/usr/bin/python

# chaap (print), version 3.0
#   by John Wiegley <johnw@gnu.org>
#
# Use "python chaap.py --help" for help on script usage and options.
#
# This script takes an input file of the XML style "Tablet".

import sys
import xml.parsers.expat
import types
import re

from arabic import *
from getopt import getopt

true, false = 1, 0

translit     = None
latex 	     = None
arabic 	     = None
novocalize   = None
fullvocalize = None
original     = None
original_wp  = None
english      = None
filter       = None
help 	     = None

def usage():
    print """usage: python chaap.py [options] <input text.xml>

options:
  --original      output only original text; default is bilingual
  --original-wp   output original, with paragraph separation
  --english       output only english text (always with paragraphs)
  --filter        act only as a filter to convert transliterated text

  --arabic        default to Arabic conventions, not Persian
  --latex         output code for LaTeX2e; default is HTML 4
  --novocalize    do not output vowellization marks
  --fullvocalize  output sukun, in addition to other diacriticals
  --translit      print transliterated text; default is to print
                  using Arabic script"""
    sys.exit(0)

long_opts = [
    'translit', 'latex', 'arabic', 'novocalize', 'fullvocalize',
    'original', 'original-wp', 'english', 'filter', 'help'
]

opts, args = getopt(sys.argv[1:], [], long_opts)

for opt in opts:
    if opt[0] == "--help":
        usage()

    for lopt in long_opts:
        if opt[0] == "--" + lopt:
            exec("%s = true" % lopt)

if novocalize:
    vocal = "novocalize"
elif fullvocalize:
    vocal = "fullvocalize"
else:
    vocal = "vocalize"

if arabic:
    lang = "arab"
    default_style = ARABIC
else:
    lang = "farsi"
    default_style = PERSIAN

arabic_enter = " \\\\setarab \\\\newtanwin \\\\fullvocalize "
arabic_exit  = " \\\\setfarsi \\\\newtanwin \\\\%s " % vocal

if translit:
    output_form = LATEX_HOUSE
else:
    output_form = ARABTEX

class Tablet:
    Parser = ""
    stack  = ""

    # prepare for parsing

    def __init__(self):
        self.Parser    = xml.parsers.expat.ParserCreate()
        self.stack     = []
        self.data      = ""
        self.last_lang = default_style

        self.Parser.CharacterDataHandler = self.handleCharData
        self.Parser.StartElementHandler = self.handleStartElement
        self.Parser.EndElementHandler = self.handleEndElement

    # parse the XML file
  
    def parse(self, xml_file):
        self.Parser.ParseFile(open(xml_file, "r"))

    def handleCharData(self, data):
        data = re.sub ("^\s+", "", data)
        data = re.sub ("\s+$", "", data)
        if data:
            if self.stack[-1] == "pers" and "eng" in self.stack:
                data = convert(data, AASAAN, LATEX_HOUSE, PERSIAN)
            if self.data and re.match("[A-Za-z0-9]", self.data[-1]):
                self.data += " "
            self.data = self.data + data

    def handleStartElement(self, name, attrs):
        meth = None
        try: meth = getattr (self, "start_" + name)
        except AttributeError: pass
        self.stack.append(name)
        if type (meth) is types.MethodType:
            meth(attrs)

    def handleEndElement(self, name):
        meth = None
        try: meth = getattr (self, "end_" + name)
        except AttributeError: pass
        if type (meth) is types.MethodType:
            meth()
        else:
            self.data = ""
        self.stack.pop ()

class LatexTablet(Tablet):
    path = None
    
    def start_tablet(self, attrs):
        print """
\\documentclass[12pt]{article}
\\usepackage{arabtex}
\\usepackage{twoblks}
\\usepackage[margin=1in,nohead,nofoot]{geometry}

\\pagestyle{plain}

\\begin{document}
"""
        if not translit:
            print """

\\setlength{\\parskip}{1\\baselineskip}

\\set%s
\\%s
\\newarabfont
""" % (lang, vocal)

        if (original or original_wp) and not english:
            print "\\begin{RLtext}"

    def end_tablet(self):
        if (original or original_wp) and not english:
            print "\\end{RLtext}"

        print "\\end{document}"

    def start_text (self, attrs):
        print "\\vspace{2ex}"

    def start_arab (self, attrs):
        if default_style != ARABIC:
            self.data += " A/ "
    def end_arab (self):
        if default_style != ARABIC:
            self.data += " /A "
        self.last_lang = ARABIC

    def start_pers (self, attrs):
        if "eng" in self.stack:
            self.data += " "
        elif default_style != PERSIAN:
            self.data += " P/ "
    def end_pers (self):
        if "eng" in self.stack:
            self.data += " "
        elif default_style != PERSIAN:
            self.data += " /P "
        self.last_lang = PERSIAN

    def end_eng (self):
        self.last_lang = -1

    def start_quote (self, attrs):
        self.data += " ``"
    def end_quote (self):
        self.data += "'' "
    def end_emdash (self): self.data += "---"
    def end_line (self): pass

    def end_orig (self):
        print "\\twoblocks{\n\\begin{RLtext}"
        if self.data:
            if self.last_lang == -1:
                style = default_style
            else:
                style = self.last_lang
            text = convert(self.data, AASAAN, output_form, style)
            self.data = ""
            text = re.sub ("A/", arabic_enter, text)
            text = re.sub ("/A", arabic_exit, text)
            print text
        print "\n\\end{RLtext}}",

    def end_trans (self):
        print "{"
        print self.data
        self.data = ""
        print "}"

tablet = LatexTablet()
tablet.parse(args[0])
