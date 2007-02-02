import sys
import re

from arabic import *

for line in sys.stdin:
    match = re.search ("<<(.+?)>>", line)
    while match:
        elements = parse (match.group (1), style.AASAAN, mode.PERSIAN)
        text = render (elements, style.ARABTEX, mode.PERSIAN)
        line = (line[: match.start ()] +
                "<" + text + ">" +
                line[match.end () :])
        match = re.search ("<<(.+?)>>", line)
    print line,
