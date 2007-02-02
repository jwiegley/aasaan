#!/usr/bin/python

# chaap (print), version 2.1
#   by John Wiegley <johnw@gnu.org>
#
# Use "perl chaap --help" for help on script usage and options.
#
# This script takes an input file either of all original text
# paragraphs in Aasaan format, or of alternating original/translation
# paragraphs.  The rules governing the input format are as follows:
#
# 1. Alternate verses of the original language (encoded in *aasaan*
#    style), with translation.  Or, input only original verses.
#
# 2. At least one line of each original verse/paragraph must occur in
#    column one (that is, have no preceding whitespace).
#
# 3. Conversely, every line of the translation must begin with
#    whitespace.
#
# 4. The original verse and the translation should be separated by a
#    blank line.
#
# 5. Separate these verse/translation pairs with a blank line, except
#    when they occur in different paragraphs, when they should be
#    separated with two blank lines.
#
# 6. Intra-verse whitespace is allowed, for the sake of readability.
#
# 7. For poetic verse, precede the text with a greater-than sign,
#    followed by at least one space.
#
# 8. For Persian/Arabic poetry, if you wish each line to be stretched
#    to the same width, specific the width before the greater-than
#    character.

import sys
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
    print """usage: chaap [options] <input text>

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

if translit:
    output_form = LATEX_HOUSE
else:
    output_form = ARABTEX

class Break: None

class Verse:
    text = None
    def __init__(self, str):
        self.text = str

class Original(Verse): pass
class Translation(Verse): pass

class VersePair:
    original = None
    translation = None

    def __init__(self, org, trans):
        self.original    = org
        self.translation = trans

def single_read(file):
    largest_indent = 0
    line = None
    text = ""

    line = file.readline()
    while line:
        if re.match("\s*$", line):
            break
        leading = re.match("\s+", line)
        if leading and len(leading.group(0)) > largest_indent:
            largest_indent = len(leading.group(0))
        text += line
        line = file.readline()

    if len(text) == 0:
        if not line:
            return None
        else:
            return Break()

    if largest_indent == 0:
        return Original(text)
    else:
        return Translation(text)

def read(file):
    tok = single_read(file)
    if isinstance(tok, Original):
        pos = file.tell()
        tok2 = single_read(file)
        if isinstance(tok2, Translation):
            return VersePair(tok, tok2)
        file.seek(pos)
    return tok

class Document: pass

class LatexDocument(Document):
    path = None
    
    def __init__(self, _out):
        self.out = _out

    def write(self, text = ""):
        self.out.write(text)
        self.out.write("\n")

    def prologue(self):
        self.write("""
\\documentclass[12pt]{article}
\\usepackage{arabtex}
\\usepackage{twoblks}
\\usepackage[margin=1in,nohead,nofoot]{geometry}

\\pagestyle{plain}

\\begin{document}
""")
        if not translit:
            self.write("""

\\setlength{\\parskip}{1\\baselineskip}

\\set%s
\\%s
\\newarabfont
""" % (lang, vocal))

        if (original or original_wp) and not english:
            self.write("\\begin{RLtext}")

    def output(self, verse):
        if isinstance(verse, Break):
            self.write()
        elif isinstance(verse, VersePair):
            text = verse.original.text
            text = re.sub('([0-9.]+in)?>\s+', '', text)
            text = convert(text, AASAAN, output_form, default_style)

            if not translit:
                text = """\\begin{RLtext}
%s
\\end{RLtext}""" % text

            trans = verse.translation.text
            result = ""
            last_group = None

            for group in re.finditer("<<([^>]+)>>", trans):
                if last_group:
                    result += trans[last_group.end() : group.start()]
                else:
                    result = trans[: group.start()]

                result += convert(trans[group.start() + 2 : group.end() - 2],
                                  AASAAN, LATEX_HOUSE, default_style)
                last_group = group

            if last_group:
                result += trans[last_group.end() :]
                trans = result
            trans = re.sub('([0-9.]+in)?>\s+', '', trans)

            self.write("""\\vspace{2ex}
\\twoblocks{
%s}{
%s
}
""" % (trans, text))

    def epilogue(self):
        if (original or original_wp) and not english:
            self.write("\\end{RLtext}")

        self.write("\\end{document}")

file = open(args[0])

try:
    if filter:
        conv = false
        for line in file:
            if not latex or (conv and not re.match("(\\\\|})", line)):
                sys.stdout.write(convert(line, AASAAN, output_form,
                                         default_style))
                sys.stdout.write("\n")
            else:
                sys.stdout.write(line)

            if re.match("\\\\begin{(arabtext|RLtext)}", line):
                conv = true
            elif re.match("\\\\end{(arabtext|RLtext)}", line):
                conv = false
    else:
        doc = LatexDocument(sys.stdout)
        doc.prologue()
        verse = read(file)
        while verse:
            doc.output(verse)
            verse = read(file)
        doc.epilogue()
finally:
    file.close()
