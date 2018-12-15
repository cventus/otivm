define_utility()
{
  local category_=util
  case "$1" in
  -c) category_=$2; shift 2;;
  -c*) category_=${1#-c}; shift;;
  esac

  local source_="\$(SOURCE)/$MODULE_DIR/$1"
  local name_=$(basename "$source_" .c)

  case "$2" in
  -*) ;;
  ?*.c) source_="\$(SOURCE)/$MODULE_DIR/$2"; shift;;
  esac
  shift

  local object_="\$(TARGET)/$category_/$MODULE/obj/$name_.o"
  local binary_="\$(TARGET)/$category_/$MODULE/bin/$name_"
  local depend_="\$(TARGET)/make/$MODULE/$category_/make$name_.d"

  cc_target "$object_ ${object_%.o}.d" "$source_"
    CC_object "$object_" "$source_" "$@"

  new_target "$depend_" "${object_%.o}.d"
    AR "$depend_" "${object_%.o}.d"

  ld_target "$binary_" "$object_" "$depend_" "\$(TARGET)/lib/lib$MODULE.a"
    LD -o "\"$binary_"\" "\"$object_"\" -l$MODULE
}
