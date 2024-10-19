# To autoload this file into GDB, put the following line in ~/.gdbinit:
#
#     python execfile("/path/to/icu_unicodestring_prettyprinter.py")
#
# You can also run that line of code in the GDB console without adding it to ~/.gdbinit.

from __future__ import print_function
from array import array
import re
import gdb

class UnicodeStringPrinter:
	"""GDB pretty printer for ICU4C UnicodeString"""

	def __init__(self, val):
		self.val = val

	def to_string(self):

		fLengthAndFlags = self.val["fUnion"]["fFields"]["fLengthAndFlags"]

		if fLengthAndFlags >= 0:
			# Short length
			length = fLengthAndFlags >> 5
		else:
			# Long length
			length = self.val["fUnion"]["fFields"]["fLength"]

		if (fLengthAndFlags & 2) != 0:
			# Stack storage
			if (fLengthAndFlags & 1) != 0:
				return u"UnicodeString (BOGUS)"
			stack = True
			buffer = self.val["fUnion"]["fStackFields"]["fBuffer"]
		else:
			# Heap storage
			stack = False
			buffer = self.val["fUnion"]["fFields"]["fArray"]

		content = array('B', [buffer[i] for i in range(length)]).tostring()
		return u"UnicodeString (%d on %s): \"%s\"" % (
			length,
			u"stack" if stack else u"heap",
			content)

unicode_string_re = re.compile("^icu_?\d*::UnicodeString$")

def lookup_type(val):
	if unicode_string_re.match(str(val.type.unqualified().strip_typedefs())):
		return UnicodeStringPrinter(val)
	return None

gdb.pretty_printers.append(lookup_type)
