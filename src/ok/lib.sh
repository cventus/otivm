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

  target_cc "$object_" "$source_"
    object_cmd "$object_" "$source_" "$@"
  end_target

  target_ld "$binary_" "$object_" \
    "\$(TARGET)/lib/lib$MODULE.a" \
    "\$(TARGET)/lib/libok.a" \
    "\$(TARGET)/make/$MODULE/ok\$D"

    ar_cmd "$depend_" "${object_%.o}.d"
    ld_cmd -o "\"$binary_"\" "\"$object_"\" -lok -l$MODULE
  end_target
}
