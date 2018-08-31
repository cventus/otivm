#!/bin/sh

set -e

# Selectively look for make-style variable assignments
for A; do
  case "$A" in
  CFLAGS=*|CPPFLAGS=*|LDFLAGS=*|LDLIBS=*) eval "export '$A'";;
  TARGET=*|SOURCE=*) eval "export '$A'";;
  BUILD_CONFIG=*) eval "export '$A'";;
  esac
done

: ${AR:=ar} ${BUILD_CONFIG:=${SOURCE:-.}/build.conf}

readonly BUILD_CONFIG SOURCE=$(dirname "$BUILD_CONFIG")
readonly DIRECTORY_TARGET_FILE=.dir

globexists() { test -f "$1"; }

export CFLAGS CPPFLAGS LDFLAGS LDLIBS

MAKEFILE_COMMANDS="\
build_module_rules
build_list_tests_target
build_depend_rules
"

MODULE_COMMANDS="\
build_module_variables
build_module_tests
build_module_generated_headers
build_module_targets
"

if [ -f "$BUILD_CONFIG" ]; then . "$BUILD_CONFIG"; fi
if [ -z "$MODULES" ]; then
  echo "MODULES not defined in environment or $BUILD_CONFIG" >&2
  exit 1
fi

readonly MODULES
readonly TARGET="${TARGET:=target}" SOURCE_INCLUDE="${SOURCE_INCLUDE:=include}"
readonly DEPEND_FLAGS='-MMD -MP -MT $@ -MF'
readonly INCLUDE_FLAGS="\
-I\$(TARGET)/include\
${SOURCE_INCLUDE:+$(printf ' -I$(SOURCE)/%s' $SOURCE_INCLUDE)}"

# Module specific variables, defined when module configuration is sourced
unset SOURCES OBJECTS HEADERS REQUIRE TESTS BINARY

require() { REQUIRE="$REQUIRE $*"; }
binary() { BINARY="${1:-$MODULE}"; }

# Short-hand functions for defining rules
target_generic() { printf '%s: ' "$1"; shift; echo "$*"; }
target_phony() { target_generic .PHONY "$*"; }
target_source() { target_generic "\$(TARGET)/src/$MODULE/$@"; }
target_include() { target_generic "\$(TARGET)/include/$@"; }
target_cc() { target_generic "$@" "\$(HEADERS.$MODULE)"; }
target_ld() { target_generic "$@" "\$(LIBRARIES.$MODULE)"; }
end_target() { printf '\n'; }

# Functions for adding rule commands
sh_cmd() { printf '\t%s\n' "$*"; }
ar_cmd() { sh_cmd '$(AR)' -rc "$*"; }

# Output command to compile a single source file (add neccessary compilation
# and preprocessor flags).
# Usage: cc_cmd <cc args>...
cc_cmd() {
  sh_cmd "\$(CC) \$(CFLAGS) \$(CFLAGS.$MODULE)" \
    "\$(CPPFLAGS) \$(CPPFLAGS.$MODULE) $INCLUDE_FLAGS $*"
}

# Output command to compile a single source file (add neccessary compilation
# and preprocessor flags) and generate makefile rules at the specified path at
# the same time.
# Usage: ccmk_cmd <makefile rules path> <cc args>...
ccmk_cmd() { cc_cmd "$DEPEND_FLAGS" "$@"; }

# Output command to compile and link a single source file (add neccessary
# linkage flags).
# Usage: ld_cmd <cc args>...
ld_cmd() {
  sh_cmd "\$(CC) \$(CFLAGS) \$(CFLAGS.$MODULE)" \
    "-L\$(TARGET)/lib \$(LDFLAGS) \$(LDFLAGS.$MODULE)" \
    "$* \$(LDLIBS.$MODULE) \$(LDLIBS)"
}

# Output command to compile a source file into an object with optional extra
# compiler flags (with default compilation flags) and generate makefile rules
# in the same directory as the object file (with a ".d" appended to the
# object's path name.
# Usage: object_cmd <target object> <source file> [<cc args>...]
object_cmd() {
  object_cmd_obj_="$1"
  shift 1
  ccmk_cmd "${object_cmd_obj_%.o}.d" -o "$object_cmd_obj_" -c "$@"
}

# Output a makefile rule to compile an object for a module.
# Usage: define_object <object> <source> [<cc flags> ...]
define_object() {
  OBJECTS="$OBJECTS $1"
  define_object_obj_="\$(TARGET)/obj/$MODULE/$1"
  define_object_src_="\$(SOURCE)/$MODULE_DIR/$2"
  shift 2

  target_cc "$define_object_obj_" "$define_object_src_"
    object_cmd "$define_object_obj_" "$define_object_src_" "$@"
  end_target
}

# Called in context of a module where CFLAGS, CPPFLAGS, LDFLAGS and LDLIBS are
# module specific
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

build_module_tests() {
  printf 'TESTS.%s =' "$MODULE"
  [ "$TESTS" ] && printf ' %s' $TESTS
  printf '\n'

  target_phony "$MODULE-test"
  target_generic "test $MODULE-test" "\$(TESTS.$MODULE)"
}

build_module_generated_headers() {
  printf 'HEADERS.%s =' "$MODULE"
  [ "$HEADERS" ] && printf ' $(TARGET)/include/%s' $HEADERS
  [ "$REQUIRE" ] && printf ' $(HEADERS.%s)' $REQUIRE
  printf '\n'
}

# Create common module targets, such as lib$MODULE.a and binary
build_module_targets() {
  # The default behavior when the module configuration defines neither sources
  # nor objects is to treat all source files in MODULE_DIR as source files,
  # which is convenient for small modules without even a module configuration.
  if [ -z "$SOURCES$OBJECTS" ] && globexists *.c; then SOURCES=$(echo *.c); fi
  for S in $SOURCES; do define_object "$(basename $S .c).o" $S; done

  if [ "$OBJECTS" ]; then
    printf 'OBJECTS.%s =' "$MODULE"
    printf '\\\n $(TARGET)/obj/'"$MODULE"'/%s' $OBJECTS
    echo
  fi

  # $1 = library path
  set "\$(TARGET)/lib/lib$MODULE.a"

  target_phony "$MODULE" "lib$MODULE.a"
  target_generic "all $MODULE lib$MODULE.a" "$1"
  echo

  target_generic "$1" "\$(OBJECTS.$MODULE)" '$(TARGET)/make$D'
    sh_cmd rm -f "$1" "\$(TARGET)/make/make$MODULE.a"
    ar_cmd -s "$1" "\$(OBJECTS.$MODULE)"
    ar_cmd "\$(TARGET)/make/make$MODULE.a" "\$(OBJECTS.$MODULE:.o=.d)"
  end_target

  if [ "$BINARY" ]; then
    target_generic "all $MODULE" "\$(TARGET)/bin/$BINARY"
    if [ "$BINARY" != "$MODULE" ]; then
      target_generic "$BINARY" "\$(TARGET)/bin/$BINARY"
      target_phony "$BINARY"
    fi
    target_ld "\$(TARGET)/bin/$BINARY" "$1"
      ld_cmd "$1" -o "\$(TARGET)/bin/$BINARY"
    end_target
  fi
}

# Added first as-is to output makefile
build_makefile_header() {
  target_generic all
  target_generic test
  target_generic clean \; find "$TARGET" -type f -exec rm -- {} '\;'
  target_phony all test clean

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

build_list_tests_target() {
  target_generic list-tests
    sh_cmd printf '"%s\\n"' $(for MODULE_DIR in $MODULES; do
      printf ' $(TESTS.%s)' "$(basename $MODULE_DIR)"
    done)
  end_target
  target_phony list-tests
}

build_module_rules() {
  echo
  # Produce all normal rules (each ``module.sh'' file evaluated in an
  # independent sub-shell)
  for MODULE_DIR in $MODULES; do (
    readonly MODULE_DIR MODULE=$(basename $MODULE_DIR)
    cd "$SOURCE/$MODULE_DIR"
    unset CFLAGS CPPFLAGS LDFLAGS LDLIBS
    if [ -f module.sh ]; then . ../$MODULE/module.sh; fi
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
      if (line ~ /^[^:=]{1,}=/) { // variable definition
        n -= m
        for (i = 0; i < m; i++) { print buffer[n + i]; }
      } else if (line ~ /[^:]{1,}:/) { // rule definition
        m = split(line, targets, /[ \t:]{1,}/)
        for (i = 1; i <= m; i++) {
          target = targets[i]
          while (target ~ /\$[({]TARGET[^})]*[)]\/./) {
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
      print ":; test -d $(@D) || mkdir $(@D); touch $@;\n"
      for (t in target_dirs) printf("%s: %s\n", t, target_dirs[t]) | "sort"
      close("sort")
    }
  }'
}

case "$*" in
[Mm]akefile) build_makefile;;
*) build_makefile | ${MAKE:-make} -rf- "$@";;
esac
