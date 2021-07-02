include $(SRC_PATH)/tools/common.mk

ifeq (,$(filter %clean config,$(MAKECMDGOALS)))
-include $(SUBDIR)lib$(NAME).version
endif

LIBVERSION := $(lib$(NAME)_VERSION)
INCINSTDIR := $(INCDIR)/lib$(NAME)

INSTHEADERS := $(INSTHEADERS) $(HEADERS:%=$(SUBDIR)%)

# TODO: adjust target for static and shared libraries
all-yes: $(SUBDIR)$(LIBNAME) $(SUBDIR)lib$(FULLNAME).pc

LIBOBJS := $(OBJS)

$(SUBDIR)$(LIBNAME): $(OBJS)
	$(RM) $@
	$(AR) $(ARFLAGS) $(AR_O) $^
	$(RANLIB) $@

install-headers: install-lib$(NAME)-headers install-lib$(NAME)-pkgconfig

install-lib-yes: install-lib$(NAME)-static

define RULES
$(SUBDIR)lib$(NAME).version: $(SUBDIR)version.h | $(SUBDIR)
	$$(M) $$(SRC_PATH)/tools/libversion.sh $(NAME) $$< > $$@

$(SUBDIR)lib$(FULLNAME).pc: $(SUBDIR)version.h config.sh | $(SUBDIR)
	$$(M) $$(SRC_PATH)/tools/pkgconfig_generate.sh $(NAME) "$(DESC)"

$(SUBDIR)lib$(NAME).ver: $(SUBDIR)lib$(NAME).v $(OBJS)
	$$(M)$(VERSION_SCRIPT_POSTPROCESS_CMD) > $$@

clean::
	$(RM) $(addprefix $(SUBDIR),$(CLEANFILES) $(CLEANSUFFIXES) $(LIBSUFFIXES))

install-lib$(NAME)-static: $(SUBDIR)$(LIBNAME)
	$(Q)mkdir -p "$(LIBDIR)"
	$$(INSTALL) -m 644 $$< "$(LIBDIR)"
	$(LIB_INSTALL_EXTRA_CMD)

install-lib$(NAME)-headers: $(addprefix $(SUBDIR),$(HEADERS) $(BUILT_HEADERS))
	$(Q)mkdir -p "$(INCINSTDIR)"
	$$(INSTALL) -m 644 $$^ "$(INCINSTDIR)"

install-lib$(NAME)-pkgconfig: $(SUBDIR)lib$(FULLNAME).pc
	$(Q)mkdir -p "$(PKGCONFIGDIR)"
	$$(INSTALL) -m 644 $$^ "$(PKGCONFIGDIR)"

uninstall-lib::
	-$(RM) "$(LIBDIR)/$(LIBNAME)"

uninstall-headers::
	$(RM) $(addprefix "$(INCINSTDIR)/",$(HEADERS) $(BUILT_HEADERS))
	-rmdir "$(INCINSTDIR)"

uninstall-pkgconfig::
	$(RM) "$(PKGCONFIGDIR)/lib$(FULLNAME).pc"
endef

$(eval $(RULES))
