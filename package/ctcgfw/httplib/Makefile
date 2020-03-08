####
 #  File: /httplib/Makefile
 #  Project: ootoc-OpenWRT
 #  File Created: Sunday, 8th March 2020 10:45:13 pm
 #  Author: ElonH[EH](elonhhuang@gmail.com)
 #  License: GNU General Public License v3.0 or later(http://www.gnu.org/licenses/gpl-3.0-standalone.html)
 #  Copyright (C) 2020 [ElonH]
####

include $(TOPDIR)/rules.mk

PKG_NAME:=httplib
PKG_VERSION:=0.5.7
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/yhirose/cpp-httplib.git
PKG_MIRROR_HASH:=7b04f20cbb27f6e11120c9b185ee6b425d464cd0566b03b907b9881f1642594d
PKG_SOURCE_VERSION:=bf7700d1924e12a27faa5d346a5b8a9f8e163b61
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=ElonH <elonhhuang@gmail.com>

HOST_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/host-build.mk
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/cmake.mk

define Package/httplib
	SECTION:=lib
	CATEGORY:=Libraries
	VARIANT:=lib
	URL:=https://github.com/yhirose/cpp-httplib
	TITLE:=A C++11 single-file header-only cross platform HTTP/HTTPS library.
endef

define Package/httplib/description
	A C++11 single-file header-only cross platform HTTP/HTTPS library.

	It's extremely easy to setup. Just include `httplib.h` file in your code!
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,httplib))
