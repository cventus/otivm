# main
binary main
require base text glapi
LDLIBS="-lGL"

if contains "$TAGS" posix; then
  require xw
  LDLIBS="$LDLIBS -lX11"
  SOURCES="$SOURCES $(echo posix/*.c)"
fi
