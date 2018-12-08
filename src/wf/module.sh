require base text gm
LDLIBS="-lm"

define_ok_test test/mtl.c
define_ok_test test/obj.c
