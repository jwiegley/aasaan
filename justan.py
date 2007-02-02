#!/usr/bin/python

import sys
import os
import re
import gc
import string
import bisect
#import readline
import gzip
import cPickle

from wxPython import *
from arabic import *
from getopt import getopt

true, false = 1, 0

graphical = true

correspondences = [
    [ '.d', 'z', '.z', 'dh' ],
    [ '.h', 'h', 'H' ],
    [ '.s', 's', 'th' ],
    [ '.t', 't', 'T' ],
    [ 'N', 'n' ],
    [ 'Y', 'aa', '_a' ],
    [ '`', "'" ],
    [ 'ey', 'ii', 'y' ],
    [ 'gh', 'q' ],
    [ 'n', 'm' ],
    [ 'o', 'uu', 'uw', 'wa', 'v' ]
]

vowels = [ 'aa', 'ii', 'uu', '_a', 'uw', 'o', 'Y', 'ey' ]

sortorder = [
    'a/aa/Y/_a', 'b', 'p', 't/T', 'th', 'j', 'ch', '.h', 'kh', 'd',
    'dh', 's', 'sh', 'r', 'z', 'zh', '.s', '.d', '.t', '.z', '`',
    'gh', 'f', 'q', 'k', 'g', 'l', 'm', 'n/N', 'v/uu/uw/o', 'y/ii/ey',
    'H/h'
]

stop_words = [ 'az', 'bar' ]

lookup = {}

index = 1
for letter in sortorder:
    for elem in string.split(letter, "/"):
	lookup[elem] = index
    index += 1

def split_letters(word, keep_initial):
    letters = []
    i = l = let = found = pre_i = None

    i = 0
    l = len(word)
    while i < l:
	pre_i = i
	found = 0

        if i + 2 <= l:
            let = word[i : i + 2];
            if lookup.has_key(let):
		i += 2
		found = 1

        if not found:
            let = word[i : i + 1]
            i += 1

        if ((not keep_initial or pre_i > 0) and
            (let == "a" or let == "i" or let == "u")):
            continue

	letters.append(let)

    return letters

def generate_strings(letters, sofar, phonetic):
    isarray = false

    if not letters:
        return sofar

    let = letters.pop(0)
    if phonetic or let == "y" or let == "v":
	for corr in correspondences:
            for memb in corr:
                if memb == let:
                    let = corr
                    isarray = true
                    break
            if isarray:
                break

    if not isarray:
        let = [ let ]

    lets = []
    if not sofar:
	lets = let
    else:
        last_is_vowel = false
        if split_letters(sofar[0], true)[-1] in vowels:
            last_is_vowel = true

        last_l = None
        for l in let:
            lets.append(l)

            if last_is_vowel or l in vowels or \
                   (last_l and last_l in vowels):
                if l not in vowels and phonetic:
                    lets.append(l + l)
                last_l = l
                continue

            lets.append("a" + l)
            lets.append("i" + l)
            lets.append("u" + l)

            # If generating possible phonetic matches, make sure to
            # check for tashdid

            if phonetic:
                lets.append(l + l)
                lets.append("a" + l + l)
                lets.append("i" + l + l)
                lets.append("u" + l + l)

            last_l = l

    if not sofar:
	bucket = lets
    else:
        bucket = []
        for memb in sofar:
            for l in lets:
                if not (re.search('(.+)\\1', memb) and
                        re.search('(.+)\\1', l)):
                    bucket.append(memb + l)

    return generate_strings(letters, bucket, phonetic)

def similar(word, phonetic):
    return generate_strings(split_letters(word, true), [], phonetic)

def find_word(word):
    exact = approx = phonetic = false
    if re.match("=", word):
        exact = true
        word = word[1:]
    if re.match("\?", word):
        approx = true
        word = word[1:]
    if re.match("/", word):
        approx = true
        phonetic = true
        word = word[1:]

    if not approx:
        words = [ word ]
    else:
        words = similar(word, phonetic)

    seen = {}
    entries = []
    for table in (headwords, subwords):
        for word in words:
            key = hash(word)
            if table.has_key(key):
                pat = re.compile(re.escape(word))
                for entry in table[key]:
                    if not seen.has_key(entry):
                        entries.append((pat, entry))
                        seen[entry] = true
        if exact:
            break

    return entries

if graphical:
    frame = wxFrame(None, -1, "Lughat 1.0")
    dlg = wxMessageDialog(frame, "Lughat is loading, please wait...",
                          "Loading...", wxICON_INFORMATION)

if os.path.exists('steingass.dat'):
    print "Reading index, please wait..."
    #data = gzip.open('steingass.dat')
    data = open('steingass.dat')
    (headwords, subwords, pages, dictionary) = cPickle.load(data)
    data.close()
else:
    dictionary = []
    headwords  = {}
    subwords   = {}
    pages      = []

    entry = 0
    page  = 0

    print "Building index, this may take time..."
    file = open('steingass.txt')
    for line in file:
        if re.search('{P[0-9]+}', line):
            pages.append(entry)
            line = re.sub('\s*{P[0-9]+}', '', line)

        dictionary.append(line)

        for headword in re.findall('<([^>]+)>', line):
            key = hash(headword)
            if not headwords.has_key(key):
                headwords[key] = [ entry ]
            else:
                headwords[key].append(entry)

            words = map(lambda x: re.sub('(^al-|-i$)', '', x),
                        string.split(headword))
            words.sort()

            last_word = None
            for word in words:
                if word == headword or \
                   word == last_word or \
                   word in stop_words:
                    continue

                key = hash(word)
                if not subwords.has_key(key):
                    subwords[key] = [ entry ]
                else:
                    subwords[key].append(entry)

                last_word = word

        entry += 1
    file.close()

    #data = gzip.open('steingass.dat', 'wb')
    data = open('steingass.dat', 'wb')
    cPickle.dump((headwords, subwords, pages, dictionary), data, true)
    data.close()

if graphical:
    dlg.Destroy()

if not graphical:
    sys.stdout.write("> ")
    line = sys.stdin.readline()
    while line:
        word = line[:-1]
        if word == "quit" or word == "exit":
            break
        else:
            entries = find_word(word)
            if not entries:
                sys.stdout.write("Cannot find a definition for '%s'" % word)
            else:
                for pat, entry in entries:
                    definition = dictionary[entry][:-1]
                    if pat.search(definition):
                        page = bisect.bisect(pages, entry)
                        sys.stdout.write(definition + " [%d]\n" % page)

        sys.stdout.write("> ")
        gc.collect()
        line = sys.stdin.readline()

    sys.exit(0)
else:
    from wxPython.wx import *
    from wxPython.html import *

    #if not wxUSE_UNICODE:
    #    print "Sorry, wxPython was not built with Unicode support.",
    #    sys.exit(1)

    ID_HELP  = 101
    ID_ABOUT = 102
    ID_EXIT  = 103

    class MyHtmlWindow(wxHtmlWindow):
        def __init__(self, parent, id):
            wxHtmlWindow.__init__(self, parent, id)

        def OnLinkClicked(self, linkinfo):
            self.base_OnLinkClicked(linkinfo)

    class MyFrame(wxFrame):
        def __init__(self, parent, id, title):
            wxFrame.__init__(self, parent, -1, title, size = (600, 400),
                             style= (wxDEFAULT_FRAME_STYLE |
                                     wxNO_FULL_REPAINT_ON_RESIZE))
            menu = wxMenu()
            menu.Append(ID_HELP, "&Help", "How to use this program")
            menu.Append(ID_ABOUT, "&About", "Information about this program")
            menu.AppendSeparator()
            menu.Append(ID_EXIT, "E&xit", "Terminate the program")

            menuBar = wxMenuBar()
            menuBar.Append(menu, "&File");

            self.SetMenuBar(menuBar)

            self.word = wxTextCtrl(self, -1)

            EVT_TEXT(self, self.word.GetId(), self.OnCharEntry)

            self.html = MyHtmlWindow(self, -1)
            self.html.SetRelatedFrame(self, "Lughat 1.0")
            self.html.SetRelatedStatusBar(0)

            self.box = wxBoxSizer(wxVERTICAL)
            subbox = wxBoxSizer(wxHORIZONTAL)
            subbox.Add(self.word, 1)

            self.box.Add(subbox, 0, wxGROW)
            self.box.Add(self.html, 1, wxGROW)

            self.SetSizer(self.box)
            self.SetAutoLayout(true)

            EVT_MENU(self, ID_HELP,  self.OnHelp)
            EVT_MENU(self, ID_ABOUT, self.OnAbout)
            EVT_MENU(self, ID_EXIT,  self.TimeToQuit)

            self.Show(true)

        def OnCharEntry(self, event):
            entries = find_word(event.GetString())
            if not entries:
                return
            
            lines = [ "<html>", ]
            for pat, entry in entries:
                definition = dictionary[entry][:-1]
                if pat.search(definition):
                    page = bisect.bisect(pages, entry)
                    text = re.sub('<', '{', definition)
                    text = re.sub('>', '}', text)
                    text = re.sub('{', '<b>', text)
                    text = re.sub('}', '</b>', text)
                    lines.append("<p>" + text + " [%d]\n" % page)
            lines.append("</p></html>")

            self.html.SetPage(string.join(lines, '\n'))

        def OnHelp(self, event):
            self.html.SetPage("""<html>

<p>To search for a word, simply type that word using the
<a href="http://www.gci-net.com/users/j/johnw/TranslationTools.html\#aasaan">Aasaan
style of transliteration</a>.  For example:</p>

<pre>`ilm</pre>

<p>This will find the definition of <b>`ilm</b>, as well as any other
definitions that make reference to <b>`ilm</b>.  To find the
definition only for the word itself, put an equal sign (<tt>=</tt>)
in front of the word:</p>

<pre>=`ilm</pre>

<p>To search approximately -- which assumes that you have the
consonants correct, but the short vowels may be wrong -- put a
question mark (<tt>?</tt>) in front of the word:</p>

<pre>?tuliffu.z</pre>

<p>This will correctly find <b>talaffu.z</b>, even though the short
vowelling is wrong.  In fact, short vowels are not necessary; you
could search for <b>tlff.z</b> as well.  (Note: The equal sign also
works with these other search forms, so to find only the definition
of <b>talaffu.z</b>, use <b>=?tlff.z</b>).</p>

<p>To search phonetically, which assumes only that you know the basic
sound of the word, use a forward slash (<tt>/</tt>).  For
example:</p>

<pre>/talafuz         # finds talaffu.z
/sadaf</pre>

<p>This second query returns the definitions of <b>.sadf</b> (turning
away), <b>.sadaf</b> (seashell), <b>.sudaf</b> (a kind of bird) and
<b>sadaf</b> (night-season).  Note that the first three of these
words are written identically, which means that searching
approximately using <b>?.sadaf</b> would also have found them.</p>

<p>NOT WORKING YET!  To see all entries on page 47 of Steingass,
use:</p>

<pre>page 47</pre>
</html>""")

        def OnAbout(self, event):
            dlg = wxMessageDialog(self,
"""Lughat: A Persian dictionary lookup program
             Version 1.0
Written by John Wiegley <johnw@gnu.org>""",
                                  "About Lughat", wxOK | wxICON_INFORMATION)
            dlg.ShowModal()
            dlg.Destroy()


        def TimeToQuit(self, event):
            self.Close(true)

    class MyApp(wxApp):
        def OnInit(self):
            frame = MyFrame(None, -1, "Lughat 1.0")
            self.SetTopWindow(frame)
            return true

    app = MyApp(0)
    app.MainLoop()
