require base
LDLIBS="-lX11"

define_ok_test test/test.c
