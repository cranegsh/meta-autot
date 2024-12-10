# SPDX-License-Identifier: LGPL-2.1-only
#
# Makefile - global Makefile for all components
#
# Copyright (C) 2001-2020 PEAK System-Technik GmbH <www.peak-system.com>
#
# Contact:       <linux@peak-system.com>
# Maintainer:    Stephane Grosjean <s.grosjean@peak-system.com>
# Contributions: Klaus Hitschler <klaus.hitschler@gmx.de>
#                Pablo Yanez Trujillo <yanez@pse.de> cross-compile
#

# CROSS-COMPILING SETTINGS
#
# You need a cross-compiler. You can build one with crosstool
# http://www.kegel.com/crosstool/current/doc/crosstool-howto.html
#
# These variables work with the toolchains created by crosstool

# defines the architecture. For example 'arm' for an ARM system
#export ARCH=arm

# the path and prefix of the cross-compiler
#export CROSS_COMPILE=/home/yanez/matrix500/arm-9tdmi-linux-gnu/bin/arm-9tdmi-linux-gnu-

define make-pcanbasic
@$(MAKE) -C libpcanbasic $1
endef

all:
#	@$(MAKE) -C lib $1
#	@$(MAKE) -C libpcanbasic $1
	@$(MAKE) -C lib CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"
	@$(MAKE) -C libpcanbasic CC="$(CC)" CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)"

clean:
	@$(MAKE) -C lib clean
	$(call make-pcanbasic, clean)
