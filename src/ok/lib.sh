define_ok_test()
{
  local source_="\$(SOURCE)/$MODULE_DIR/$1"
  local name_=$(basename "$source_" .c)
  case "$2" in
  (-*) ;;
  (?*.c) source_="\$(SOURCE)/$MODULE_DIR/$2"; shift;;
  esac
  shift

  local object_="\$(TARGET)/test/$MODULE/obj/$name_.ok.o"
  local binary_="\$(TARGET)/test/$MODULE/bin/$name_"
  local depend_="\$(TARGET)/make/$MODULE/ok/make$name_.d"

  TESTS="$TESTS $binary_"

  cc_target "$object_" "$source_"
    CC_object "$object_" "$source_" "$@"

  ld_target "$binary_" "$object_" \
    "\$(TARGET)/lib/lib$MODULE.a" \
    "\$(TARGET)/lib/libok.a" \
    "\$(TARGET)/make/$MODULE/ok\$D"

    AR "$depend_" "${object_%.o}.d"
    LD -o "\"$binary_"\" "\"$object_"\" -lok -l$MODULE
}
