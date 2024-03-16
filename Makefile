#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
#
# Modified by REIS0

PLUGIN=mimid
PLUGIN_NAME=MiMi-d
WITH_LTO=true
NOOPT=false

include dpf/Makefile.base.mk

all: dgl plugins gen

# --------------------------------------------------------------

PREFIX ?= /usr/local
DESTDIR ?=

# --------------------------------------------------------------

dgl:
ifeq ($(HAVE_OPENGL),true)
	$(MAKE) -C dpf/dgl opengl
endif

plugins: dgl
	$(MAKE) WITH_LTO=$(WITH_LTO) NOOPT=$(NOOPT) all -C plugins/$(PLUGIN)

ifneq ($(CROSS_COMPILING),true)
gen: plugins dpf/utils/lv2_ttl_generator
	@$(CURDIR)/dpf/utils/generate-ttl.sh
ifeq ($(MACOS),true)
	@$(CURDIR)/dpf/utils/generate-vst-bundles.sh
endif

dpf/utils/lv2_ttl_generator:
	$(MAKE) -C dpf/utils/lv2-ttl-generator
else
gen:
endif

# --------------------------------------------------------------

clean:
	$(MAKE) clean -C dpf/dgl
	$(MAKE) clean -C dpf/utils/lv2-ttl-generator
	$(MAKE) clean -C plugins/$(PLUGIN)
	rm -rf bin build

# --------------------------------------------------------------

# install target just installs, it doesn't attempt to build anything
install:
	install -d $(DESTDIR)$(PREFIX)/lib/lv2/$(PLUGIN_NAME).lv2
	install -m 755 bin/$(PLUGIN_NAME).lv2/*.so $(DESTDIR)$(PREFIX)/lib/lv2/$(PLUGIN_NAME).lv2
	install -m 644 bin/$(PLUGIN_NAME).lv2/*.ttl $(DESTDIR)$(PREFIX)/lib/lv2/$(PLUGIN_NAME).lv2

# --------------------------------------------------------------

.PHONY: plugins
