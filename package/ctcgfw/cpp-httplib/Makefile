####
 #  File: /httplib/Makefile
 #  Project: ootoc-OpenWRT
 #  File Created: Sunday, 8th March 2020 10:45:13 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2020 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=cpp-httplib
PKG_VERSION:=0.5.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/yhirose/cpp-httplib/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=27b7f6346bdeb1ead9d17bd7cea89d9ad491f50f0479081053cc6e5742a89e64

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

HOST_BUILD_PARALLEL:=1
PKG_BUILD_PARALLEL:=1
CMAKE_INSTALL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/cpp-httplib
	SECTION:=lib
	CATEGORY:=Libraries
	VARIANT:=lib
	URL:=https://github.com/yhirose/cpp-httplib
	TITLE:=A C++11 single-file header-only cross platform HTTP/HTTPS library.
endef

define Package/cpp-httplib/description
	A C++11 single-file header-only cross platform HTTP/HTTPS library.
	It's extremely easy to setup. Just include `httplib.h` file in your code!
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,cpp-httplib))
