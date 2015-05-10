#!/usr/bin/awk -f

# Awk script for parsing test protocols that is based on TAP
#
# The implementation uses four arrays:
#   planned - array of test ids that have been declared with a "x..y" plan
#   passed - array of test ids that passed
#   failed - array of test ids that failed
#   skipped - array of test ids that were skipped
#
# These output specifiers can be added when calling the program
#   verbose - Print test and description on separate lines
#   color - Add terminal color escape codes
#   info - Print additional information to output, such as skipped tests

# Functions to add terminal color escape codes surrounding strings
# The global variable "color" needs to have a truthy value
function tcol(str, c)	{ return color ? c str COL_NORMAL : str }
function okcol(str)	{ return tcol(str, COL_OK) }
function warncol(str)	{ return tcol(str, COL_WARN) }
function failcol(str)	{ return tcol(str, COL_FAIL) }
function noticecol(str)	{ return tcol(str, COL_NOTICE) }

# Directive parsing
function is_skip(directive) { return directive ~ /^[Ss][Kk][Ii][Pp]/ }
function is_todo(directive) { return directive ~ /^[Tt][Oo][Dd][Oo]/ }

function is_defined(id) {
	return id in passed || id in failed || id in skipped
}

function print_test(label, id, text, is_first) {
	if (is_first) printf "%s", label

	if (verbose) printf "\n\t%d %s", id, text[id]
	else printf "%s %d", (is_first ? "" : ","), id

	system("") # Flush output buffer
}

function print_table(msg, table, text, first) {
	first = 1
	for (id in table) {
		print_test(msg, id, text, first)
		first = 0
	}
	printf "\n"
}

BEGIN {
	# Id counter, if not in feed
	counter = 1

	# Highlight colors, when enabled
	COL_NORMAL="\033[1;000m"	# Default
	COL_OK="\033[0;32m"		# Green
	COL_NOTICE="\033[1;34m"		# Blue
	COL_WARN="\033[1;33m"		# Yellow
	COL_FAIL="\033[1;31m"		# Red

	# Only use tab as field separator
	FS="\t"
}

# Plan record, $2=from, $3=to, $4=directive (there can be more than one)
$1 == "plan" {
	if ($2 == 1 && $3 == 0 && is_skip($4)) {
		# Special case for skipping the whole test
		skip_all = 1
	} else for (i = $2; i <= $3; i++) {
		# Declare new test
		planned[i] = 1

		if ($4 ~ /^[Ss][Kk][Ii][Pp]/) {
			# Skip range of tests
			if (is_defined(i)) {
				# TODO: Test already defined, signal warning
			} else {
				skipped[i] = 1
				message[i] = $5
				explain[i] = $5
			}
		}
	}
}

# Test line, $1=status, $2=id, $3=description, $4=directive, $5=explanation
$1 ~ /(not )?ok/ {
	id = $2

	# Make sure the test has an id
	if (!id) id = counter++
	else counter = id + 1

	# Have we seen this test result already somewhere?
	if (is_defined(id)) {
		repeat[id]++
		repeat_text[id] = id " repeated " repeat[id] " times"
	}

	# The test has passed
	message[id] = $3
	explain[id] = $5

	if (is_todo($4)) {
		# TODO directive. These tests count as passed no matter what
		passed[id] = 1

		# If it was successful, then notify user at the end
		if (!fail) done[id] = 1
	} else if (is_skip($4)) {
		# SKIP directive
		skipped[id] = 1
	} else if ($1 ~ /^not/) {
		# Test failed
		failed[id] = 1
		print_test(failcol("FAILED"), id, message, length(failed) == 1)
	} else {
		passed[id] = 1
	}
}

/Bail out!/ {
	# Bail out
	print
	exit
}

/^#/ {
	# Diagnostic
}

END {
	# Mark planned tests that failed to run as failed
	for (id in planned) {
		if (id in passed || id in skipped || id in failed) continue;
		failed[id] = 1
		print_test(failcol("FAILED"), id, message, length(failed) == 1)
	}

	# End line of failures
	nfail = length(failed)
	if (nfail > 0) {
		printf "\n"
	}

	# Check whether there was a plan at all
	if (length(planned) == 0 && !skip_all) {
		print failcol("NO PLAN")
	}

	if (info) {
		# Check special cases
		if (length(repeat)) {
			print_table(failcol("Repeated"), repeat, repeat_text);
		}
		if (length(skipped)) {
			print_table(noticecol("Skipped"), skipped, explain);
		}
		if (length(done)) {
			print_table(warncol("Passed"), done, explain);
		}
	}

	# Were there any unplanned tests?
	first = 1
	for (id in passed) {
		if (!(id in planned)) {
			print_test(warncol("Not planned"), id, message, first)
			first = 0
		}
	}
	if (!first) printf "\n"

	# Print test summary
	npass = length(passed) + length(skipped)
	ndone = length(done) # Not exclusive list
	n = npass + nfail
	if (n > 0) {
		if (nfail) {
			percent = npass*100.0/n
			printf "Failed %d/%d tests\n", nfail, n
		} else {
			print okcol("OK")
		}
	} else if (skip_all) {
		print okcol("OK")
	} else {
		print warncol("NO TESTS")
	}
}

