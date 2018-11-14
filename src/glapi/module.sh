# graphics
require base adt text gm

LDLIBS="-lGL"
SOURCES=*.c

if contains "$TAGS" glx; then
  LDLIBS="$LDLIBS -lX11"
  SOURCES="$SOURCES $(echo glx/*.c)"
fi

define_ok_test test/context.c
