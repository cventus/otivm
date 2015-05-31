# generic math routines for graphics
OUT=archive
LIB=-lm
MOD=ok
GINC=$(addprefix $(GINCDIR)/,matrix.h vector.h quaternion.h misc.h array.h)

override common_CPPFLAGS:=$(CPPFLAGS)

# List of type generic tests
tests=	matrix44 matrix44x

gfiles=	vector2 \
	vector3 vector3x \
	quaternion quaternionx \
	matrix22 \
	matrix33 matrix33x \
	matrix44 matrix44x \
	array \
	misc \


EXT_O=f.o .o l.o
EXT_H=f.h .h l.h

# $1: list of target files, $2: source file
def_osource=$(foreach F,$1,$(foreach X,$(EXT_O),\
$(eval $(OBJDIR)/$F$X: SOURCE:=$(MDIR)/gen/$(if $2,$2,$1).g.c)))

# $1: list of test files, $2: source file
def_test_source=$(foreach F,$1,$(foreach X,$(EXT_O),\
$(eval $(OBJDIR)/test/$F$X: SOURCE:=$(MDIR)/test-gen/$(if $2,$2,$1).g.c)))

# $1: list of generated header files, $2: source header file
def_hsource=$(foreach F,$1,$(foreach X,$(EXT_H),\
$(eval $(GSRCDIR)/$F$X: $(MDIR)/gen/$(if $2,$2,$1).g.h)\
$(eval $(GSRCDIR)/$F$X: SOURCE:=$(MDIR)/gen/$(if $2,$2,$1).g.h)))

# Define SOURCE for objects and generated header files
def_source=$(call def_osource,$1,$2) $(call def_hsource,$1,$2)

# Add common CPPFLAGS for a set of files
# If a source file needs additional pre-processor definitions, then the test
# should have the same name, i.e. for matrix44.o, the rows and columns macros
# are set, and if there's a test, it should be named test/matrix44.o. This
# ensures that sources and tests use the same pre-processor environment.
def_cppextra=$(foreach F,$1,\
$(foreach X,$(EXT_O),\
$(eval $(OBJDIR)/$F$X $(OBJDIR)/test/$F$X: gm_CPPEXTRAFLAGS:=$2))\
$(foreach X,$(EXT_H),$(eval $(GSRCDIR)/$F$X: gm_CPPEXTRAFLAGS:=$2)))

# $1=file name, $2=list of generated, typed headers to concatenate
def_header=$(eval $(GINCDIR)/$1.h: \
		$(MDIR)/gen/$1_prefix.h \
		$(foreach F,$2,$(foreach X,f.h .h l.h,$(GSRCDIR)/$F$X)) \
		$(MDIR)/gen/$1_suffix.h) \
$(eval $$(GINCDIR)/$1.h: SOURCES+=$(MDIR)/gen/$1_prefix.h) \
$(foreach F,$2,\
$(eval $(GINCDIR)/$1.h: SOURCES+=$(foreach X,f.h .h l.h,$(GSRCDIR)/$F$X))) \
$(eval $(GINCDIR)/$1.h: SOURCES+=$(MDIR)/gen/$1_suffix.h)


# $1=type, $2=function (and file) suffix, $3=literal suffix, $4=macro prefix,
# $5=printf format, $6=scanf format
def_type_cpp=$(foreach S,\
$(foreach F,$(gfiles),$(GSRCDIR)/$F$2.h) \
$(foreach F,$(gfiles),$(OBJDIR)/$F$2.o) \
$(foreach F,$(tests),$(OBJDIR)/test/$F$2.o),\
$(eval $S: CPPFLAGS:=-D'T=$1' -D'S=$2' -D'F=$3' -D'FPFX=$4' \
                                -D'PRINTF_FORMAT="$5"' \
                                -D'SCANF_FORMAT="$6"' \
                                $(common_CPPFLAGS)\
                                $$(gm_CPPEXTRAFLAGS)))

# Map objects to source files - the same source is re-used to build the same
# functions for different types
$(call def_source,vector2 vector3 quaternion,vector)
$(call def_source,matrix22 matrix33 matrix44,matrix)
$(call def_source,vector3x)
$(call def_source,matrix33x)
$(call def_source,matrix44x)
$(call def_source,quaternionx)
$(call def_source,misc)
$(call def_source,array)

# The same for tests, but tests do not have generic headers
$(call def_test_source,matrix22 matrix33 matrix44,matrix)
$(call def_test_source,matrix44x)
$(call def_test_source,equals)

$(call def_cppextra,vector2,-D'L=2')
$(call def_cppextra,vector3 vector3x,-D'L=3')
$(call def_cppextra,quaternion quaternionx,-D'L=4' -D'VECTOR_TAG=q')
$(call def_cppextra,matrix22,-D'M=2' -D'N=2')
$(call def_cppextra,matrix33 matrix33x,-D'M=3' -D'N=3')
$(call def_cppextra,matrix44 matrix44x,-D'M=4' -D'N=4')

$(call def_header,vector,vector2 vector3 vector3x)
$(call def_header,quaternion,quaternion quaternionx)
$(call def_header,matrix,matrix22 matrix33 matrix33x matrix44 matrix44x)
$(call def_header,misc,misc)
$(call def_header,array,array)

$(call def_type_cpp,float,f,f,FLT)
$(call def_type_cpp,double,,,DBL,,l)
$(call def_type_cpp,long double,l,l,LDBL,L,L)

# Register all targets
OBJ+=$(foreach F,$(gfiles),$(foreach X,$(EXT_O),$(OBJDIR)/$F$X))
GSRC+=$(foreach F,$(gfiles),$(foreach X,$(EXT_H),$(GSRCDIR)/$F$X))
TEST_OBJ+=$(foreach T,$(tests),$(addprefix $(OBJDIR)/test/$T,$(EXT_O)))

