# graphics
OUT=archive
MOD=base adt text gm
LIB=-lGL
TEST_MOD=ok
GSRC:=$(GSRCDIR)/genapi.h
GINC:=$(GINCDIR)/api.h

ifneq (,$(findstring glx,$(TAGS)))
LIB+=-lX11
endif

$(GSRCDIR)/genapi.h: $(MDIR)/api.h $(MDIR)/api.inc
$(GSRCDIR)/genapi.h: SOURCE:=$(MDIR)/api.h
$(GINCDIR)/api.h: $(GSRCDIR)/genapi.h
$(GINCDIR)/api.h: SOURCES:=$(GSRCDIR)/genapi.h

