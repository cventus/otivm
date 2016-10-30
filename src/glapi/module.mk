# graphics
OUT=archive
MOD=base adt text gm
LIB=-lGL
TEST_MOD=ok

ifneq (,$(findstring glx,$(TAGS)))
LIB+=-lX11
endif

