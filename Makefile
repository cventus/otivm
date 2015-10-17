
.SUFFIXES:

# function : list of modules -> modules that are archives
ar_mod=$(foreach M,$1,$(if $(if $(OUT_$M),$(filter ar%,$(OUT_$M)),ok),$M))

# function : list of object files -> list of dependency files
obj2dep=$(subst $(OBJDIR_BASE),$(DEPDIR_BASE),$(1:.o=.d))

# function : list of modules -> module dependencies
modrec=$(foreach M,$(MOD_$1),$M $(call modrec,$M))

# function : list of modules -> module dependencies
testmodrec=$(foreach M,$(TEST_MOD_$1),$M $(call testmodrec,$M))

# function : module -> list of archive files
arrec=$(patsubst %,$(AR_DIR)/lib%.a,$(call modrec,$1))

# function : list of modules -> library dependencies
librec=$(foreach M,$1,$(call librec,$(MOD_$M))) $(LIB_$M)

# function : list of modules -> modules that have an include directory
modinc=$(foreach M,$1,$(if $(wildcard $(MODULE_DIR)/$M/include),$M))

# function : list of modules -> module include directories
incdir=$(foreach M,$(call modinc,$1),-I'$(INCDIR_BASE)/$M')

# function : list of modules -> modules that have generated header files
modginc=$(foreach M,$1,$(if $(GINC_$M),$M))

# function : list of modules -> module include directories
gincdir=$(foreach M,$(call modginc,$1),-I'$(GINCDIR_BASE)/$M')

# Template rule to create .d directory dependencies from a .o file path
# object=$1
define DEPDIR_TEMPLATE
$(subst $(OBJDIR_BASE),$(DEPDIR_BASE),$(1:.o=.d): | $(dir $(1:.o=.d)))
endef

# Template rule to set default object source parameter used by implicit rule
# stem=$1
define OBJECT_TEMPLATE
$(OBJDIR_BASE)/$1.o: SOURCE=$(MODULE_DIR)/$1.c
endef

# Template rule to add a file's path directories as order-only prerequisites
# file=$1
define FILEDIR_TEMPLATE
$1: | $(dir $1)
endef

# Template to load module config
# name=$1
define MODULE_CONFIG
# $1 config
OUT:=
OBJ:=
MOD:=
LIB:=
GINC:=
GSRC:=
OBJDIR:=$$(OBJDIR_BASE)/$1
GINCDIR:=$$(GINCDIR_BASE)/$1/$1
GSRCDIR:=$$(GSRCDIR_BASE)/$1
MDIR:=$$(MODULE_DIR)/$1
TEST_OBJ:=
TEST_MOD:=

# Check include during template expansion and include during evaluation
$(if $(wildcard $(MODULE_DIR)/$1/module.mk),-include $(MODULE_DIR)/$1/module.mk)

# Store module specific variables
SRC_$1:=$$(patsubst $$(MODULE_DIR)/%,%,\
	$$(wildcard $$(MDIR)/*.c) \
        $(foreach T,$(TAGS),$$(wildcard $$(MDIR)/$T/*.c)))
OBJ_$1:=$$(patsubst %.c,$$(OBJDIR_BASE)/%.o,$$(SRC_$1)) $$(OBJ)
OUT_$1:=$$(OUT)
MOD_$1:=$$(MOD)
LIB_$1:=$$(LIB)
GINC_$1:=$$(GINC)
GSRC_$1:=$$(GSRC)
TEST_SRC_$1:=$$(patsubst $$(MODULE_DIR)/%,%,\
	$$(wildcard $$(MDIR)/test/*.c) \
	$(foreach T,$(TAGS),$$(wildcard $$(MDIR)/test/$T/*.c)))
TEST_OBJ_$1:=$$(patsubst %.c,$(OBJDIR_BASE)/%.o,$$(TEST_SRC_$1)) $$(TEST_OBJ)
TEST_MOD_$1:=$$(TEST_MOD)

$$(sort $$(OBJDIR_BASE)/$1/ \
	$$(OBJDIR_BASE)/$1/test/ \
        $$(DEPDIR_BASE)/$1/ \
	$$(DEPDIR_BASE)/$1/test/ \
        $$(INCDIR_BASE)/$1/ \
        $$(GINCDIR_BASE)/$1/ \
        $$(GSRCDIR_BASE)/$1/ \
	$$(TESTDIR_BASE)/$1/bin/ \
        $$(dir $$(OBJ_$1)) \
        $$(dir $$(call obj2dep,$$(OBJ_$1))) \
        $$(dir $$(TEST_OBJ_$1)) \
        $$(dir $$(call obj2dep,$$(TEST_OBJ_$1))) \
        $$(dir $$(GINC_$1)) \
        ):
	mkdir -p $$@

$$(GINC_$1): | $$(GINCDIR_BASE)/$1
$$(GSRC_$1): | $$(GSRCDIR_BASE)/$1

$$(foreach I,$$(GINC_$1),\
  $$(eval $$(call FILEDIR_TEMPLATE,$$I)))

ifneq ($$(wildcard $$(MDIR)/include),)
$$(INCDIR_BASE)/$1/$1: | $$(INCDIR_BASE)/$1
	ln -rs $$(MODULE_DIR)/$1/include $$@
endif
endef

# Template for test
#
# Tests are C source files that reside in the test directory $MOD/test/ of any
# module. Each test should have a definition of main (or get a definition from
# a static library), and are linked against the module archive, the module and
# library dependencies of the module and whatever is defined in $(TEST_LDLIBS).
# A test file should preferably contain as little code as possible other than
# the test, and any utility functions that several tests use might as well be
# part of the module. A test named bar.c of module foo produces the binary bar
# in the build directory and a rule test-foo-bar is added to build it.
# Additionally, there is a rule test-foo which builds all the tests of module
# foo. Running tests, and converntions used by tests is up to whatever test
# harness is used.
#
# name=$1
define TEST_TEMPLATE

# Are the any tests at all?
ifneq ($$(TEST_OBJ_$1),)

# Add rule for each test binary
$(foreach T,$(TEST_OBJ_$1),\
$(call TESTBIN_TEMPLATE,$1,$(patsubst $(OBJDIR_BASE)/$1/test/%.o,%,$T),$T))

clean-test-$1:
test-$1:

test: test-$1
clean-test: clean-test-$1

TESTS += $$(TESTS_$1)

endif
endef

# Template for individual tests
#
# Binary tests are compiled into separate executables and linked against the
# module object files and its dependencies.
#
# name=$1, test=$2, source=$3
define TESTBIN_TEMPLATE

TESTS_$1 += $$(TESTDIR_BASE)/$1/bin/$2

clean-test-$1-$2:
	rm -f $$(TESTDIR_BASE)/$1/bin/$2
clean-test-$1: clean-test-$1-$2
test-$1-$2: $$(TESTDIR_BASE)/$1/bin/$2
test-$1: test-$1-$2

# Rule to link test executable
$$(TESTDIR_BASE)/$1/bin/$2: $$(AR_DIR)/lib$1.a \
                            $$(patsubst %.c,$$(OBJDIR_BASE)/%.o,$3) \
                            $$(call arrec,$1) \
                            | $$(TESTDIR_BASE)/$1/bin/
	$$(CC) $$(LDFLAGS) \
               $$(patsubst %.c,$$(OBJDIR_BASE)/%.o,$3) \
               $$(addprefix -l,$$(call ar_mod,$$(call testmodrec,$1))) \
               $$(TEST_LDLIBS) \
               -L$$(AR_DIR) \
               -l$1 \
               $$(addprefix -l,$$(call ar_mod,$$(call modrec,$1))) \
               $$(call librec,$1) \
               $$(LDLIBS) \
               -o "$$@"

endef

# Template to create rules for archives and binaries
# name=$1
define MODULE_TARGET
# $1 targets

# Add rules for each test
$(call TEST_TEMPLATE,$1)

# Conditionally include dependency files
$$(if $$(wildcard $$(call obj2dep,$$(OBJ_$1) $$(TEST_OBJ_$1))),\
      $$(eval -include $$(wildcard $$(call obj2dep,$$(OBJ_$1) \
                                                   $$(TEST_OBJ_$1)))))

# Rule to create and update the module archive
$$(AR_DIR)/lib$1.a: $$(OBJ_$1) | $$(AR_DIR)
	$$(AR) $$(ARFLAGS) $$@ $$?

$1: $$(AR_DIR)/lib$1.a

# For binary targets add rule to build it
ifneq ($$(filter bin%,$$(OUT_$1)),)

$$(BIN_DIR)/$1: $$(OBJ_$1) $$(call arrec,$1) | $$(BIN_DIR)
	$$(CC) $$(LDFLAGS) \
               $$(OBJ_$1) \
               -L$$(AR_DIR) \
               $$(addprefix -l,$$(call ar_mod,$$(call modrec,$1))) \
               $$(call librec,$1) \
               $$(LDLIBS) \
               -o $$@ 

# Default binary module target also builds archive
$1: $$(BIN_DIR)/$1
endif

# Add include directories for self and other modules
ifneq ($$(strip $$(call modinc,$1 $$(MOD_$1))),)
$$(OBJ_$1) $$(TEST_OBJ_$1): CPPFLAGS+=$$(call incdir,$1 $$(MOD_$1))
$$(OBJ_$1) $$(TEST_OBJ_$1): \
    | $$(foreach M,$$(call modinc,$1 $$(MOD_$1)),$$(INCDIR_BASE)/$$M/$$M)
endif

# Add include directories for test modules
ifneq ($$(strip $$(call modinc,$1 $$(TEST_MOD_$1))),)
$$(TEST_OBJ_$1): CPPFLAGS+=$$(call incdir,$1 $$(TEST_MOD_$1))
$$(TEST_OBJ_$1): \
    | $$(foreach M,$$(call modinc,$1 $$(TEST_MOD_$1)),$$(INCDIR_BASE)/$$M/$$M)
endif

# Depend on all generated headers from dependent modules
ifneq ($$(strip $$(call modginc,$1 $$(MOD_$1))),)
$$(OBJ_$1) $$(TEST_OBJ_$1): CPPFLAGS+=$$(call gincdir,$1 $$(MOD_$1))
$$(OBJ_$1) $$(TEST_OBJ_$1): $$(sort $$(foreach M,$1 $$(MOD_$1),$$(GINC_$$M)))
endif

# Depend on all generated headers from dependent test modules
ifneq ($$(strip $$(call modginc,$1 $$(TEST_MOD_$1))),)
$$(TEST_OBJ_$1): CPPFLAGS+=$$(call gincdir,$1 $$(TEST_MOD_$1))
$$(TEST_OBJ_$1): $$(sort $$(foreach M,$1 $$(TEST_MOD_$1),$$(GINC_$$M)))
endif

# Add directory and dependency file rules for each object
$$(foreach O,$$(OBJ_$1) $$(TEST_OBJ_$1),\
  $$(eval $$(call FILEDIR_TEMPLATE,$$O))\
  $$(eval $$(call DEPDIR_TEMPLATE,$$O)))

# Add SOURCE target specific variable for each object file
$$(foreach S,$$(basename $$(SRC_$1) $$(TEST_SRC_$1)),\
  $$(eval $$(call OBJECT_TEMPLATE,$$S)))
endef

define MODULE_RULES
$1-rules:
	$$(info $$(call MODULE_CONFIG,$1))
	$$(info $$(call MODULE_TARGET,$1))
endef

all:

-include config.mk

ifeq ($(strip $(MODULES)),)
$(error MODULES not defined! The file "config.mk" is a good place to put it.)
endif

all: $(MODULES)

clean: clean-test

test:

clean-test:

BUILD_DIR?=build
override DEPDIR_BASE:=$(BUILD_DIR)/depend
override OBJDIR_BASE:=$(BUILD_DIR)/object
override GSRCDIR_BASE:=$(BUILD_DIR)/gsrc
override GINCDIR_BASE:=$(BUILD_DIR)/ginc
override INCDIR_BASE:=$(BUILD_DIR)/include
override TESTDIR_BASE:=$(BUILD_DIR)/test
override AR_DIR:=$(BUILD_DIR)/lib
override BIN_DIR:=$(BUILD_DIR)/bin

$(BUILD_DIR) \
$(DEPDIR_BASE) \
$(OBJDIR_BASE) \
$(AR_DIR) \
$(BIN_DIR) \
$(INCDIR_BASE) \
$(TESTDIR_BASE) \
$(GSRCDIR_BASE) \
$(GINCDIR_BASE) \
	:
	mkdir -p $@

MODULE_DIR?=.

$(foreach M,$(MODULES),$(eval $(call MODULE_CONFIG,$M)))
$(foreach M,$(MODULES),$(eval $(call MODULE_TARGET,$M)))
$(foreach M,$(MODULES),$(eval $(call MODULE_RULES,$M)))

list-tests:
	@for test in $(TESTS); do \
	  echo $$test; \
	done

.PHONY: all clean test clean-test list-tests \
	$(foreach M,$(MODULES),$M test-$M clean-test-$M)

.PRECIOUS: $(DEPDIR_BASE)/%.d

# Rule to generate object dependency files
#
# Each object target has an auto-generated dependency file which lists the
# header files an object file depends on. Whenever this rule is invoked the
# target specific variable SOURCE needs to be set.
$(DEPDIR_BASE)/%.d:
	@set -e; \
	  $(CC) $(CPPFLAGS) -E -MM "$(SOURCE)" >$@.$$$$; \
	  sed 's,^[^:]*:,$@ $(patsubst $(DEPDIR_BASE)/%.d,$(OBJDIR_BASE)/%.o,$@):,' \
	  -i $@.$$$$; \
	  echo "$@: SOURCE=$(SOURCE)" >$@; \
	  echo "$@: CPPFLAGS=$(CPPFLAGS)" >>$@; \
	  cat "$@.$$$$" >>$@; \
	  rm -f "$@.$$$$"

# Rule for building object files
#
# Every object target needs to set the target specific variable SOURCE, which
# is used by both the dependency file and the object file. The target specific
# file is used in place of pattern rules so that it is possible for a
# many-to-one mapping between source files and object files. For instance, the
# same source can be compiled with different compiler flags to produce 
# different symbols at compile time.
$(OBJDIR_BASE)/%.o: | $(DEPDIR_BASE)/%.d
	$(CC) $(CFLAGS) $(CPPFLAGS) -c "$(SOURCE)" -o "$@"

# Target to print the value of a variable, e.g. `make print-BUILD_DIR`
print-%:
	@echo $($*)

# Rule for building intermediate header files with the C pre-processor
$(GSRCDIR_BASE)/%.h: $(SOURCE)
	@$(CPP) -C -nostdinc $(CPPFLAGS) "$(SOURCE)" | \
	  sed '/^#/d' | \
	  tr '\n' '#' | \
	  sed 's/##*/\n/g' | \
	  sed 's/;/;\n/' >"$@"

# A generated header consists of several files (generated headers) concatenated
# together
$(GINCDIR_BASE)/%.h: $(SOURCES)
	@cat $(foreach S,$(SOURCES),"$S") >"$@"

