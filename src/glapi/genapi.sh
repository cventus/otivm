#!/bin/sh

# Bare-bones SAX parser and header/source generator for gl.xml
#
# usage:
#  ./genapi.sh <gl.xml
# or
#  curl -s 'https://raw.githubusercontent.com/KhronosGroup/OpenGL-Registry/master/xml/gl.xml' | ./genapi.sh

xmltokens() {
  awk '
# Tokenize XML stream into line-oriented records
#
# For each element and text node, print either the opening/closing element on
# one line (including every attribute) or a single line of text for text nodes.
# Comments are removed. An output line is part of a tag if it starts with an
# opening angle bracket for tags, and is a text node otherwise. Un-escape
# XML-entities before using texts, particularly line breaks, which are encoded
# as `&#10;`.
#
# No intentional support for DTD.

function print_text(text, i) {
	if (length(text) > 0) {
		gsub(/\n/, "\\&#10;", text)
		gsub(/\r/, "\\&#13;", text)
		printf("%s\n", text)
	}
}

function print_tag(tag) {
	# Turn new-lines within tags into spaces
	gsub(/\n|\r/, " ", tag)

	# Remove leading spaces to ensure that there is no space between the
	# opening angle bracket and the tag name
	gsub(/^[[:space:]]*/, "", tag);

	# Remove excessive spaces
	gsub(/[[:space:]]+/, " ", tag);

	# Ensure there is no space before or after an equal sign among
	# the attributes
	gsub(/[[:space:]]*=[[:space:]]*/, "=", tag)

	# Output one line of tag
	printf("<%s>\n", tag);
}

BEGIN {
	# read one tag at a time
	RS = "<"
	FS = ""
	comment = 0
	text_node = ""
}
# Put every tag and all of its properties on one line

/^!--/ {
	# begin comment section
	comment = 1
}

!comment {
	# Print previously accumulated text node
	print_text(text_node)

        # Split into before tag and first text node of tag
	end = index($0, ">")
	if (end < 1) {
		# prefix of XML file or text after comment
		print_text($0)
		text_node = "";
	} else {
		tag = substr($0, 1, end - 1)
		text = substr($0, end + 1)

		print_tag(tag);
		text_node = text;
	}
}

comment && /-->/ {
	# end comment section (tag part element possible)
	end = index($0, ">")
	# concatenate text node content to avoid creating multiple text nodes
	# when there is XML comments within text
	text_node = text_node substr($0, index($0, ">") + 1)
	comment = 0
}

END {
	# trainling text node after root element
	print_text(text_node)
}
'
}

xmlawk() {
  local pat="$1"
  shift
  awk '
# Used like AWK to parse a stream of tags and text nodes produced by
# `xml-tokens`, where the `xml_path` global variable is available, as well as
# `xml_text`, `xml_start`, `xml_end`, and the `xml_attr()` function.

function xml_tagname(elem_) {
	if (match(elem_, /<\/[^ >\/]+/)) {
		return substr(elem_, RSTART + 2, RLENGTH - 2)
	} else if (match(elem_, /<[^ >\/]+/)) {
		return substr(elem_, RSTART + 1, RLENGTH - 1)
	} else {
		return ""
	}
}

function xml_unescape(line_, number_, before_, after_) {
	gsub(/&lt;/, "<", line_)
	gsub(/&gt;/, ">", line_)
	gsub(/&amp;/, "\\&", line_)
	gsub(/&apos;/, "'\''", line_)
	gsub(/&quot;/, "\"", line_)
	while (match(line_, /&#[0-9]+;/)) {
		number_ = strtonum(substr(line_, RSTART + 2, RLENGTH - 3))
		before_ = substr(line_, 1, RSTART - 1)
		after_ = substr(line_, RSTART + RLENGTH)
		line_ = sprintf("%s%c%s", before_, number_, after_)
	}
	return line_
}

# double quotes are assumed to be escaped with &quot; within attribute values
function xml_attribute(elem_, attr_, defval_, pat_, pfx_) {
	pat_ = sprintf(" %s=\"[^\"]*\"", attr_)
	if (match(elem_, pat_)) {
		pfx_ = 3 + length(attr_)
		return xml_unescape(substr(elem_, RSTART + pfx_, RLENGTH - 1 - pfx_))
	} else {
		return defval_
	}
}

function xml_attr(attr, default_value) {
	return xml_attribute($0, attr, default_value)
}

# start tag -> add to path
/^<[^\/]/ {
	if ($0 !~ /^<\?/) {
		# Don not add e.g. <?xml> to path
		xml_path = xml_path "/" xml_tagname($0)
	}
	xml_start = 1
	xml_end = $0 ~ /\/>$/ # empty?
	xml_text = 0
	xml_tag = "/" xml_tagname($0)
	# print "open", xml_tag
}

# end tag -> remove from path once processing is done
/^<\// {
	xml_start = 0
	xml_end = 1
	xml_text = 0
	xml_tag = "/" xml_tagname($0)
	# print "close", xml_tag
}

# text node
/^[^<]/ {
	xml_text = 1
	xml_start = 0
	xml_end = 0
	xml_tag = ""
}

# user patterns
'"$pat"'

# restore  
xml_end {
	if (sub(sprintf("%s$", xml_tag), "", xml_path) == 0) {
		printf("unexpected tag: <%s>\n", xml_tag) > "/dev/stderr"
		xml_error = 1
		exit 1
	}
}

END {
	if (!xml_error && xml_path != "") {
		printf("unexpected EOF at %s\n", xml_path) > "/dev/stderr"
		exit 1
	}
}
'
}

xmltokens | xmlawk '
# Parse the format of gl.xml from stdin and output one header and source files

BEGIN {
	feature_parts["type"] = 1;
	feature_parts["enum"] = 1;
	feature_parts["command"] = 1;

	# Output OpenGL API versions from 3.0 onwards
	MIN_TARGET_MAJOR_VERSION = 3
	MIN_TARGET_MINOR_VERSION = 0

	# Set these to a version to avoid removing functions
	MAX_TARGET_MAJOR_VERSION = -1
	MAX_TARGET_MINOR_VERSION = -1

	# MAX_MAJOR_VERSION = 4
	# MAX_MINOR_VERSION = 5

	delete type_before
	delete type_defined
	delete type_after
	delete type_requires

	delete enum_value
	delete cmd_names
	delete cmd_types
	delete cmd_field
	delete cmd_get_proc

	# core/obsolete by 3.0
	ext_blacklist["GL_ARB_matrix_palette"] = 1 
	ext_blacklist["GL_ARB_multitexture"] = 1 
	ext_blacklist["GL_ARB_shader_objects"] = 1 
	ext_blacklist["GL_ARB_transpose_matrix"] = 1 
	ext_blacklist["GL_ARB_vertex_blend"] = 1 
	ext_blacklist["GL_ARB_vertex_array_object"] = 1
	ext_blacklist["GL_ARB_vertex_buffer_object"] = 1
	ext_blacklist["GL_ARB_vertex_program"] = 1
	ext_blacklist["GL_ARB_vertex_shader"] = 1
	ext_blacklist["GL_ARB_window_pos"] = 1 
}

# Licence comment
#
xml_path ~ "/registry/comment$" && xml_text {
	comment = xml_unescape($0);
}

# Types
#
xml_path ~ "/registry/types/type$" && xml_start {
	# "name" is either in an attribute of the <type> or a child of the <name> element
	name = xml_attr("name", "")
	requires = xml_attr("requires")
	api = xml_attr("api", "gl")
	before = ""
	after = ""
	defined = 0
}

xml_path ~ "/registry/types/type$" && xml_text && xml_path !~ /name/ {
	if (name == "") {
		before = before xml_unescape($0);
	} else {
		after = after xml_unescape($0);
	}
}

xml_path ~ "/registry/types/type/apientry$" && xml_start {
	if (name == "") {
		before = before "APIENTRY";
	} else {
		after = after "APIENTRY";
	}
}

xml_path ~ "/registry/types/type/name$" && xml_text {
	name = xml_unescape($0)
	defined = 1
}

xml_path ~ "/registry/types/type$" && xml_end {
	if (api == "gl") {
		type_before[name] = before;
		type_after[name] = after;
		if (defined) { type_defined[name] = name; }
		type_requires[name] = requires;
	}
}

# Enumeration constants
#
xml_path ~ "/registry/enums/enum" && xml_start {
	enum_value[xml_attr("name")] = xml_attr("value")
}

# Commands
#
xml_path ~ "/registry/commands/command$" && xml_start {
	type = ""
	name = ""
	param_count = 0
	delete param_names
	delete param_types
	delete param_type_name
	param_type_name["count"] = 0
}

xml_path ~ "/registry/commands/command/proto" && xml_path !~ /name/ && xml_text {
	# Add content to return type
	type = type xml_unescape($0)
}

xml_path ~ "/registry/commands/command/proto/name" && xml_text {
	name = xml_unescape($0)
}

xml_path ~ "/registry/commands/command/param" && xml_path !~ /name$/ && xml_text {
	param_types[param_count] = param_types[param_count] xml_unescape($0)
}

xml_path ~ "/registry/commands/command/" && xml_path ~ /ptype$/ && xml_text {
	n = param_type_name["count"]++;
	param_type_name[n] = xml_unescape($0)
}

xml_path ~ "/registry/commands/command/param/name$" && xml_text {
	param_names[param_count] = xml_unescape($0)
}

xml_path ~ "/registry/commands/command/param$" && xml_end {
	param_count++
}

xml_path ~ "/registry/commands/command$" && xml_end {
	field = sprintf("%s(APIENTRYP %s)(", type, substr(name, 3)); 
	get_proc = "(" type "(APIENTRYP)("
	for (i = 0; i < param_count; i++) {
		if (i > 0) {
			field = field ", "
			get_proc = get_proc ", "
		}
		field = field param_types[i] param_names[i]
		get_proc = get_proc param_types[i]
	}
	if (param_count == 0) {
		field = field "void"
		get_proc = get_proc "void"
	}
	field = field ")"
	get_proc = get_proc "))gl_get_proc(api, \"" name "\")"

	cmd_field[name] = field
	cmd_get_proc[name] = get_proc
	cmd_names[name] = name

	cmd_types[name, "count"] = param_type_name["count"];
	for (i = 0; i < param_type_name["count"]; i++) {
		cmd_types[name, i] = param_type_name[i]
	}
}

# Common for features and extensions
#
xml_path ~ "/registry/(feature|extensions/extension)/require$" && xml_start {
	profile = xml_attr("profile");
	core_profile = (profile != "compatibility");
}

xml_path ~ "/registry/(feature|extensions/extension)/(require|remove)/(type|enum|command)$" && xml_start {
	match(xml_path, /(type|enum|command)$/);
	part = substr(xml_path, RSTART, RLENGTH);
}

# Features
#
xml_path ~ "/registry/feature$" && xml_start {
	api = xml_attr("api")
	name = xml_attr("name")
	number = xml_attr("number")

	if (split(number, parts, ".") != 2) {
		printf "unexpected version format: %s\n", number >/dev/stderr
		exit 1
	}

	# Force strings into numbers
	major = 0 + parts[1];
	minor = 0 + parts[2];

	# Store feature descriptions for later
	feature[api, major, minor] = name
	feature_api[name] = api
	feature_major[name] = major
	feature_minor[name] = minor
	feature_number[name] = number

	# Keep track of number of minor versions per major version
	if (major > max_major) { max_major = major; }
	if (minor > max_minor[major]) { max_minor[major] = minor; }

	feature_names[api, number] = name;

	# Check for parts that are added/removed per api per version
	for (part in feature_parts) {
		require[api, major, minor, part, "count"] = 0;
		removal[api, major, minor, part, "count"] = 0;
	}
}

# Check for enum|type|command require/remove
xml_path ~ "/registry/feature/require/(type|enum|command)$" && xml_start && core_profile {
	i = require[api, major, minor, part, "count"]++
	require[api, major, minor, part, i] = xml_attr("name")
}
xml_path ~ "/registry/feature/remove/(type|enum|command)$" && xml_start {
	i = removal[api, major, minor, part, "count"]++;
	removal[api, major, minor, part, i] = xml_attr("name")
}

#
# Extensions
xml_path ~ "/registry/extensions/extension$" && xml_start {
	name = xml_attr("name")
	supported = xml_attr("supported") ~ /^gl$|^gl\||\|gl\||\|gl$/;
	supported = supported && name ~ /ARB|KHR/;
	supported = supported && !(name in ext_blacklist);
	if (supported) {
		n = exts["count"]++
		exts[n] = name
	}
}

xml_path ~ "/registry/extensions/extension/require/(type|enum|command)$" && xml_start && supported && core_profile {
	n = require[name, part, "count"]++
	require[name, part, n] = xml_attr("name")
}

#
# Generate headers and sources
function min_minor_version(major_) {
	if (major__ == MIN_TARGET_MAJOR_VERSION) {
		return MIN_TARGET_MINOR_VERSION;
	} else {
		return 0;
	}
}

function max_minor_version(major_) {
	if (major_ == MAX_TARGET_MAJOR_VERSION) {
		return MAX_TARGET_MINOR_VERSION;
	} else {
		return max_minor[major_];
	}
}

function init_removals(i_, j_, k_, min_, end_, n_, feat_, name_, part_, vers_) {
	# clear global array
	removals["foo"] = 1;
	for (i_ in removals) {
		delete removals[i_];
	}

	# initialize per API version and check what and when things are removed
	# extensions ostensibly never remove anything
	for (i_ = 1; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		end_ = max_minor_version(i_);
		for (j_ = 0; j_ <= end_; j_++) {
			for (part_ in feature_parts) {
				n_ = removal["gl", i_, j_, part_, "count"];
				for (k_ = 0; k_ < n_; k_++) {
					feat_ = feature["gl", i_, j_];
					name_ = removal["gl", i_, j_, part_, k_];
					vers_ = feature_number[feat_];
					removals[part_, name_] = vers_;
				}
			}
		}
	}
}

# Is the feature part removed in this or future versions?
function is_removed_in(part_, name_, api_, feat_) {
	if ((part_ SUBSEP name_) in removals && api_ in feature) {
		feat_ = feature[api_];
		return removals[part_, name_] >= feature_number[feat_];
	} else {
		return 0;
	}
}

function add_out_type(name_, arr_, _i) {
	if (!arr_["added", name_]) {
		arr_["added", name_] = 1
		# add depended upon types recursively
		if (type_requires[name_]) {
			add_out_type(type_requires[name_], arr_);
		}
		i_ = arr_["count"]++;
		arr_[i_] = sprintf("%s%s%s", type_before[name_], type_defined[name_], type_after[name_]);
	}
}

function add_command_types(api_, arr_, i_, j_, m_, n_, name_, type_) {
	# For each command
	m_ = require[api_, "command", "count"];
	for (i_ = 0; i_ < m_; i_++) {
		name_ = require[api_, "command", i_];
		if (is_removed_in("command", name_, api_)) {
			continue;
		}
		# For each parameter
		n_ = cmd_types[name_, "count"]
		for (j_ = 0; j_ < n_; j_++) {
			type_ = cmd_types[name_, j_];
			if (!is_removed_in("type", type_, api_)) {
				add_out_type(type_, arr_);
			}
		}
	}
}

function add_required_types(api_, arr_, i_, n_, name_) {
	n_ = require[api_, "type", "count"];
	for (i_ = 0; i_ < n_; i_++) {
		name_ = require[api_, "type", i_]
		if (!is_removed_in("type", type_, api_)) {
			add_out_type(name_, arr_);
		}
	}
}

function init_types(i_, j_, end_, n_) {
	# Make sure the global `types` is an array
	types["foo"] = 1;
	for (i_ in types) {
		delete types[i_];
	}

	# Add types used by core API versions
	for (i_ = 1; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		end_ = max_minor_version(i_);
		for (j_ = 0; j_ <= end_; j_++) {
			# Look for types used by each command
			add_command_types(("gl" SUBSEP i_ SUBSEP j_), types);
			
			# Add required types not referenced by any command
			add_required_types(("gl" SUBSEP i_ SUBSEP j_), types);
		}
	}

	# Add types used by GL extensions
	end_ = exts["count"];
	for (i_ = 0; i_ < end_; i_++) {
		add_command_types(exts[i_], types);
		add_required_types(exts[i_], types);
	}
}

function add_required_enums(api_, arr_, i_, n_, j_, name_) {
	n_ = require[api_, "enum", "count"];
	for (i_ = 0; i_ < n_; i_++) {
		name_ = require[api_, "enum", i_];
		if (!is_removed_in("enum", name_, api_)) {
			if (!arr_["added", name_]) {
				arr_["added", name_] = 1;
				j_ = arr_["count"]++;
				arr_[j_] = name_;
			}
		}
	}
}

function init_enums(i_, j_, end_, n_) {
	# Make sure the global `enums` is an array
	enums["foo"] = 1;
	for (i_ in enums) {
		delete enums[i_];
	}

	# Add enums used by core API versions
	for (i_ = 1; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		end_ = max_minor_version(i_);
		for (j_ = 0; j_ <= end_; j_++) {
			# Add required enums
			add_required_enums(("gl" SUBSEP i_ SUBSEP j_), enums);
		}
	}

	# Add enums used by GL extensions
	end_ = exts["count"];
	for (i_ = 0; i_ < end_; i_++) {
		add_required_enums(exts[i_], enums);
	}
}

function add_required_commands(api_, arr_, srcapi_, i_, n_, j_, name_) {
	n_ = require[srcapi_, "command", "count"];
	for (i_ = 0; i_ < n_; i_++) {
		name_ = require[srcapi_, "command", i_]
		if (!is_removed_in("command", name_, srcapi_)) {
			if (!arr_[api_, "added", name_]) {
				arr_[api_, "added", name_] = 1
				j_ = arr_[api_, "count"]++;
				arr_[api_, j_] = name_;
			}
		}
	}
}

function add_gl_struct_members(major_, minor_, arr_, api_, srcapi_, end_, n_) {
	api_ = "gl" SUBSEP major_ SUBSEP minor_;

	arr_[api_, "name"] = sprintf("core%d%d", major_, minor_);

	# Include commands from API versions less than and including this one
	for (i_ = 1; i_ <= major_; i_++) {
		end_ = (i_ == major_) ? minor_ : max_minor[i_];
		for (j_ = 0; j_ <= end_; j_++) {
			srcapi_ = "gl" SUBSEP i_ SUBSEP j_;
			if (srcapi_ == api_) {
				n_ = arr_[api_, "count"];
				arr_[api_, "first"] = n_;
			}
			add_required_commands(api_, arr_, srcapi_);
			if (i_ > MIN_TARGET_MAJOR_VERSION || (i_ == MIN_TARGET_MAJOR_VERSION && j_ >= MIN_TARGET_MINOR_VERSION)) {
				n_ = arr_["gl", major_, minor_, "union", "count"]++;
				arr_["gl", major_, minor_, "union", n_] = arr_[srcapi_, "name"];
			}
		}
	}
}

function init_structs(i_, j_, begin_, end_, n_) {
	# Make sure global `structs` is an array
	structs["foo"] = 1;
	for (i_ in structs) {
		delete structs[i_];
	}

	# Add types used by core API versions
	for (i_ = MIN_TARGET_MAJOR_VERSION; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		begin_ = min_minor_version(i_);
		end_ = max_minor_version(i_);
		for (j_ = begin_; j_ <= end_; j_++) {
			# Add required enums
			structs["gl", i_, j_, "union", "count"] = 0;
			add_gl_struct_members(i_, j_, structs);
		}
	}
	i_ = MIN_TARGET_MAJOR_VERSION;
	j_ = MIN_TARGET_MINOR_VERSION;
	structs["gl", i_, j_, "first"] = 0;

	# Add structs for GL extensions
	end_ = exts["count"];
	for (i_ = 0; i_ < end_; i_++) {
		structs[exts[i_], "first"] = 0;
		structs[exts[i_], "name"] = substr(exts[i_], 4);
		add_required_commands(exts[i_], structs, exts[i_]);
	}
}

function print_prelude() {
	comment_ = comment
	printf "#if defined(_WIN32) && !defined(APIENTRY) && !defined(__CYGWIN__)\n" >>h_file;
	printf "#define APIENTRY __stdcall\n" >>h_file;
	printf "#endif\n" >>h_file;
	printf "\n" >>h_file;
	printf "#ifndef APIENTRY\n" >>h_file;
	printf "#define APIENTRY\n" >>h_file;
	printf "#endif\n" >>h_file;
	printf "\n" >>h_file;
	printf "#ifndef APIENTRYP\n" >>h_file;
	printf "#define APIENTRYP APIENTRY *\n" >>h_file;
	printf "#endif\n" >>h_file;
	printf "\n" >>h_file;
}

function print_api_struct(api_, i_, n_, name_) {
	printf "struct gl_%s\n{\n", structs[api_, "name"] >>h_file;
	n_ = structs[api_, "count"];
	for (i_ = 0; i_ < n_; i_++) {
		name_ = structs[api_, i_];
		printf "\t%s;\n", cmd_field[name_] >>h_file;
	}
	printf "};\n" >>h_file;
}

function print_unions(i_, j_, k_, begin_, end_, n_) {
	# Add unions for initialization/down-casting easier by utilizing the
	# fact that the function pointers of any two versions have a common
	# initial sequence
	printf "/*\n" >>h_file;
	printf " * Each subsequent version of the core structs only appends\n" >>h_file;
	printf " * new members to previous one. When added into a union they\n" >>h_file;
	printf " * each have pair-wise Common Initial Sequences. According\n" >>h_file;
	printf " * to the standard, it should be legal to write to one and\n" >>h_file;
	printf " * read another without running into aliasing optimization\n" >>h_file;
	printf " * issues. Initialize the highest version you need and cast\n" >>h_file;
	printf " * pointers to lower versions where necessary.\n" >>h_file;
	printf " */\n" >>h_file;
	for (i_ = MIN_TARGET_MAJOR_VERSION; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		begin_ = min_minor_version(i_);
		end_ = max_minor_version(i_);
		for (j_ = begin_; j_ <= end_; j_++) {
			printf "union gl_combined_%s\n{\n", structs["gl", i_, j_, "name"] >>h_file;
			n_ = structs["gl", i_, j_, "union", "count"];
			for (k_ = 0; k_ < n_; k_++) {
				name_ = structs["gl", i_, j_, "union", k_];
				printf "\tstruct gl_%s %s;\n", name_, name_ >>h_file;
			}
			printf "};\n\n" >>h_file;
		}
	}
}

function print_structs(i_, j_, k_, begin_, end_, n_) {
	# Add types used by core API versions
	printf "/* core OpenGL APIs */\n" >>h_file;
	for (i_ = MIN_TARGET_MAJOR_VERSION; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		begin_ = min_minor_version(i_);
		end_ = max_minor_version(i_);
		for (j_ = begin_; j_ <= end_; j_++) {
			# Add required enums
			print_api_struct("gl" SUBSEP i_ SUBSEP j_)
			printf "\n" >>h_file;
		}
	}

	# Add structs for GL extensions
	end_ = exts["count"];
	for (i_ = 0; i_ < end_; i_++) {
		if (structs[exts[i_], "count"] > 0) {
			printf "/* %s */\n", exts[i_] >>h_file;
			print_api_struct(exts[i_]);
			printf("\n") >>h_file;
		}
	}
}

function print_proto(name_) {
	printf "struct gl_%s *", name_ >>h_file;
	printf "gl_resolve_%s(", name_ >>h_file;
	printf "struct gl_api *, " >>h_file;
	printf "struct gl_%s *);\n", name_ >>h_file;
}

function print_prototypes(i_, j_, k_, begin_, end_, n_) {
	# Add types used by core API versions
	printf "/*\n * Initialize function pointers of core API structs. A GL\n" >>h_file;
	printf " * context needs to be current that supports the version in\n" >> h_file;
	printf " * question when calling.\n */\n" >>h_file;
	printf "struct gl_api;\n" >>h_file;
	for (i_ = MIN_TARGET_MAJOR_VERSION; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		begin_ = min_minor_version(i_);
		end_ = max_minor_version(i_);
		for (j_ = begin_; j_ <= end_; j_++) {
			# Add required enums
			name_ = structs["gl", i_, j_, "name"];
			print_proto(name_);
		}
	}

	# Add structs for GL extensions
	printf "\n/*\n *Initialize function pointers in extension struct. Return\n" >>h_file;
	printf " * NULL if extension is not supported, otherwise `ext`. A GL\n" >>h_file;
	printf " * context must be current when calling.\n */\n" >>h_file;

	end_ = exts["count"];
	for (i_ = 0; i_ < end_; i_++) {
		if (structs[exts[i_], "count"] > 0) {
			name_ = structs[exts[i_], "name"];
			print_proto(name_);
		}
	}
}

function print_lookups(api_, prefix_, i_, n_, name_)
{
	n_ = structs[api_, "count"];
	for (i_ = structs[api_, "first"]; i_ < n_; i_++) {
		name_ = structs[api_, i_];
		printf "%s%s = %s;\n", prefix_, substr(name_, 3), cmd_get_proc[name_] >>c_file;
	}
}

function print_core_function(major_, minor_, name_, prev_, var_)
{
	name_ = sprintf("core%d%d", major_, minor_);
	c_file = "resolve-core.c";
	if (!(c_file in files)) {
		printf "#include \"include/core.h\"\n\n" >c_file;
		printf "void (*gl_get_proc(struct gl_api *api, char const *))(void);\n" >>c_file;
		files[c_file] = c_file;
	}

	if (major_ != MIN_TARGET_MAJOR_VERSION || minor_ != MIN_TARGET_MINOR_VERSION) {
		if (minor_ == 0) {
			prev_ = sprintf("core%d%d", major_ - 1, max_minor[major_ - 1]);
		} else {
			prev_ = sprintf("core%d%d", major_, minor_ - 1);
		}
	} else {
		prev_ = 0;
	}

	printf "\n" >>c_file;
	printf "struct gl_%s *", name_ >>c_file;
	printf "gl_resolve_%s(", name_ >>c_file;
	printf "struct gl_api *api, ", name_ >>c_file;
	printf "struct gl_%s *core)\n{\n", name_ >>c_file;
	if (prev_) {
		printf "\tunion gl_combined_%s *u = ", name_ >>c_file;
		printf "(union gl_combined_%s *)core;\n", name_ >>c_file;
		printf "\t(void)gl_resolve_%s(api, &u->%s);\n\n", prev_, prev_ >>c_file;
		var_ = sprintf("\tu->%s.", name_);
	} else {
		var_ = "\tcore->";
	}
	print_lookups("gl" SUBSEP major_ SUBSEP minor_, var_);
	printf "\n\treturn core;\n" >>c_file;
	printf "}\n" >>c_file;
	c_file = "";
}

function print_ext_function(api_, name_, vnd_)
{
	name_ = structs[api_, "name"];
	vnd_ = tolower(substr(name_, 1, index(name_, "_") - 1));

	c_file = "resolve-" vnd_ ".c";
	if (!(c_file in files)) {
		printf "#include \"include/core.h\"\n\n" >c_file;
		printf "struct gl_api;\n\n" >>c_file;
		printf "void (*gl_get_proc(struct gl_api *, char const *))(void);\n" >>c_file;
		printf "int gl_has_ext(struct gl_api *, char const *);\n" >>c_file;
		files[c_file] = c_file;
	}

	printf "\n" >>c_file;
	printf "struct gl_%s *gl_resolve_%s(struct gl_api *api, ", name_, name_ >>c_file;
	printf "struct gl_%s *ext)\n{\n", name_ >>c_file;
	printf "\tif (!gl_has_ext(api, \"%s\")) {\n", exts[i_] >>c_file;
	printf "\t\treturn 0;\n\t}\n" >>c_file;
	print_lookups(api_, "\text->");
	printf "\n\treturn ext;\n" >>c_file;
	printf "}\n" >>c_file;
	c_file = "";
}

function print_functions(i_, j_, k_, begin_, end_, n_) {
	# Add types used by core API versions
	for (i_ = MIN_TARGET_MAJOR_VERSION; i_ <= MAX_TARGET_MAJOR_VERSION; i_++) {
		begin_ = min_minor_version(i_);
		end_ = max_minor_version(i_);
		for (j_ = begin_; j_ <= end_; j_++) {
			# Add required enums
			print_core_function(i_, j_)
		}
	}

	# Add structs for GL extensions
	end_ = exts["count"];
	for (i_ = 0; i_ < end_; i_++) {
		if (structs[exts[i_], "count"] > 0) {
			print_ext_function(exts[i_]);
		}
	}
}

END {
	if (MAX_TARGET_MAJOR_VERSION < 1) {
		MAX_TARGET_MAJOR_VERSION = max_major;
	}
	if (MAX_TARGET_MINOR_VERSION < 0) {
		MAX_TARGET_MINOR_VERSION = max_minor[MAX_TARGET_MAJOR_VERSION];
	}

	init_removals();
	init_types();
	init_enums();
	init_structs();

	# Generate header
	h_file = "include/core.h";
	files[h_file] = h_file;
	printf "" >h_file;
	print_prelude();
	for (i = 1; i <= MAX_TARGET_MAJOR_VERSION; i++) {
		end = max_minor_version(i);
		for (j = 0; j <= end; j++) {
			name = feature_names["gl", i "." j];
			printf "#define %s 1\n", name >>h_file;
		}
	}
	printf "\n" >>h_file;
	for (i = 0; i < enums["count"]; i++) {
		printf "#define %s %s\n", enums[i], enum_value[enums[i]] >>h_file;
	}
	printf "\n" >>h_file;
	for (i = 0; i < types["count"]; i++) {
		printf "%s\n", types[i] >>h_file;
	}
	printf "\n" >>h_file;
	print_structs();
	print_unions();
	print_prototypes();

	close(h_file);

	# Generate source file to resolve function pointers
	print_functions();

	for (file in files) {
		close(files[file]);
	}
}
'
