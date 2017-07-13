MODULES=ok base adt gm text fs tempo rescache wf glapi glcache glam xw main xylo spline
TAGS=posix glx
MODULE_DIR=src

CFLAGS=-g -Wall -Wextra -pedantic -Wswitch-default -Wswitch-enum -Wshadow -std=c11 -Werror

ifneq (,$(findstring posix,$(TAGS)))
CFLAGS+=-D_POSIX_C_SOURCE=199309L
endif
