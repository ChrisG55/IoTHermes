# SPDX-License-Identifier: GPL-3.0-or-later
#
# common bits used by all libraries
#

ifndef SUBDIR

ifndef V
Q     = @
ECHO  = printf "$(1)\t%s\n" $(2)

M     = @$(call ECHO,$(TAG),$@);
$(eval INSTALL = @$(call ECHO,INSTALL,$$(^:$(LIB_DIR)/%=%)); $(INSTALL))
endif

IFLAGS   := -I. -I$(SRC_LINK)/
CPPFLAGS := $(IFLAGS) $(CPPFLAGS)
CCFLAGS   = $(CPPFLAGS) $(CFLAGS)

LDFLAGS  := $(LD_PATH)lib $(LDFLAGS)

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

%.c %.h %.pc %.version: TAG = GEN

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
GDIOTLIB    := $($(NAME)_GDIOTLIB) $(GDIOTLIB-yes) $(GDIOTLIB)

OBJS        := $(sort $(OBJS:%=$(SUBDIR)%))
HEADERS     += $(HEADERS-yes)

PATH_LIBNAME     = $(foreach NAME,$(1),lib/$($(2)LIBNAME))
STATIC_DEP_LIBS := $(foreach lib,$(GDIOTLIB),$(call PATH_LIBNAME,$(lib)))

LIB_DIR    := $(SRC_PATH)/lib
ALLHEADERS := $(subst $(LIB_DIR)/,$(SUBDIR),$(wildcard $(LIB_DIR)/*.h))

$(OBJS):     | $(sort $(dir $(OBJS)))

OUTDIRS := $(OUTDIRS) $(dir $(OBJS))

CLEANSUFFIXES = *.d *.o *.pc *.version *~
LIBSUFFIXES   = *.a *.so

-include $(wildcard $(OBJS:.o=.d))
