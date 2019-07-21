export NM=${NM:-nm}

NM() { SH '$(NM)' "$@"; }

define_ok_test()
{
  local source_="\$(SOURCE)/$MODULE_DIR/$1"
  local name_=$(basename "$source_" .c)

  case "$2" in
  (-*) ;;
  (?*.c) source_="\$(SOURCE)/$MODULE_DIR/$2"; shift;;
  esac
  shift

  local params_=''
  while [ $# -gt 0 ]; do
    if [ "$1" == ":" ]; then
      shift
      break
    else
      params_="$params_ $1"
      shift
    fi
  done

  local object_="\$(TARGET)/test/$MODULE/obj/$name_.o"
  local gensrc_="\$(TARGET)/test/$MODULE/src/$name_.ok.c"
  local genobj_="\$(TARGET)/test/$MODULE/obj/$name_.ok.o"
  local binary_="\$(TARGET)/test/$MODULE/bin/$name_"
  local depend_="\$(TARGET)/make/$MODULE/ok/make$name_.a"
  local script_="\$(SOURCE)/src/ok/stub.awk"

  # Build test functions
  cc_target "$object_ ${object_%.o}.d" "$source_ $*"
    CC_object "$object_" "$source_" $params_

  # Generate stub functions based on what was defined
  new_target "$gensrc_" "$object_" "$script_"
    NM -PA "$object_" \| awk -f "$script_" '>$@'

  # Build generated stub source
  cc_target "$genobj_ ${genobj_%.o}.d" "$gensrc_"
    CC_object "$genobj_" "$gensrc_"

  new_target "$depend_" "$object_" "$genobj_"
    AR "$depend_" "${object_%.o}.d" "${genobj_%.o}.d"

  ld_target "$binary_" "$object_" "$genobj_" \
    "\$(TARGET)/lib/lib$MODULE.a" \
    "\$(TARGET)/lib/libok.a" \
    "$depend_" \

    LD -o "\"$binary_"\" "\"$object_"\" "\"$genobj_"\" -lok -l$MODULE
}
