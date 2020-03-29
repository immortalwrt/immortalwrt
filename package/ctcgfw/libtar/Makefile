####
 #  File: /Makefile
 #  Project: libtar
 #  File Created: Saturday, 7th March 2020 6:57:51 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2020 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=libtar
PKG_VERSION:=1.2.20
PKG_RELEASE:=3

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ElonH/libtar.git
PKG_MIRROR_HASH:=6ba99fe56d83ec6a8f8be0448383f49426a6a9290c3296f3f7d5b2d2c71cdb45
PKG_SOURCE_VERSION:=987a29eaf8c68f8a502dd3880c58e0e1ef99d6a7
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1
PKG_FIXUP:=autoreconf
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

CONFIGURE_ARGS+= --enable-shared=yes --enable-static=no

define Package/libtar
	SECTION:=lib
	CATEGORY:=Libraries
	URL:=https://github.com/ElonH/libtar
	TITLE:=libtar - C library for manipulating tar files
	DEPENDS:=+zlib
endef

define Package/libtar/description
libtar is a library for manipulating tar files from within C programs.
Here are some of its features:

  * Handles both POSIX tar file format and the GNU extensions.
  * API provides functions for easy use, such as tar_extract_all().
  * Also provides functions for more granular use, such as 
    tar_append_regfile().

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/libtar $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
endef

define Package/libtar/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/libtar $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef


$(eval $(call BuildPackage,libtar))
