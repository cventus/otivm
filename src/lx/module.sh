SCRIPT="\$(SOURCE)/src/$MODULE/alx.awk"
STATE="\$(SOURCE)/src/$MODULE/test/alx"
TARGET_SRC="\$(TARGET)/src/$MODULE"

define_alx() {
  local alx bits_bytes bits bytes
  for alx; do
    for bits_bytes in 32:4; do
      bits=${bits_bytes%:*}
      bytes=${bits_bytes#*:}

      new_target "$TARGET_SRC/alx/$alx$bits.c" "$STATE/$alx.alx"
        SH awk -f "$SCRIPT" -v CELL_SIZE=$bytes "$STATE/$alx.alx" '>$@'
    done
  done
}

define_alx_test() {
  local bin="$1" src="$2" alx="$3"
  shift 3

  for bits in 32; do
    define_ok_test "$bin$bits" "$src" \
      -I $TARGET_SRC \
      -D STATE_DEFINITION="'\"alx/$alx$bits.c\"'" "$@" \
      : "$TARGET_SRC/alx/$alx$bits.c"
  done
}

define_alx lists rec shared tree copies adjacent wb-tree

define_alx_test adjacent-list test/list.c adjacent
define_alx_test linked-list test/list.c lists
define_alx_test compact test/compact.c lists
define_alx_test compact-tree  test/compact-tree.c tree
define_alx_test compact-shared test/compact-shared.c shared
define_alx_test compact-rec test/compact-rec.c rec
define_alx_test equals test/equals.c copies
define_alx_test wb-tree test/wb-tree.c wb-tree
define_ok_test test/cons.c
define_ok_test test/mark.c
define_ok_test test/ref.c
define_ok_test test/heap.c
define_ok_test test/modify.c
