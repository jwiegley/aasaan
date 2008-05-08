import sys

from arabic import *

flags = {TF_NO_FLAGS:       "TF_NO_FLAGS",
         TF_CONSONANT:      "TF_CONSONANT",
         TF_CARRIER:        "TF_CARRIER",
         TF_VOWEL:          "TF_VOWEL",
         TF_DIPHTHONG:      "TF_DIPHTHONG",
         TF_SILENT:         "TF_SILENT",
         TF_FATHA:          "TF_FATHA",
         TF_KASRA:          "TF_KASRA",
         TF_DHAMMA:         "TF_DHAMMA",
         TF_DEFECTIVE_ALIF: "TF_DEFECTIVE_ALIF",
         TF_EXPLICIT:       "TF_EXPLICIT",
         TF_TANWEEN:        "TF_TANWEEN",
         TF_TANWEEN_ALIF:   "TF_TANWEEN_ALIF",
         TF_SILENT_ALIF:    "TF_SILENT_ALIF",
         TF_IZAAFIH:        "TF_IZAAFIH",
         TF_SUN_LETTER:     "TF_SUN_LETTER",
         TF_SHADDA:         "TF_SHADDA",
         TF_WITH_HAMZA:     "TF_WITH_HAMZA",
         TF_CAPITALIZE:     "TF_CAPITALIZE",
         TF_BAA_KULAA:      "TF_BAA_KULAA"}

display = False 
if len (sys.argv) > 1:
    word = sys.argv[1]
    display = True
else:
    word = "A/ 'amruN 'ibiluN 'u.htuN ra'suN 'ar'asu sa'ala qara'a bu'suN 'ab'usuN ra'ufa ru'asaa'u bi'ruN 'as'ilaTuN ka'iba qaa'imuN ri'aasaTuN su'ila samaa'uN barii'uN suu'uN bad'uN shay'uN shay'iN shay'aN saa'ala mas'alaTuN saw'aTuN kha.tii'aTuN /A"

elements = parse (word, style.AASAAN, mode.PERSIAN)
if display:
    print "aasaan input:", word
    for elem in elements:
	print "%10s " % elem.token,

	for flag in flags.keys ():
	    if elem.flags & flag:
		print flags[flag],

	print

    print "output:"

print """<?xml version="1.0" encoding="UTF-8"?>
<opml version="1.0">
  <head>
    <title>test</title>
    <expansionState>0</expansionState>
  </head>
  <body>"""

print "<outline text=\"%s\"/>" % (render (elements, style.UNICODE, mode.PERSIAN))

print """ </body>
</opml>"""

#print render (elements, style.HTML_HOUSE, mode.PERSIAN)
#print render (elements, style.ARABTEX, mode.PERSIAN)
