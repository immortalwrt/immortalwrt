#
# Copyright (C) 2020 Project OpenWrt
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=naiveproxy
PKG_VERSION:=86.0.4240.75-1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/klzgrad/naiveproxy/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=91f946e137565115649bc5787788a6889d84e0a20e9878553a832b89c0cb161f
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_LICENSE:=BSD 3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@project-openwrt.eu.org>

PKG_BUILD_DEPENDS:=ncurses/host ninja/host python3/host
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

define Package/naiveproxy/config
	depends on !(arc||armeb||mips||mips64||powerpc)
endef

define Package/naiveproxy
	SECTION:=net
	CATEGORY:=Network
	SUBMENU:=Web Servers/Proxies
	URL:=https://github.com/klzgrad/naiveproxy
	TITLE:=Make a fortune quietly
	DEPENDS:=+libatomic +libnss
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

AFDO_VER:=86.0.4237.0_rc-r1-merged
AFDO_FILE:=chromeos-chrome-amd64-$$(AFDO_VER).afdo.bz2
define Download/AFDO_PROF
	URL:=https://storage.googleapis.com.cnpmjs.org/chromeos-prebuilt/afdo-job/llvm \
	     https://storage.googleapis.com/chromeos-prebuilt/afdo-job/llvm
	URL_FILE:=$$(AFDO_FILE)
	FILE:=$$(URL_FILE)
	HASH:=d5730bd681766bcf4fd22c0e560ad500b2d6eb09164a9b80146cc327a156c98c
endef

CLANG_VER:=llvmorg-12-init-3492-ga1caa302-1
CLANG_FILE:=clang-$$(CLANG_VER).tgz
define Download/CLANG
	URL:=https://commondatastorage.googleapis.com/chromium-browser-clang/Linux_x64
	URL_FILE:=$$(CLANG_FILE)
	FILE:=$$(URL_FILE)
	HASH:=05df154999115386fb390a5ec7e6ca9e28aeae2ea1cbf76435247c0eee5b9ef3
endef

COLON:= :
GN_VER:=git_revision$$(COLON)e327ffdc503815916db2543ec000226a8df45163
GN_FILE:=gn-$$(GN_VER).zip
define Download/GN_TOOL
	URL:=https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+
	URL_FILE:=$$(GN_VER)
	FILE:=$$(GN_FILE)
	HASH:=6d141b84a541e31ed7d3cd4d938987fdfa7988d5081d3089bc1509a43d6b742f
endef

define Build/Prepare
	$(call Build/Prepare/Default)

	( \
		cd $(PKG_BUILD_DIR)/src ; \
		bzip2 -cd > "chrome/android/profiles/afdo.prof" < $(DL_DIR)/$(AFDO_FILE) ; \
		mkdir -p "third_party/llvm-build/Release+Asserts" ; \
		$(TAR) -xzf $(DL_DIR)/$(CLANG_FILE) -C "third_party/llvm-build/Release+Asserts" ; \
		mkdir -p "gn/out" ; \
		unzip -o $(DL_DIR)/$(GN_FILE) -d "gn/out" ; \
	)
endef

define Build/Compile
( \
	cd $(PKG_BUILD_DIR)/src ; \
	. ../init_env.sh "$(ARCH)" "$(BOARD)" $(CPU_TYPE) $(CPU_SUBTYPE) "$(TOOLCHAIN_DIR)" ; \
	export naive_flags="$$$${naive_flags} $$$${naive_ccache_flags}" ; \
	export OP_STAGING_DIR="$(STAGING_DIR)" ; \
	mkdir -p out ; \
	./gn/out/gn gen "out/Release" --args="$$$${naive_flags}" --script-executable="$(STAGING_DIR_HOSTPKG)/bin/python3" ; \
	$(STAGING_DIR_HOSTPKG)/bin/ninja -C "out/Release" naive ; \
)
endef

define Package/naiveproxy/conffiles
/etc/config/naiveproxy
endef

define Package/naiveproxy/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/out/Release/naive $(1)/usr/bin/naive
	$(INSTALL_DIR) $(1)/etc/config $(1)/etc/init.d
	$(INSTALL_CONF) $(CURDIR)/files/naiveproxy.config $(1)/etc/config/naiveproxy
	$(INSTALL_BIN) $(CURDIR)/files/naiveproxy.init $(1)/etc/init.d/naiveproxy
endef

$(eval $(call Download,AFDO_PROF))
$(eval $(call Download,CLANG))
$(eval $(call Download,GN_TOOL))

$(eval $(call BuildPackage,naiveproxy))
