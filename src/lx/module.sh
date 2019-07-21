SCRIPT="\$(SOURCE)/src/$MODULE/alx.awk"
STATE="\$(SOURCE)/src/$MODULE/test/alx"
TARGET_SRC="\$(TARGET)/src/$MODULE"

define_alx() {
  for bits_bytes in 32:4; do
    bits=${bits_bytes%:*}
    bytes=${bits_bytes#*:}

    new_target "$TARGET_SRC/alx/${1}$bits.c" "$STATE/$1.alx"
      SH awk -f "$SCRIPT" -v CELL_SIZE=$bytes "$STATE/$1.alx" '>$@'
  done
}

define_alx_test() {
  local bin="$1" src="$2" alx="$3"
  shift 3

  for bits in 32; do
    bits=${bits_bytes%:*}

    define_ok_test "$bin$bits" "$src" \
      -I $TARGET_SRC \
      -D STATE_DEFINITION="'\"alx/$alx$bits.c\"'" "$@" \
      : "$TARGET_SRC/alx/$alx.c"
  done
}

define_alx lists

define_ok_test test/equals.c
define_ok_test adjacent-list test/list.c -DADJACENT_LIST_TEST
define_ok_test linked-list test/list.c -DLINKED_LIST_TEST
define_ok_test test/cons.c
define_alx_test compact test/compact.c lists
define_ok_test test/compact-tree.c
define_ok_test test/compact-shared.c
define_ok_test test/compact-rec.c
define_ok_test test/mark.c
define_ok_test test/ref.c
define_ok_test test/heap.c
define_ok_test test/modify.c
