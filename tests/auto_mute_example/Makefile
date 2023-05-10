CXXFLAGS = 	-Wall \
			-Wextra \
		 	-std=c++2b \
			-I. \
			-I.. \
			-I../lib/cpputil \
			-I../../src \
			-Wno-unused-const-variable \
			-DTESTPROJECT \

SOURCES = auto_mute_tests.cc
BUILDDIR = build

LDFLAGS = -lstdc++
OBJECTS = $(addprefix $(BUILDDIR)/, $(subst ../,,$(addsuffix .o, $(basename $(SOURCES)))))
DEPFLAGS = -MT $@ -MMD -MP -MF $(BUILDDIR)/$(subst ../,,$(basename $<).d)

.PHONY: all clean

all: $(BUILDDIR)/automute_example

clean:
	rm -rf $(BUILDDIR)
	rm test.wav

$(BUILDDIR)/automute_example: $(OBJECTS)
	@$(CXX) $(LDFLAGS) -o $@ $(OBJECTS)
	@$(BUILDDIR)/automute_example
	@ls -l test.wav

$(BUILDDIR)/%.o: %.cc
	@mkdir -p $(dir $@)
	$(info Building $<)
	@$(CXX) -c $(DEPFLAGS) $(CXXFLAGS) $< -o $@
