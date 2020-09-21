#
# Copyright (C) 2020 Project OpenWrt
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=naiveproxy
PKG_VERSION:=85.0.4183.83-3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/klzgrad/naiveproxy/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=66a8f275a9aa0fa04649062fb141a10add9bd76855f32ee65e6ee60a7fc4649c
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=BSD 3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_DEPENDS:=ninja/host python3/host
PKG_USE_MIPS16:=0
PKG_BUILD_PARALLEL:=1

ifneq ($(CONFIG_CPU_TYPE)," ")
	CPU_TYPE:=$(word 1, $(subst +," ,$(CONFIG_CPU_TYPE)))
	CPU_SUBTYPE:=$(word 2, $(subst +, ",$(CONFIG_CPU_TYPE)))
	ifeq ($(CPU_SUBTYPE),)
		CPU_SUBTYPE:=""
	endif
else
	CPU_TYPE:=""
	CPU_SUBTYPE:=""
endif

include $(INCLUDE_DIR)/package.mk

define Package/naiveproxy
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	URL:=https://github.com/klzgrad/naiveproxy
	TITLE:=Make a fortune quietly
	DEPENDS:=@!(arc||armeb||powerpc) +libatomic +libnss
endef

define Package/naiveproxy/description
	NaïveProxy uses Chrome's network stack to camouflage traffic with strong
	censorship resistance and low detectability. Reusing Chrome's stack also
	ensures best practices in performance and security.
endef

ifneq ($(CONFIG_CCACHE),)
	export CCACHE_SLOPPINESS=time_macros
	export CCACHE_BASEDIR=$(PKG_BUILD_DIR)/src
	export CCACHE_CPP2=yes
	export naive_ccache_flags=cc_wrapper="ccache"
endif

define Build/Compile
( \
	cd $(PKG_BUILD_DIR) ; \
	./tools/import-upstream.sh ; \
	. ./init_env.sh "$(ARCH)" "$(BOARD)" $(CPU_TYPE) $(CPU_SUBTYPE) "$(TOOLCHAIN_DIR)" "$(DL_DIR)"; \
	export naive_flags="$$$${naive_flags} $$$${naive_ccache_flags}" ; \
	export OP_STAGING_DIR="$(STAGING_DIR)" ; \
	mkdir -p out ; \
	./gn/out/gn gen "out/Release" --args="$$$${naive_flags}" --script-executable="$(STAGING_DIR_HOSTPKG)/bin/python3" ; \
	$(STAGING_DIR_HOSTPKG)/bin/ninja -C "out/Release" naive ; \
)
endef

define Package/naiveproxy/conffiles
/etc/chromium/policies/managed/proxy.json
/etc/config/naiveproxy
endef

define Package/naiveproxy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/out/Release/naive $(1)/usr/bin/naive
	$(INSTALL_DIR) $(1)/etc/chromium/policies/managed
	$(INSTALL_CONF) $(CURDIR)/files/proxy.json $(1)/etc/chromium/policies/managed/proxy.json
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_CONF) $(CURDIR)/files/naiveproxy.config $(1)/etc/config/naiveproxy
	$(INSTALL_BIN) $(CURDIR)/files/naiveproxy.init $(1)/etc/init.d/naiveproxy
endef

$(eval $(call BuildPackage,naiveproxy))
