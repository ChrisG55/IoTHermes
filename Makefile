# SPDX-License-Identifier: GPL-3.0-or-later
MAIN_MAKEFILE=1
include config.mk

vpath %.c    $(SRC_PATH)
vpath %.h    $(SRC_PATH)
vpath %.texi $(SRC_PATH)
vpath %.v    $(SRC_PATH)

# first so "all" becomes default target
all: all-yes

include $(SRC_PATH)/tools/common.mk

config.h: .config
.config:
	@-tput bold 2>/dev/null
	@-printf '\nWARNING: $(?) newer than config.h, rerun configure\n\n'
	@-tput sgr0 2>/dev/null

SUBDIR_VARS := CLEANFILES HOSTPROGS HEADERS \
               BUILT_HEADERS OBJS SLIBOBJS HOSTOBJS

define RESET
$(1) :=
$(1)-yes :=
endef

$(foreach V,$(SUBDIR_VARS),$(eval $(call RESET,$(V))))
SUBDIR := lib/
include $(SRC_PATH)/lib/Makefile
include $(SRC_PATH)/tools/library.mk

include $(SRC_PATH)/src/Makefile
include $(SRC_PATH)/doc/Makefile

$(PROGS): %$(PROGSSUF)$(EXESUF): %$(PROGSSUF)_g$(EXESUF)
ifeq ($(STRIPTYPE),direct)
	$(STRIP) -o $@ $<
else
	$(CP) $< $@
	$(STRIP) $@
endif

%$(PROGSSUF)_g$(EXESUF):
	$(LD) $(LDFLAGS) $(LDEXEFLAGS) $(LD_O) $(OBJS-$*)

VERSION_SH = $(SRC_PATH)/tools/version.sh
GIT_LOG    = $(SRC_PATH)/.git/logs/HEAD

.version: $(wildcard $(GIT_LOG)) $(VERSION_SH) config.mk
.version: M=@

src/gdiotversion.h .version:
	$(M)$(VERSION_SH) $(SRC_PATH) src/gdiotversion.h $(EXTRA_VERSION)
	$(Q)touch .version

# force version.sh to run whenever version might have changed
-include .version

install: install-lib install-headers

install-lib: install-lib-yes

uninstall: uninstall-headers uninstall-lib uninstall-pkgconfig

clean::
	$(RM) $(CLEANSUFFIXES)

distclean:: clean
	$(RM) .version .config config.* \
		src/gdiotversion.h
	$(RM) -rf doc/examples/pc-uninstalled
ifeq ($(SRC_LINK),src)
	$(RM) src
endif

config:
	$(SRC_PATH)/configure $(value LIBGDIOT_CONFIGURATION)

build: all

$(sort $(OUTDIRS)):
	$(Q)mkdir -p $@

# Dummy rule to stop make trying to rebuild removed or renamed headers
%.h:
	@:

.SUFFIXES:

.PHONY: all all-yes build config
.PHONY: *clean install* uninstall*
