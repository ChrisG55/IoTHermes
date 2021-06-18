# SPDX-License-Identifier: GPL-3.0-or-later
GDIOTPROGS-$(CONFIG_GDIOT_SRC)  += gdiot-src
GDIOTPROGS-$(CONFIG_GDIOT_GATE) += gdiot-gate
GDIOTPROGS-$(CONFIG_GDIOT_EVAP) += gdiot-evap

GDIOTPROGS := $(GDIOTPROGS-yes:%=%$(PROGSSUF)$(EXESUF))
PROGS   += $(GDIOTPROGS)

GDIOTBASENAMES  = gdiot-src gdiot-gate gdiot-evap
ALLGDIOTPROGS   = $(GDIOTBASENAMES:%=%$(PROGSSUF)$(EXESUF))
ALLGDIOTPROGS_G = $(GDIOTBASENAMES:%=%$(PROGSSUF)_g$(EXESUF))

define DOGDIOTPROG
OBJS-$(1) += src/args.o src/exit.o src/io.o src/$(1).o $(OBJS-$(1)-yes)
$(1)$(PROGSSUF)_g$(EXESUF): $$(OBJS-$(1))
$$(OBJS-$(1)): | src
$$(OBJS-$(1)): CFLAGS  += $(CFLAGS-$(1))
$(1)$(PROGSSUF)_g$(EXESUF): LDFLAGS += $(LDFLAGS-$(1))
$(1)$(PROGSSUF)_g$(EXESUF): GDIOT_EXTRALIBS += $(EXTRALIBS-$(1))
-include $$(OBJS-$(1):.o=.d)
endef

$(foreach P,$(GDIOTPROGS-yes),$(eval $(call DOGDIOTPROG,$(P))))

all: $(GDIOTPROGS)

src/gdiot-src.o src/gdiot-gate.o src/gdiot-evap.o: | src
OUTDIRS += src

ifdef GDIOTPROGS
install: install-progs
endif

install-progs-yes:
install-progs-$(CONFIG_SHARED): install-lib

install-progs: install-progs-yes $(GDIOTPROGS)
	$(Q)mkdir -p "$(BINDIR)"
	$(INSTALL) -c -m 755 $(GDIOTPROGS) "$(BINDIR)"

uninstall: uninstall-progs

uninstall-progs:
	$(RM) $(addprefix "$(BINDIR)/", $(ALLGDIOTPROGS))

clean::
	$(RM) $(ALLGDIOTPROGS) $(ALLGDIOTPROGS_G) $(CLEANSUFFIXES:%=src/%)
