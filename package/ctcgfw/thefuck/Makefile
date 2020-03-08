#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=thefuck
PKG_VERSION:=3.29
PKG_RELEASE:=1

PYPI_NAME:=thefuck
PKG_HASH:=7b907b6ef6863cc0d3e9bb3e573054547a60f89572250b767ccadb317d3c8297

PKG_MAINTAINER:=[CTCGFW]Project OpenWrt
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(TOPDIR)/feeds/packages/lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/python/python3-package.mk

define Package/thefuck
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Magnificent app which corrects your previous console command.
  URL:=https://github.com/nvbn/thefuck
  DEPENDS:= \
    +python3 \
    +python3-codecs \
    +python3-ctypes \
    +python3-setuptools
  VARIANT:=python3
endef

define Package/thefuck/description
  The Fuck is a magnificent app, inspired by a @liamosaur tweet, that corrects errors in previous console commands.
endef

$(eval $(call Py3Package,thefuck))
$(eval $(call BuildPackage,thefuck))
$(eval $(call BuildPackage,thefuck-src))

