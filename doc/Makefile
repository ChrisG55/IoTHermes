# SPDX-License-Identifier: GPL-3.0-or-later
HTMLPAGES = $(GDIOTPROGS-yes:%=doc/%.html) doc/gdiot.html
INFOPAGES = $(GDIOTPROGS-yes:%=doc/%.info) doc/gdiot.info
TXTPAGES  = $(GDIOTPROGS-yes:%=doc/%.txt) doc/gdiot.txt

DOCS-$(CONFIG_HTMLPAGES) += $(HTMLPAGES)
DOCS-$(CONFIG_INFOPAGES) += $(INFOPAGES)
DOCS-$(CONFIG_TXTPAGES)  += $(TXTPAGES)
DOCS = $(DOCS-yes)

all-$(CONFIG_DOC): doc

doc: documentation

documentation: $(DOCS)

ifdef HAVE_MAKEINFO
doc/%.html: TAG = HTML
doc/%.html: doc/%.texi
	$(M)makeinfo --html -I doc -o $@ $<

doc/%.info: TAG = INFO
doc/%.info: doc/%.texi
	$(M)makeinfo --info -I doc -o $@ $<

doc/%.txt: TAG = TXT
doc/%.txt: doc/%.texi
	$(M)makeinfo --plaintext -I doc -o $@ $<
endif

install-doc: install-html install-info install-txt

install-html:

install-info:

ifdef CONFIG_HTMLPAGES
install-progs-$(CONFIG_DOC): install-html

install-html: $(HTMLPAGES)
	$(Q)mkdir -p "$(DOCDIR)"
	$(INSTALL) -m 644 $(HTMLPAGES) "$(DOCDIR)"
endif

ifdef CONFIG_INFOPAGES
install-progs-$(CONFIG_DOC): install-info

install-info: $(INFOPAGES)
	$(Q)mkdir -p "$(INFODIR)"
	$(INSTALL) -m 644 $(INFOPAGES) "$(INFODIR)"
endif

uninstall: uninstall-doc

uninstall-doc: uninstall-html uninstall-info

uninstall-html:
	$(RM) -r "$(DOCDIR)"

uninstall-info:
	$(RM) -r "$(INFODIR)"

clean:: docclean

distclean:: docclean
	$(RM) doc/config.texi

docclean::
	$(RM) $(CLEANSUFFIXES:%=doc/%)
	$(RM) $(TXTPAGES) doc/*.info
	$(RM) -R doc/*.html

-include $(wildcard $(DOCS:%=%.d))

.PHONY: doc documentation
