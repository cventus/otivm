#!/bin/sh
set -ef

# Short-hand functions for defining targets and rules
new_target() { printf '%s: ' "$1"; shift; echo "$*"; }
cc_target() { new_target "$@" "\$(HEADERS.$MODULE)"; }
ld_target() { new_target "$@" "\$(LIBRARIES.$MODULE)"; }

SH() { printf '\t%s\n' "$*"; }
AR() { SH '$(AR)' -rc "$*"; }
CC() {
  SH "\$(CC) \$(CFLAGS) \$(CFLAGS.$MODULE)" \
    "\$(CPPFLAGS) \$(CPPFLAGS.$MODULE) $INCLUDE_FLAGS $*"
}
LD() {
  SH "\$(CC) \$(CFLAGS) \$(CFLAGS.$MODULE)" \
    "-L\$(TARGET)/lib \$(LDFLAGS) \$(LDFLAGS.$MODULE)" \
    "$* \$(LDLIBS.$MODULE) \$(LDLIBS)"
}

# Output command to compile a source file into an object with optional extra
# compiler flags (with default compilation flags) and generate makefile rules
# in the same directory as the object file (with a ".d" in place of ".o").
# Usage: object_cmd <target object> <source file> [<cc args>...]
CC_object() { CC -MMD -MP -MT "'\$$1'" -MF "${1%.o}.d" -c -o "$@"; }

define_source() { SOURCES="$SOURCES${SOURCES:+ }$*"; }

# Output a makefile rule to compile an object for a module.
# Usage: define_object <object> <source> [<cc flags> ...]
define_object() {
  OBJECTS="$OBJECTS${OBJECTS:+ }$1"
  define_object_obj_="\$(TARGET)/obj/$MODULE/$1"
  define_object_make_="\$(TARGET)/obj/$MODULE/${1%.o}.d"
  define_object_src_="\$(SOURCE)/$MODULE_DIR/$2"
  shift 2

  cc_target "$define_object_obj_ $define_object_make_" "$define_object_src_"
    CC_object "$define_object_obj_" "$define_object_src_" "$@"
}

# Output a makefile rule to compile a module as a binary.
# Usage: define_binary <name>
define_binary() {
  new_target "all" "\$(TARGET)/bin/${1:-$MODULE}"
  ld_target "\$(TARGET)/bin/${1:-$MODULE}" "\$(TARGET)/lib/lib$MODULE.a"
    LD "\$(TARGET)/lib/lib$MODULE.a" -o "\$(TARGET)/bin/${1:-$MODULE}"
}

build_module_variables() {
  printf 'CFLAGS.%s = %s\n' "$MODULE" "$CFLAGS"
  printf 'CPPFLAGS.%s = %s\n' "$MODULE" "$CPPFLAGS"
  printf 'LIBRARIES.%s =' "$MODULE"
  if [ "$REQUIRE" ]; then
    printf ' $(TARGET)/lib/lib%s.a' $REQUIRE
    printf ' $(LIBRARIES.%s)' $REQUIRE
  fi
  printf '\n'
  printf 'LDFLAGS.%s = %s' "$MODULE" "$LDFLAGS"
  if [ "$REQUIRE" ]; then printf ' $(LDFLAGS.%s)' $REQUIRE; fi
  printf '\n'
  printf 'LDLIBS.%s = %s' "$MODULE" "$LDLIBS"
  if [ "$REQUIRE" ]; then
    printf ' -l%s' $REQUIRE
    printf ' $(LDLIBS.%s)' $REQUIRE
  fi
  printf '\n'
}

build_module_generated_headers() {
  printf 'HEADERS.%s =' "$MODULE"
  [ "$HEADERS" ] && printf ' $(TARGET)/include/%s' $HEADERS
  [ "$REQUIRE" ] && printf ' $(HEADERS.%s)' $REQUIRE
  printf '\n'
}

build_module_targets() {
  # The default behavior when the module configuration defines neither sources
  # nor objects is to treat all source files in MODULE_DIR as source files,
  # which is convenient for small modules without a module configuration.
  if [ ! "$SOURCES$OBJECTS" ]; then SOURCES=$(ls | grep \\.c\$); fi
  for S in $SOURCES; do define_object "$(basename "$S" .c).o" "$S"; done

  if [ "$OBJECTS" ]; then
    printf 'OBJECTS.%s =' "$MODULE"
    printf '\\\n $(TARGET)/obj/'"$MODULE"'/%s' $OBJECTS
    echo
  fi

  # $1 = library path, $2 = makefile rules
  set "\$(TARGET)/lib/lib$MODULE.a" "\$(TARGET)/make/make$MODULE.a"

  new_target "$2" "\$(OBJECTS.$MODULE:.o=.d)"
    SH rm -f "$2"
    AR "$2" "\$(OBJECTS.$MODULE:.o=.d)"

  new_target "$1" "$2" "\$(OBJECTS.$MODULE)"
    SH rm -f "$1"
    AR -s "$1" "\$(OBJECTS.$MODULE)"

  new_target "all" "$1"
}

# Added first as-is to output makefile
build_makefile_header() {
  new_target all
  new_target .PHONY all

  echo
  echo "SOURCE = $SOURCE"
  echo "TARGET = $TARGET"
  echo "D = /$DIRECTORY_TARGET_FILE"
  echo
  echo "CFLAGS = $CFLAGS"
  echo "CPPFLAGS = $CPPFLAGS"
  echo "LDFLAGS = $LDFLAGS"
  echo "LDLIBS = $LDLIBS"
  echo
}

build_module_rules() {
  # Produce all normal rules (each ``module.sh'' file evaluated in an
  # independent sub-shell)
  for MODULE_DIR in $MODULES; do (
    require() { REQUIRE="$REQUIRE${REQUIRE:+ }$*"; }
    readonly MODULE_DIR MODULE=$(basename $MODULE_DIR)
    cd "$SOURCE/$MODULE_DIR"
    unset CFLAGS CPPFLAGS LDFLAGS LDLIBS
    unset SOURCES OBJECTS HEADERS REQUIRE
    if [ -f module.sh ]; then
      set +f
      if ! . ./module.sh; then
        >&2 echo "$(basename $0): in $SOURCE/$MODULE_DIR/module.sh"
	exit 1
      fi
      set -f
    fi
    for C in $MODULE_COMMANDS; do $C; done
  ); done
}

build_depend_rules() {
  # Include source header dependencies
  if [ -d "$TARGET/make" ]; then
    find "$TARGET/make"  -name 'make*.a' -exec $AR -p {} \;
  fi
}

# Print entire generated makefile to standard output
build_makefile() {
  build_makefile_header

  for C in $MAKEFILE_COMMANDS; do $C; done | LC_ALL=C awk '
  # Save all lines in a buffer and also expand escaped lines
  { buffer[n++] = $0; m++; line = line substr($0, 1, length($0) - /\\$/); }

  /^$/ || /[^\\]$/ {
    if (line !~ /^\t/) {
      sub(/#.*$/, "", line)
      gsub(/\$[{(][^{($:]*:[^({$})=]*=[^({$})]*[)}]/, "", line)
      if (line ~ /^[^:=]{1,}=/) { # variable definition
        n -= m
        for (i = 0; i < m; i++) { print buffer[n + i]; }
      } else if (line ~ /[^:]{1,}:/) { # rule definition
        # newline before rule
        for (i = 0; i < m; i++) { buffer[n - i] = buffer[n - i - 1]; }
        buffer[n - i] = ""
        n++

	# directory targets
        m = split(line, targets, /[ \t:]{1,}/)
        for (i = 1; i <= m; i++) {
          target = targets[i]
          while (target ~ /\$[({]TARGET[^})]*[})]\/./) {
            parent_dir = target
            sub(/\/[^\/]*$/, "", parent_dir)
            target_dirs[target] = parent_dir "$D"
            target = parent_dir "$D"
          }
        }
      }
    }
    m = 0;
    line = "";
  }

  END {
    for (i = 0; i < n; i++) { print buffer[i]; }
    if (length(parent_dir) > 0) {
      for (t in target_dirs) {
        if (t ~ /\$D$/) printf("\n%s \\", t) | "sort | uniq"
        printf("\n%s \\", target_dirs[t]) | "sort | uniq"
      }
      close("sort | uniq")
      printf(":\n\ttest -d $(@D) || mkdir $(@D)\n\ttouch $@\n\n")
      for (t in target_dirs) printf("%s: %s\n", t, target_dirs[t]) | "sort"
      close("sort")
    }
  }'
}

build_targets() {
  awk -v target_dir="$TARGET" -v glob_string="$1" '
  function is_target(target,    i) {
    for (i = 2; i <= nfilters; i++) {
      if (target ~ filters[i]) return 1;
    }
    return nfilters < 2
  }

  BEGIN {
    nfilters = length(glob_string) > 0 ? split(glob_string, globs, /\n{1,}/) : 0
    for (i = 2; i <= nfilters; i++) {
      glob = globs[i]
      # Escape regex-characters
      gsub(/[]().$^*+?[]/, "\\\\&", glob)

      # Translate glob wildcards into regex equivalents
      gsub(/\\\?/, "[^/]", glob)
      gsub(/\\\*\\\*/, ".*", glob)
      gsub(/\\\*/, "[^/]*", glob)

      filters[i] = glob
    }
  }

  { line = line substr($0, 1, length($0) - /\\$/); }

  /[^\\]$/ {
    if (line !~ /^\t/) {
      sub(/#.*$/, "", line)
      gsub(/\$[{(][^{($:]*:[^({$})=]*=[^({$})]*[)}]/, "", line)
      if (line ~ /[^:=]{1,}:/) {
        n = split(line, targets, /[ \t:]{1,}/)
        for (i = 1; i <= n; i++) {
	  target = targets[i]
          if (target ~ /^\$[({]TARGET[})]/ && target !~ /\$D$/) {
	    sub(/^\$[({]TARGET[})]/, target_dir, target)
            if (is_target(target)) print target
          }
        }
      }
    }
    line = "";
  }' | LC_ALL=C sort | uniq
}

NEWLINE="
"
MAKE_ARGUMENTS=; SHOW_TARGETS=; TARGET_TARGETS=; MAKE_TARGETS=; CLEAN_TARGETS=;
i=1
# Remove phony targets like "clean:foo" or "make:bar" from "$@" and store in
# *_TARGETS
for ARG; do
  case "$ARG" in
  show:*) SHOW_TARGETS="$SHOW_TARGETS${NEWLINE}${ARG#show:}";;
  target:*) TARGET_TARGETS="$TARGET_TARGETS${NEWLINE}${ARG#target:}";;
  make:*) MAKE_TARGETS="$MAKE_TARGETS${NEWLINE}${ARG#make:}";;
  clean:*) CLEAN_TARGETS="$CLEAN_TARGETS${NEWLINE}${ARG#clean:}";;
  *) MAKE_ARGUMENTS="$MAKE_ARGUMENTS \"\${$i}\"";;
  esac
  i=$((i + 1))
done
eval "set -- $MAKE_ARGUMENTS"
for ARG; do
  case "$ARG" in
  CFLAGS=*|CPPFLAGS=*|LDFLAGS=*|LDLIBS=*|TARGET=*|SOURCE=*|BUILD_CONFIG=*|AR=*)
    export "$ARG";;
  esac
done
unset i NEWLINE ARG MAKE_ARGUMENTS

: ${BUILD_CONFIG:=${SOURCE:-.}/build.conf} ${AR:=ar}
readonly SHOW_TARGETS TARGET_TARGETS MAKE_TARGETS CLEAN_TARGETS
readonly BUILD_CONFIG SOURCE=$(dirname "$BUILD_CONFIG")
readonly DIRECTORY_TARGET_FILE=.dir
export CFLAGS CPPFLAGS LDFLAGS LDLIBS

MAKEFILE_COMMANDS="\
build_module_rules
build_depend_rules
"

MODULE_COMMANDS="\
build_module_variables
build_module_generated_headers
build_module_targets
"

# read config
set +f
if [ -f "$BUILD_CONFIG" ]; then . "$BUILD_CONFIG"; fi
if [ -z "$MODULES" ]; then
  echo "MODULES not defined in environment or $BUILD_CONFIG" >&2
  exit 1
fi
set -f

readonly TARGET="${TARGET:=target}" INCLUDE="${INCLUDE:=include}"
readonly MODULES INCLUDE_FLAGS="\
-I\$(TARGET)/include\
${INCLUDE:+$(printf ' -I$(SOURCE)/%s' $INCLUDE)}"

readonly MAKEFILE=$(build_makefile)

# Either run make or show Makefile
if [ "$TARGET_TARGETS" ]; then
  echo "$MAKEFILE" | build_targets "$TARGET_TARGETS"
elif [ "$CLEAN_TARGETS" ]; then
  FILTERED_CLEAN=$(echo "$MAKEFILE" | build_targets "$CLEAN_TARGETS")
  if [ "$FILTERED_CLEAN" ]; then
    echo rm -f $FILTERED_CLEAN
    rm -f $FILTERED_CLEAN
  fi
elif [ "$MAKE_TARGETS" -o ! "$SHOW_TARGETS" ]; then
  echo "$MAKEFILE" | ${MAKE:-make} -rf- "$@" \
    $(if [ "$MAKE_TARGETS" ]; then
      echo "$MAKEFILE" | build_targets "$MAKE_TARGETS"
    fi)
else
  for ARG in $SHOW_TARGETS; do
    case "$ARG" in
    [Mm]akefile) echo "$MAKEFILE";;
    *) >&2 echo "$0: Unknown show option \"$ARG\""; exit 1;;
    esac
  done
fi
