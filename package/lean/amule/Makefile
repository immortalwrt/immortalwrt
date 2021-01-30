# Copyright (C) 2021 CTCGFW Project-OpenWrt
# <https://project-openwrt.eu.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=aMule
PKG_VERSION:=2.3.2
PKG_RELEASE:=5

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/amule-project/amule.git
PKG_SOURCE_DATE:2021-01-30
PKG_SOURCE_VERSION:=c3a1018329633234ac941055b64a43b9e2df7fad
PKG_MIRROR_HASH:=f03b60e909346c4d984aeb9cf3a115b5b43fa6158ee01ed41b99e7217b436bb7

PKG_CONFIG_DEPEDS:= CONFIG_AMULE_CRYPTOPP_STATIC_LINKING

PKG_FIXUP:=autoreconf
PKG_BUILD_DEPENDS:=libcryptopp libgd
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/nls.mk

define Package/amule
  SUBMENU:=P2P
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A multi-platform eMule-like ed2k client
  URL:=http://www.amule.org/
  DEPENDS:=+libpng +libpthread +libncurses +libreadline +libwxbase +libupnp +libbfd \
	$(ICONV_DEPENDS) $(INTL_DEPENDS) +!AMULE_CRYPTOPP_STATIC_LINKING:libcryptopp
endef

define Package/amule/config
	config AMULE_CRYPTOPP_STATIC_LINKING
		bool "Link libcryptopp statically"
		default n
endef

TARGET_LDFLAGS+= -liconv

CONFIGURE_ARGS+= \
	--disable-static \
	--disable-rpath \
	--with-gnu-ld \
	--disable-ccache \
	--disable-optimize \
	--disable-profile \
	--disable-monolithic \
	--enable-amule-daemon \
	--enable-amulecmd \
	--enable-webserver \
	--disable-amule-gui \
	--disable-cas \
	--disable-wxcas \
	--disable-ed2k \
	--disable-alc \
	--disable-alcc \
	--disable-fileview \
	--disable-plasmamule \
	--without-wxdebug \
	--enable-dlp \
	--enable-upnp \
	--with-zlib="$(STAGING_DIR)/usr" \
	--with-gdlib-prefix="$(STAGING_DIR)/usr" \
	--with-libpng-prefix="$(STAGING_DIR)/usr" \
	--with-wx-prefix="$(STAGING_DIR)/usr" \
	--with-crypto-prefix="$(STAGING_DIR)/usr" \
	--with-libiconv-prefix="$(ICONV_PREFIX)" \
	--with-libintl-prefix="$(INTL_PREFIX)" \
	--with-libupnp-prefix="$(STAGING_DIR)/usr" \
	--without-x \
	--disable-debug 

MAKE_FLAGS += HOSTCC="$(HOSTCC)"

define Build/Configure
	cd $(PKG_BUILD_DIR); $(SHELL) autogen.sh
	$(call Build/Configure/Default)
ifeq ($(CONFIG_AMULE_CRYPTOPP_STATIC_LINKING),y)
	$(SED) 's;^CRYPTOPP_LIBS.*;CRYPTOPP_LIBS \= "$(STAGING_DIR)/usr/lib/libcryptopp.a";g' \
		$(PKG_BUILD_DIR)/src/Makefile
endif
endef

define Package/amule/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/amulecmd $(1)/usr/bin/amulecmd
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/amuled $(1)/usr/bin/amuled
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/amuleweb $(1)/usr/bin/amuleweb

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/amule $(1)/usr/share/
endef

$(eval $(call BuildPackage,amule))
