# SPDX-License-Identifier: GPL-3.0-or-later
#
# common bits used by all libraries
#

ifndef SUBDIR

ifndef V
Q     = @
ECHO  = printf "$(1)\t%s\n" $(2)

M     = @$(call ECHO,$(TAG),$@);
endif

IFLAGS   := -I. -I$(SRC_LINK)/
CPPFLAGS := $(IFLAGS) $(CPPFLAGS)
CCFLAGS   = $(CPPFLAGS) $(CFLAGS)

define COMPILE
	$(call $(1)DEP,$(1))
	$($(1)) $($(1)FLAGS) $($(1)_DEPFLAGS) $($(1)_C) $($(1)_O) $(patsubst $(SRC_PATH)/%,$(SRC_LINK)/%,$<)
endef

COMPILE_C = $(call COMPILE,CC)

%.o: %.c
	$(COMPILE_C)

%.s: %.c
	$(CC) $(CFLAGS) -S -o $@ $<

%.i: %.c
	$(CC) $(CFLAGS) $(CC_E) $<

%.c %.h %.version: TAG = GEN

# Dummy rule to stop make tryping to rebuild removed or renamed headers
%.h:
	@:

# Disable suffix rules. Most of the builtin rules are suffix rules,
# so this saves some time on slow systems
.SUFFIXES:

# Do not delete intermediate files from chains of implicit rules
$(OBJS):
endif

OBJS        += $(OBJS-yes)

OBJS        := $(sort $(OBJS:%=$(SUBDIR)%))
HEADERS     += $(HEADERS-yes)

$(OBJS):     | $(sort $(dir $(OBJS)))

OUTDIRS := $(OUTDIRS) $(dir $(OBJS))

CLEANSUFFIXES = *.d *.o *.version *~

-include $(wildcard $(OBJS:.o=.d))
