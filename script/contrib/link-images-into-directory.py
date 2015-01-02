import sys
import os
import math

# A really dumb helper script for producing a directory of images for handing
# out to ordinary people not using KPA at all
#
# Select some images in KPA, use Edit -> Copy. Launch your favorite text editor,
# paste data, trim the "file://" scheme in the contents from the beginning of
# every line. Run this script with your new file as an argument and redirect the
# output somewhere. Check the output and interpret it with bash.
#
# This is rather hackish approach to the problem, but works surprisingly well if
# you just need a quick & dirty fix. Its handling of shell quotes isa unsafe and
# will break, though. A nice KIPI plugin should be produced, imho...
#
# Hacked by Jan Kundrat, <jkt@flaska.net>

counter = 0
all_files = file(sys.argv[1]).readlines()
digits = int(math.ceil(math.log10(len(all_files))))
format = "%%0%dd" % digits

order = {}

for fname in all_files:
    fname = fname.strip()
    if fname.startswith('file://'):
        fname = fname[len('file://'):]
    basename = os.path.basename(fname)
    dirname = os.path.dirname(fname).split(os.path.sep)[-1]
    order[fname] = counter
    print ('ln "%%s" "%s_%%s_%%s"' % format) % (fname, counter, dirname, basename)

    counter += 1
