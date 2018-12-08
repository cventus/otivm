# time keeping functionality
define_source *.c

if contains "$TAGS" posix; then
  define_source posix/*.c
  LDLIBS=-lrt
fi

define_ok_test test/stopwatch.c
