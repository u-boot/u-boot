#! /usr/bin/python
########################################################################
#
# reorder and reformat a file in columns
#
# this utility takes lines from its standard input and reproduces them,
# partially reordered and reformatted, on its standard output.
#
# It has the same effect as a 'sort | column -t', with the exception
# that empty lines, as well as lines which start with a '#' sign, are
# not affected, i.e. they keep their position and formatting, and act
# as separators, i.e. the parts before and after them are each sorted
# separately (but overall field widths are computed across the whole
# input).
#
# Options:
#   -i:
#   --ignore-case:
#	Do not consider case when sorting.
#   -d:
#   --default:
#	What to chage empty fields to.
#    -s <N>:
#    --split=<N>:
#       Treat only the first N whitespace sequences as separators.
#       line content after the Nth separator will count as only one
#       field even if it contains whitespace.
#       Example : '-s 2' causes input 'a b c d e' to be split into
#       three fields, 'a', 'b', and 'c d e'.
#
# boards.cfg requires -ids 6.
#
########################################################################

import sys, getopt, locale

# ensure we sort using the C locale.

locale.setlocale(locale.LC_ALL, 'C')

# check options

maxsplit = 0
ignore_case = 0
default_field =''

try:
	opts, args = getopt.getopt(sys.argv[1:], "id:s:",
		["ignore-case","default","split="])
except getopt.GetoptError as err:
	print str(err) # will print something like "option -a not recognized"
        sys.exit(2)

for o, a in opts:
	if o in ("-s", "--split"):
		maxsplit = eval(a)
	elif o in ("-i", "--ignore-case"):
		ignore_case = 1
	elif o in ("-d", "--default"):
		default_field = a
	else:
		assert False, "unhandled option"

# collect all lines from standard input and, for the ones which must be
# reformatted and sorted, count their fields and compute each field's
# maximum size

input_lines = []
field_width = []

for line in sys.stdin:
	# remove final end of line
	input_line = line.strip('\n')
	if (len(input_line)>0) and (input_line[0] != '#'):
		# sortable line: split into fields
		fields = input_line.split(None,maxsplit)
		# if there are new fields, top up field_widths
		for f in range(len(field_width), len(fields)):
			field_width.append(0)
		# compute the maximum witdh of each field
		for f in range(len(fields)):
			field_width[f] = max(field_width[f],len(fields[f]))
	# collect the line for next stage
	input_lines.append(input_line)

# run through collected input lines, collect the ones which must be
# reformatted and sorted, and whenever a non-reformattable, non-sortable
# line is met, sort the collected lines before it and append them to the
# output lines, then add the non-sortable line too.

output_lines = []
sortable_lines = []
for input_line in input_lines:
	if (len(input_line)>0) and (input_line[0] != '#'):
		# this line should be reformatted and sorted
		input_fields = input_line.split(None,maxsplit)
		output_fields = [];
		# reformat each field to this field's column width
		for f in range(len(input_fields)):
			output_field = input_fields[f];
			output_fields.append(output_field.ljust(field_width[f]))
		# any missing field is set to default if it exists
		if default_field != '':
			for f in range(len(input_fields),len(field_width)):
				output_fields.append(default_field.ljust(field_width[f]))
		# join fields using two spaces, like column -t would
		output_line = '  '.join(output_fields);
		# collect line for later
		sortable_lines.append(output_line)
	else:
		# this line is non-sortable
		# sort collected sortable lines
		if ignore_case!=0:
			sortable_lines.sort(key=lambda x: str.lower(locale.strxfrm(x)))
		else:
			sortable_lines.sort(key=lambda x: locale.strxfrm(x))
		# append sortable lines to the final output
		output_lines.extend(sortable_lines)
		sortable_lines = []
		# append non-sortable line to the final output
		output_lines.append(input_line)
# maybe we had sortable lines pending, so append them to the final output
if ignore_case!=0:
	sortable_lines.sort(key=lambda x: str.lower(locale.strxfrm(x)))
else:
	sortable_lines.sort(key=lambda x: locale.strxfrm(x))
output_lines.extend(sortable_lines)

# run through output lines and print them, except rightmost whitespace

for output_line in output_lines:
	print output_line.rstrip()
