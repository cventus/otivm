# Rule for building intermediate header files with the C pre-processor
#
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

