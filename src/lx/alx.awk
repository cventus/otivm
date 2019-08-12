#!/usr/bin/awk -f

BEGIN {
	if (!NAME) NAME = "state"
	if (!CELL_SIZE) CELL_SIZE = 4

	tagof["list"] = "lx_list_tag"
	tagof["tree"] = "lx_tree_tag"
	tagof["bool"] = "lx_bool_tag"
	tagof["int"] = "lx_int_tag"
	tagof["float"] = "lx_float_tag"
	tagof["string"] = "lx_string_tag"

	cells = 0
	entries = 0
	span_offset = 0
}

function islocal(symbol) {
	return symbol ~ /^\./
}

function make_ref(src_entry, target_entry,           sc, tc, to, span_offset) {
	to = offset[target_entry]

	sc = cell[src_entry]
	tc = cell[target_entry]
	if (struct[target_entry] != "raw") tc -= to + 1
	span_offset = tc - sc

	return CELL_SIZE*span_offset + to
}

function or_else(val, default) {
	return length(val) ? val : default
}

function output_data(entry,                                val, t, pre, post) {
	t = type[entry]
	val = value[entry]
	pre = ".i = (lxint)"
	post = ""
	if (t == "list" || t == "tree" || t == "string") {
		if (!(val in symbol_entry) && val != "nil") {
			printf "Undefined symbol: %s\n", val >"/dev/stderr"
			failure = 1
			exit 1
		}
		val = (val == "nil") ? 0 : make_ref(entry, symbol_entry[val])
	} else if (t == "bool") {
		val = int(val) ? 1 : 0
	} else if (t == "float") {
		pre = ".f = (lxfloat)"
	} else if (t == "char" || t == "byte") {
		pre = "{ "
		post = " }"
	}
	printf "\t{ %s%s%s }", pre, val, post
}

function output_span(span,                      term, entry, i, raw, estruct) {
	# tag cell
	raw = 0
	printf "\t{\n\t\t{\n"
	for (i = 0; i < CELL_SIZE; i++) {
		entry = span + i
		if (raw || struct[entry] == "raw") {
			# Pad with integers
			estruct = 1
			tag = tagof["int"]
			raw = 1
		} else {
			estruct = or_else(struct[entry], 1)
			typ = or_else(type[entry], "int")
			tag = tagof[typ]
		}
		term = i + 1 < CELL_SIZE ? ",\n" : "\n\t\t}\n"
		printf "\t\t\tmktag(%d, %s)%s", estruct, tag, term
	}
	printf "\t}"

	# data cells
	for (i = 0; i < CELL_SIZE; i++) {
		entry = span + i
		if (struct[entry] == "raw" || entry >= entries) break
		printf ",\n"
		output_data(entry)
	}
	return i
}

function add_entry(str, typ, val)
{
	struct[entries] = str
	type[entries] = typ
	value[entries] = val
	if (str == "raw") {
		offset[entries] = span_offset = 0
		current_span = cells + 1
	} else {
		offset[entries] = span_offset++ % CELL_SIZE
		if (offset[entries] == 0) {
			current_span = cells++
		}
		span[entries] = current_span
	}
	cell[entries] = cells++
	entries++
}

function check_type(t)
{
	if (!(t in tagof)) {
		printf "Illegal element type on line %d: %s\n", \
			NR, t >"/dev/stderr"
		failure = 1
		exit 1
	}
}

# Remove lines comments
$1 ~ /^#/ { next }

# Record raw/structure data
{
	# Check for leading symbol
	if ($1 ~ /:$/) {
		symbol = substr($1, 1, length($1) - 1)
		if (symbol in symbol_entry) {
			printf "Redefined symbol: %s\n", symbol >"/dev/stderr"
			failure = 1
			exit 1
		}
		symbol_entry[symbol] = entries
		symbol_name[symbols++] = symbol
		sub("^[^:]*:", "")
	}
}

# Skip blank lines
/^[[:space:]]*$/ { next }

{
	if ($1 == "char") {
		n = split($0, chars, "")
		chunk = ""
		sep = ""
		count = 0
		for (i = index($0, "\"") + 1; i <= n; i++) {
			if (chars[i] == "\"") break
			ch = chars[i]
			if (chars[i] == "\\") {
				i++
				ch = ch chars[i]
			}
			chunk = sprintf("%s%s'%s'", chunk, sep, ch)
			sep = ", "
			if (++count == CELL_SIZE) {
				add_entry("raw", "char", chunk)
				count = 0
				chunk = ""
				sep = ""
			}
		}
		for (; count < CELL_SIZE; count++) {
			chunk = sprintf("%s%s'\\0'", chunk, sep)
			sep = ", "
		}
		add_entry("raw", "char", chunk)
	} else if ($1 == "byte") {
		count = 0
		chunk = ""
		for (i = 2; i <= NF; i++) {
			if (count > 0) chunk = chunk ", "
			chunk = chunk $i
			if (++count == CELL_SIZE) {
				add_entry("raw", "byte", chunk)
				count = 0
				chunk = ""
			}
		}
		if (count) add_entry("raw", "byte", chunk)
	} else if ($1 ~ /[0-9]{1,}/) {
		check_type($2)
		add_entry($1, $2, $3)
	} else {
		check_type($1)
		add_entry("raw", $1, $2)
	}
}

END {
	if (failure) exit 1

	# Output array
	printf "static union lxcell const %s[] = {", NAME
	for (i = 0; i < entries; ) {
		printf (i > 0 ? ",\n" : "\n")
		if (struct[i] == "raw") {
			output_data(i)
			i++
		} else {
			i += output_span(i)
		}
	}
	printf "\n};\n\n"

	# Output references
	for (i = 0; i < symbols; i++) {
		symbol = symbol_name[i]
		if (islocal(symbol)) continue
		entry = symbol_entry[symbol]
		c = (entry in span) ? span[entry] : cell[entry]
		printf "#define %s_cell %d\n", symbol, c
		if (struct[entry] == "raw") continue
		printf "#define %s_offset %d\n", symbol, offset[entry]
	}
}
