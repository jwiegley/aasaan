import sys
import re

from arabic import *

file = open(sys.argv[1], "rb")

for line in file:
    print convert(line, ARABTEX, AASAAN, PERSIAN)

file.close()
