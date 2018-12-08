require base gm adt
LDLIBS="-lm"

define_ok_test test/bezier2.c
define_ok_test test/geometry.c
define_ok_test test/quadedge.c
define_ok_test test/segment.c
define_ok_test test/shape.c
define_ok_test test/triangulate.c
