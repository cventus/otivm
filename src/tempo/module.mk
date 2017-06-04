# time keeping functionality
OUT=archive
TEST_MOD=ok

ifneq (,$(findstring posix,$(TAGS)))
LIB+=-lrt
endif
