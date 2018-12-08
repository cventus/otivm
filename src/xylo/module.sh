# xylo 2D renderer
require base adt gm glapi glam spline tempo

for test in test/*.c; do
  define_ok_test $test
done
