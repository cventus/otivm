# generic math routines for graphics
OUT=archive
LIB=-lm
MOD=ok
GINC=$(addprefix $(GINCDIR)/,matrix.h vector.h quaternion.h misc.h array.h)

# List of type generic tests
tests=

# Register tests with Makefile
TEST_OBJ:=$(foreach T,$(tests),$(addprefix $(OBJDIR)/test/$T,f.o .o l.o))

# Specify type specific targets' SOURCEs
$(foreach F,$(tests),\
$(foreach X,f.o .o l.o,\
$(eval $$(OBJDIR)/test/$F$X: SOURCE:=$$(MDIR)/generic-test/$F.g.c)))

gfiles=	vector2 \
	vector3 vector3x \
	quaternion quaternionx \
	matrix22 \
	matrix33 matrix33x \
	matrix44 matrix44x \
	array \
	misc \


# Register new targets 
OBJ+=$(foreach F,$(gfiles),$(foreach X,f.o .o l.o,$(OBJDIR)/$F$X))
GSRC+=$(foreach F,$(gfiles),$(foreach X,f.h .h l.h,$(GSRCDIR)/$F$X))

# $1: list of target files, $2: source file
def_osource=$(foreach F,$1,$(foreach X,f.o .o l.o,\
$(eval $(OBJDIR)/$F$X: SOURCE:=$(MDIR)/generic/$(if $2,$2,$1).g.c)))

# $1: list of generated header files, $2: source header file
def_hsource=$(foreach F,$1,$(foreach X,f.h .h l.h,\
$(eval $(GSRCDIR)/$F$X: $(MDIR)/generic/$(if $2,$2,$1).g.h)\
$(eval $(GSRCDIR)/$F$X: SOURCE:=$(MDIR)/generic/$(if $2,$2,$1).g.h)))

# Define SOURCE for objects and generated header files
def_source=$(call def_osource,$1,$2) $(call def_hsource,$1,$2)

$(call def_source,vector2 vector3 quaternion,vector)
$(call def_source,matrix22 matrix33 matrix44,matrix)
$(call def_source,vector3x)
$(call def_source,matrix33x)
$(call def_source,matrix44x)
$(call def_source,quaternionx)
$(call def_source,misc)
$(call def_source,array)

# float
$(foreach F,$(gfiles),$(GSRCDIR)/$Ff.h) \
$(foreach F,$(gfiles),$(OBJDIR)/$Ff.o) \
$(foreach F,$(tests),$(OBJDIR)/test/$Ff.o): \
		CPPFLAGS+=-D'T=float' \
                          -D'S=f' \
                          -D'F=f' \
                          -D'FPFX=FLT' \
                          -D'PRINTF_FORMAT=""' \
                          -D'SCANF_FORMAT=""'

# double
$(foreach F,$(gfiles),$(GSRCDIR)/$F.h) \
$(foreach F,$(gfiles),$(OBJDIR)/$F.o) \
$(foreach F,$(tests),$(OBJDIR)/test/$F.o): \
		CPPFLAGS+=-D'T=double' \
                          -D'S=' \
                          -D'F=' \
                          -D'FPFX=DBL' \
                          -D'PRINTF_FORMAT=""' \
                          -D'SCANF_FORMAT="l"'

# long double
$(foreach F,$(gfiles),$(GSRCDIR)/$Fl.h) \
$(foreach F,$(gfiles),$(OBJDIR)/$Fl.o) \
$(foreach F,$(tests),$(OBJDIR)/test/$Fl.o): \
		CPPFLAGS+=-D'T=long double' \
                          -D'S=l' \
                          -D'F=L' \
                          -D'FPFX=LDBL' \
                          -D'PRINTF_FORMAT="L"' \
                          -D'SCANF_FORMAT="L"'

# Add common CPPFLAGS for a set of files
def_cpp=$(foreach F,$1,\
$(eval $(foreach X,f.o .o l.o,$(OBJDIR)/$F$X) \
       $(foreach X,f.h .h l.h,$(GSRCDIR)/$F$X): CPPFLAGS+=$2))

$(call def_cpp,vector2,-D'L=2')
$(call def_cpp,vector3 vector3x,-D'L=3')
$(call def_cpp,quaternion quaternionx,-D'L=4' -D'VECTOR_TAG=q')
$(call def_cpp,matrix22,-D'M=2' -D'N=2')
$(call def_cpp,matrix33 matrix33x,-D'M=3' -D'N=3')
$(call def_cpp,matrix44 matrix44x,-D'M=4' -D'N=4')

# $1=file name, $2=list of generated, typed headers to concatenate
def_header=$(eval $$(GINCDIR)/$1.h: \
		$$(MDIR)/generic/$1_prefix.h \
		$(foreach F,$2,$$(foreach X,f.h .h l.h,$$(GSRCDIR)/$F$$X)) \
		$$(MDIR)/generic/$1_suffix.h) \
$(eval $$(GINCDIR)/$1.h: SOURCES+=$$(MDIR)/generic/$1_prefix.h) \
$(foreach F,$2,\
$(eval $$(GINCDIR)/$1.h: SOURCES+=$$(foreach X,f.h .h l.h,$$(GSRCDIR)/$F$$X))) \
$(eval $$(GINCDIR)/$1.h: SOURCES+=$$(MDIR)/generic/$1_suffix.h)

$(call def_header,vector,vector2 vector3 vector3x)
$(call def_header,quaternion,quaternion quaternionx)
$(call def_header,matrix,matrix22 matrix33 matrix33x matrix44 matrix44x)
$(call def_header,misc,misc)
$(call def_header,array,array)
