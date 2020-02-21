#
# Copyright (C) 2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=you-get
PKG_VERSION:=0.4.1403
PKG_RELEASE:=1

PYPI_NAME:=you-get
PKG_HASH:=944daa105d81ac5fb25cec52d05db3223966e12b9aa20d5a1e746cd67b48aca4

PKG_MAINTAINER:=[CTCGFW]Project OpenWrt
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE

include $(TOPDIR)/feeds/packages/lang/python/pypi.mk
include $(INCLUDE_DIR)/package.mk
include $(TOPDIR)/feeds/packages/lang/python/python3-package.mk

define Package/you-get
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Dumb downloader that scrapes the web
  URL:=https://you-get.org
  DEPENDS:= \
    +ca-certificates \
    +ffmpeg \
    +python3 \
    +python3-email \
    +python3-xml \
    +python3-codecs \
    +python3-ctypes \
    +python3-setuptools
  VARIANT:=python3
endef

define Package/you-get/description
  You-Get is a tiny command-line utility to download
  media contents (videos, audios, images) from the Web,
  in case there is no other handy way to do it.
endef

$(eval $(call Py3Package,you-get))
$(eval $(call BuildPackage,you-get))
$(eval $(call BuildPackage,you-get-src))

