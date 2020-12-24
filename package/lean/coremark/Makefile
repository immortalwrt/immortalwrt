#
# Copyright (C) 2018 Lim Guo Wei
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=coremark
PKG_SOURCE_DATE:=2020-09-16
PKG_SOURCE_VERSION:=41537ea30b0104438b4ff993e7d349af26900acf
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_DATE).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/eembc/coremark/tar.gz/$(PKG_SOURCE_VERSION)?
PKG_HASH:=ebbaa7463084b3261438b9b378c1968532156f7fb90bf3f94f45f0b0b817e181
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_SOURCE_VERSION)

PKG_MAINTAINER:=Lim Guo Wei <limguowei@gmail.com>
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE.md

PKG_USE_MIPS16:=0

include $(INCLUDE_DIR)/package.mk

define Package/coremark
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=CoreMark Embedded Microprocessor Benchmark
  URL:=https://github.com/eembc/coremark
endef

define Package/coremark/description
  Embedded Microprocessor Benchmark
endef

DIR_ARCH:=linux$(if $(CONFIG_ARCH_64BIT),64)

define Package/coremark/config
	config COREMARK_OPTIMIZE_O3
		bool "Use all optimizations (-O3)"
		depends on PACKAGE_coremark
		default y
		help
			This enables additional optmizations using the -O3 compilation flag.

	config COREMARK_ENABLE_MULTITHREADING
		bool "Enable multithreading support"
		depends on PACKAGE_coremark
		default y
		help
			This enables multithreading support

	config COREMARK_NUMBER_OF_THREADS
		int "Number of threads"
		depends on COREMARK_ENABLE_MULTITHREADING
		default 16
		help
			Number of threads to run in parallel
endef

TARGET_CFLAGS += -flto

ifeq ($(CONFIG_COREMARK_OPTIMIZE_O3),y)
	TARGET_CFLAGS := $(filter-out -O%,$(TARGET_CFLAGS)) -O3
endif

ifeq ($(CONFIG_COREMARK_ENABLE_MULTITHREADING),y)
	EXTRA_CFLAGS := -DMULTITHREAD=$(CONFIG_COREMARK_NUMBER_OF_THREADS) -DUSE_PTHREAD
endif

define Build/Compile
	$(SED) 's|EXE = .exe|EXE =|' $(PKG_BUILD_DIR)/$(DIR_ARCH)/core_portme.mak
	mkdir $(PKG_BUILD_DIR)/$(ARCH)
	$(CP) -r $(PKG_BUILD_DIR)/$(DIR_ARCH)/* $(PKG_BUILD_DIR)/$(ARCH)
	$(MAKE) -C $(PKG_BUILD_DIR) PORT_DIR=$(ARCH) $(MAKE_FLAGS) \
		PORT_CFLAGS="$(TARGET_CFLAGS)" XCFLAGS="$(EXTRA_CFLAGS)" compile
endef

define Package/coremark/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/coremark $(1)/bin/
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./coremark.sh $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./coremark $(1)/etc/uci-defaults/xxx-coremark
endef

define Package/coremark/postinst
#!/bin/sh
[ -n "$${IPKG_INSTROOT}" ] || sed -i '/coremark/d' /etc/crontabs/root
[ -n "$${IPKG_INSTROOT}" ] || echo "0 4 * * * /etc/coremark.sh" >> /etc/crontabs/root
[ -n "$${IPKG_INSTROOT}" ] || crontab /etc/crontabs/root
endef

$(eval $(call BuildPackage,coremark))
