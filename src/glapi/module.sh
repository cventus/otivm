# graphics
require base adt text gm

LDLIBS="-lGL"
define_source *.c

if contains "$TAGS" glx; then
  LDLIBS="$LDLIBS -lX11"
  define_source glx/*.c
fi

define_utility util/info.c

define_ok_test test/context.c
