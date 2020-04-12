include $(TOPDIR)/rules.mk

PKG_NAME:=koolproxy
PKG_VERSION:=3.8.4
PKG_RELEASE:=1

PKG_MAINTAINER:=panda-mute <wxuzju@gmail.com>
PKG_LICENSE:=GPLv3
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_PARALLEL:=1

include $(INCLUDE_DIR)/package.mk

define Package/koolproxy
  SECTION:=net
  CATEGORY:=Network
  TITLE:=A powerful ad remove software
  DEPENDS:=@!aarch64 +openssl-util +ipset +dnsmasq-full +@BUSYBOX_CONFIG_DIFF +iptables-mod-nat-extra
endef

define Package/koolproxy/description
  A powerful ad remove software.
endef

define Build/Prepare
endef

define Build/Compile
endef

define Build/Install
endef

define Package/koolproxy/install
ifeq ($(ARCH),mipsel)
	$(INSTALL_BIN) ${CURDIR}/files/bin/mipsel $(1)/usr/share/koolproxy/koolproxy
endif
ifeq ($(ARCH),mips)
	$(INSTALL_BIN) ${CURDIR}/files/bin/mips $(1)/usr/share/koolproxy/koolproxy
endif
ifeq ($(ARCH),i386)
	$(INSTALL_BIN) ${CURDIR}/files/bin/i386 $(1)/usr/share/koolproxy/koolproxy
endif
ifeq ($(ARCH),x86_64)
	$(INSTALL_BIN) ${CURDIR}/files/bin/x86_64 $(1)/usr/share/koolproxy/koolproxy
endif
ifeq ($(ARCH),arm)
	$(INSTALL_BIN) ${CURDIR}/files/bin/arm $(1)/usr/share/koolproxy/koolproxy
endif
endef

$(eval $(call BuildPackage,koolproxy))

