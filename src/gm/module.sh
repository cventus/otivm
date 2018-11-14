# generic math routines for graphics
LDLIBS="-lm"

define_header() {
  local name_=$1
  local source_="\$(SOURCE)/$MODULE_DIR/$2"
  shift 2
  target_source $name_ "$source_"
  cc_cmd -E -C "$@" "$source_" '>$@'
  end_target
}

define_generic() {
  local name_=$1
  local source_=$2
  shift 2
  define_object $name_.o gen/$source_.g.c "$@" $GENERIC_FLAGS
  define_header $name_.h gen/$source_.g.h "$@" $GENERIC_FLAGS
}

joined_header() {
  name_=$1
  HEADERS="$HEADERS gm/$name_.h"
  shift 1
  pathed_sources=$(printf " \$(TARGET)/src/$MODULE/%s" "$@")
  suffixed_sources=$(
    IFS=:
    for S in $TYPE_SUFFIXES; do 
      unset IFS
      printf " %s$S.h" $pathed_sources
    done
  )
  target_include gm/$name_.h $suffixed_sources
  sh_cmd \
    "sed -n '/begin gm header/,/end gm header/p' $suffixed_sources | " \
    "sed '/gm header/d' | " \
    "cat \$(SOURCE)/src/$MODULE/${name_}_prefix.h" \
    " - \$(SOURCE)/src/$MODULE/${name_}_suffix.h | " \
    "uniq " \
    '>$@'
  end_target
}

# Specialize math functions for the following types 
# ctype:suffix:literal:macro symbol:printf format:scanf format
exec 3<<'TYPES'
float:f:f:FLT::
double:::DBL::\"l\"
long double:l:l:LDBL:\"L\":\"L\"
TYPES

while
  IFS=: read -r CTYPE S LITERAL MACRO PRINTF_FORMAT SCANF_FORMAT <&3
do
  GENERIC_FLAGS="\
    -D T=\"$CTYPE\" \
    -D S=\"$S\" \
    -D F=\"$LITERAL\" \
    -D FPFX=\"$MACRO\" \
    -D PRINTF_FORMAT=\"$PRINTF_FORMAT\" \
    -D SCANF_FORMAT=\"$SCANF_FORMAT\" \
  "
  TYPE_SUFFIXES="$TYPE_SUFFIXES${TYPE_SUFFIXES:+:}$S"

  define_generic vector2$S vector -DL=2

  define_generic vector3$S vector -DL=3
  define_generic vector3x$S vector3x -DL=3

  define_generic quaternion$S vector -DL=4 -D VECTOR_TAG=q
  define_generic quaternionx$S quaternionx -DL=4 -D VECTOR_TAG=q

  define_generic matrix22$S matrix -D'M=2' -D'N=2'
  define_generic matrix22x$S matrix22x -D'M=2' -D'N=2'

  define_generic matrix33$S matrix -D'M=3' -D'N=3'
  define_generic matrix33x$S matrix33x -D'M=3' -D'N=3'

  define_generic matrix44$S matrix -D'M=4' -D'N=4'
  define_generic matrix44x$S matrix44x -D'M=4' -D'N=4'

  define_generic misc$S misc
  define_generic array$S array
  define_generic plane$S plane

  define_ok_test matrix22$S test-gen/matrix.g.c -D'M=2' -D'N=2' $GENERIC_FLAGS
  define_ok_test matrix22x$S test-gen/matrix22x.g.c -D'M=2' -D'N=2' $GENERIC_FLAGS
  define_ok_test matrix44$S test-gen/matrix.g.c -D'M=4' -D'N=4' $GENERIC_FLAGS
  define_ok_test matrix44x$S test-gen/matrix44x.g.c -D'M=4' -D'N=4' $GENERIC_FLAGS
done

joined_header matrix matrix22 matrix22x matrix33 matrix33x matrix44 matrix44x
joined_header vector vector2 vector3 vector3x
joined_header quaternion quaternion quaternionx
joined_header misc misc
joined_header array array
joined_header plane plane
