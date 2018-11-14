# time keeping functionality
SOURCES=*.c

if contains "$TAGS" posix; then
  SOURCES="$SOURCES $(echo posix/*.c)"
  LDLIBS=-lrt
fi

define_ok_test test/stopwatch.c
