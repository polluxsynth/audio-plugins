#!/usr/bin/make -f
# Makefile for DISTRHO Plugins #
# ---------------------------- #
# Created by falkTX
# Modified for MiMi-d by Ricard Wanderlof 2024

# --------------------------------------------------------------
# Project name, used for binaries

NAME = MiMi-d

# --------------------------------------------------------------
# Files to build

FILES_DSP = \
	MiMi-d.cpp

FILES_UI =

# --------------------------------------------------------------
# Do some magic

#UI_TYPE = generic
#FILE_BROWSER_DISABLED = true
include ../../dpf/Makefile.plugins.mk

# --------------------------------------------------------------
# Extra flags

ifneq ($(WASM),true)
BASE_FLAGS += -pthread
LINK_FLAGS += -pthread
endif

# --------------------------------------------------------------
# Enable all possible plugin types

#all: jack dssi lv2_sep vst2 vst3 clap
all: lv2_sep

# --------------------------------------------------------------
