#
# Copyright (C) 2021 ImmortalWrt
# <https://immortalwrt.org>
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=naiveproxy
PKG_VERSION:=89.0.4389.72-1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://codeload.github.com/klzgrad/naiveproxy/tar.gz/v$(PKG_VERSION)?
PKG_HASH:=94b1c3f1e63ecab43eb8006a38ed1a76f1e510364ba651abaef0ec575449f687

PKG_LICENSE:=BSD 3-Clause
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=CN_SZTL <cnsztl@immortalwrt.org>

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

AFDO_VER:=89.0.4389.65_rc-r1-merged
AFDO_FILE:=chromeos-chrome-amd64-$(AFDO_VER).afdo.bz2
define Download/AFDO_PROF
	URL:=https://storage.googleapis.com.cnpmjs.org/chromeos-prebuilt/afdo-job/llvm \
	     https://storage.googleapis.com/chromeos-prebuilt/afdo-job/llvm
	URL_FILE:=$(AFDO_FILE)
	FILE:=$(AFDO_FILE)
	HASH:=75a176d57c9b5fb12bff7e9442ddf21226df8aa25de19266f7ceb426eafd3421
endef

CLANG_VER:=llvmorg-12-init-16296-g5e476061-1
CLANG_FILE:=clang-$(CLANG_VER).tgz
define Download/CLANG
	URL:=https://commondatastorage.googleapis.com/chromium-browser-clang/Linux_x64
	URL_FILE:=$(CLANG_FILE)
	FILE:=$(CLANG_FILE)
	HASH:=78729dfab39fb9d52ede2841e408cf505959b26a5d0b086ed7cf4f784f2b393a
endef

GN_VER:=595e3be7c8381d4eeefce62a63ec12bae9ce5140
GN_FILE:=gn-git_revision-$(GN_VER).zip
define Download/GN_TOOL
	URL:=https://chrome-infra-packages.appspot.com/dl/gn/gn/linux-amd64/+
	URL_FILE:=git_revision:$(GN_VER)
	FILE:=$(GN_FILE)
	HASH:=9d61dcab3814868786768c9dfab71e0cd850a61dbad9a049536d2f1aae784fe4
endef

PGO_VER:=4389-1614333145-3fb545b30f8abc065d64d3b5af88abd66c7abb5e
PGO_FILE:=chrome-linux-$(PGO_VER).profdata
define Download/PGO_PROF
	URL:=https://storage.googleapis.com.cnpmjs.org/chromium-optimization-profiles/pgo_profiles \
	     https://storage.googleapis.com/chromium-optimization-profiles/pgo_profiles
	URL_FILE:=$(PGO_FILE)
	FILE:=$(PGO_FILE)
	HASH:=2dafccee5abb30bcc2dc627cfd8af203cac17b522f66e3e4a90301a70d237ce1
endef

define Build/Prepare
	$(call Build/Prepare/Default)

	( \
		cd $(PKG_BUILD_DIR)/src ; \
		bzip2 -cd > "chrome/android/profiles/afdo.prof" < "$(DL_DIR)/$(AFDO_FILE)" ; \
		mkdir -p "chrome/build/pgo_profiles" ; \
		$(CP) "$(DL_DIR)/$(PGO_FILE)" "chrome/build/pgo_profiles" ; \
		mkdir -p "third_party/llvm-build/Release+Asserts" ; \
		$(TAR) -xzf "$(DL_DIR)/$(CLANG_FILE)" -C "third_party/llvm-build/Release+Asserts" ; \
		mkdir -p "gn/out" ; \
		unzip -o "$(DL_DIR)/$(GN_FILE)" -d "gn/out" ; \
	)
endef

define Build/Compile
( \
	cd "$(PKG_BUILD_DIR)/src" ; \
	. ../init_env.sh "$(ARCH)" "$(BOARD)" $(CPU_TYPE) $(CPU_SUBTYPE) "$(TOOLCHAIN_DIR)" ; \
	export naive_flags="$$$${naive_flags} pgo_data_path=\"$(PKG_BUILD_DIR)/src/chrome/build/pgo_profiles/$(PGO_FILE)\"" ; \
	export naive_flags="$$$${naive_flags} $$$${naive_ccache_flags}" ; \
	export OP_STAGING_DIR="$(STAGING_DIR)" ; \
	mkdir -p "out" ; \
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
$(eval $(call Download,PGO_PROF))

$(eval $(call BuildPackage,naiveproxy))
