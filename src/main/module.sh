# main
require base text glapi
LDLIBS="-lGL"

if contains "$TAGS" posix; then
  require xw
  LDLIBS="$LDLIBS -lX11"
  define_source posix/*.c
fi

define_binary main
