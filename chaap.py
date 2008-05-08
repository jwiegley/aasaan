#!/usr/bin/python

# chaap (print), version 3.0
#   by John Wiegley <johnw@gnu.org>
#
# Use "python chaap.py --help" for help on script usage and options.

import sys
import re
import string
import os

from arabic import *
from getopt import getopt

true, false = 1, 0

translit     = None
arabic 	     = None
arabtex      = None
novocalize   = None
fullvocalize = None
help 	     = None

def usage():
    print """usage: python chaap.py [options] <files...>

options:
  --arabic        default to Arabic conventions, not Persian
  --arabtex       output code for LaTeX2e; default is HTML 4
  --novocalize    do not output vowellization marks
  --fullvocalize  output sukun, in addition to other diacriticals
  --translit      print transliterated text; default is to print
                  using Arabic script"""
    sys.exit(0)

long_opts = [
    'help'
    'arabic', 'arabtex', 'novocalize', 'fullvocalize', 'translit'
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
    default_mode = mode.ARABIC
else:
    lang = "farsi"
    default_mode = mode.PERSIAN

if translit:
    output_style = style.LATEX_HOUSE
else:
    output_style = style.UNICODE

if len (args) > 1:
    original    = args[0]
    translation = args[1]
elif re.search ("\\.eng$", args[0]):
    original    = None
    translation = args[0]
else:
    original    = args[0]
    translation = None

def render_house (line):
    match = re.search ("<<(.+?)>>", line)
    while match:
        elements = parse (match.group (1), style.AASAAN, mode.PERSIAN)
        text = render (elements, style.HTML_HOUSE, mode.PERSIAN)
        line = (line[: match.start ()] + text + line[match.end () :])
        match = re.search ("<<(.+?)>>", line)
    return line

orig_data = original    and open(original, "r")
xlat_data = translation and open(translation, "r")

print """<?xml version="1.0" encoding="UTF-8"?>
<opml version="1.0">
  <head>
    <title>test</title>
    <expansionState>0</expansionState>
  </head>
  <body>
    <outline>"""

for line in orig_data or xlat_data:
    if line == "\n":
        print
        continue

    if original:
        match = re.search ("{W([^}]+)}", line)
        if match:
            width = match.group (1)
        else:
            width = 0

        line = re.sub ("{[^}]+}", "", line)
        elements = parse (line, style.AASAAN, default_mode)
        text = render (elements, style.UNICODE, default_mode)
        text = re.sub("\\s+$", "", text)
        text = re.sub("^\\s+", "", text)
        print "      <outline text=\"%s\"" % text,

    if original and translation:
        tline = xlat_data.readline ()
        while tline == "\n":
            tline = xlat_data.readline ()

        tline = render_house(tline)
        print "English=\"%s\"/>" % tline[:-1]

print """</outline>
  </body>
</opml>"""
