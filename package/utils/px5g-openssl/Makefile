# SPDX-License-Identifier: GPL-2.0-or-later

include $(TOPDIR)/rules.mk

PKG_NAME:=px5g-openssl
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0-or-later

PKG_BUILD_FLAGS:=no-mips16

include $(INCLUDE_DIR)/package.mk

define Package/px5g-openssl
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Encryption
  TITLE:=X.509 certificate generator (using OpenSSL)
  DEPENDS:=+libopenssl
  PROVIDES:=px5g
  VARIANT:=openssl
endef

define Package/px5g-openssl/description
 Px5g is a tiny standalone X.509 certificate generator.
 It is suitable for creating key files and certificates in DER
 and PEM format for use with stunnel, uhttpd and others.
endef

TARGET_CFLAGS += -Wall -Wextra
TARGET_LDFLAGS += -lcrypto

define Build/Compile
	$(TARGET_CC) $(TARGET_CPPFLAGS) $(TARGET_CFLAGS) \
		-o $(PKG_BUILD_DIR)/px5g px5g-openssl.c $(TARGET_LDFLAGS)
endef

define Package/px5g-openssl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/px5g $(1)/usr/sbin/px5g
endef

$(eval $(call BuildPackage,px5g-openssl))
