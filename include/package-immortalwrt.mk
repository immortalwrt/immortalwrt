# SPDX-License-Identifier: GPL-3.0-or-later
#
# Copyright (C) 2019-2021 ImmortalWrt.org

PKG_RELEASE ?= $(AUTORELESE)

ifndef PKG_SOURCE_VERSION
  PKG_SOURCE ?= $(PKG_NAME)-$(PKG_VERSION).tar.gz
endif

ifeq ($(PKG_SOURCE_PROTO),git)
  ifdef PKG_SOURCE_REPO
    PKG_SOURCE_URL:= \
      https://ghproxy.com/https://github.com/$(PKG_SOURCE_REPO).git \
      https://github.com.cnpmjs.org/$(PKG_SOURCE_REPO).git \
      https://pd.zwc365.com/seturl/https://github.com/$(PKG_SOURCE_REPO).git \
      https://hub.fastgit.org/$(PKG_SOURCE_REPO).git \
      https://github.com/$(PKG_SOURCE_REPO).git
  endif
endif

ifneq ($(filter $(PKG_BUILD_DEPENDS),golang/host),)
  GO_PKG_LDFLAGS ?= -s -w
  PKG_USE_MIPS16 ?= 0
endif

HOST_BUILD_PARALLEL ?= 1
PKG_BUILD_PARALLEL ?= 1

include $(INCLUDE_DIR)/package.mk
