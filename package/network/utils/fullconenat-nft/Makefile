# SPDX-License-Identifier: GPL-2.0-only
# Copyright (c) 2018 Chion Tang <tech@chionlab.moe>
#   Original xt_FULLCONENAT and related iptables extension author
# Copyright (c) 2019-2022 GitHub/llccd Twitter/@gNodeB
#   Added IPv6 support for xt_FULLCONENAT and ip6tables extension
#   Ported to recent kernel versions
# Copyright (c) 2022 Syrone Wong <wong.syrone@gmail.com>
#   Massively rewrite the whole module, split the original code into library and nftables 'fullcone' expression module

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fullconenat-nft
PKG_RELEASE:=$(AUTORELEASE)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/fullcone-nat-nftables/nft-fullcone.git
PKG_SOURCE_DATE:=2022-06-21
PKG_SOURCE_VERSION:=0d3e2bedf939fb380dc32d44d0767d283671637a
PKG_MIRROR_HASH:=66467899fbeaf52fbfb4a6d108bd7e866c115cbfea5d50420d274a4d4b12b8e3

PKG_LICENSE:=GPL-2.0-only
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Syrone Wong <wong.syrone@gmail.com>

include $(INCLUDE_DIR)/package.mk

define KernelPackage/nft-fullcone
  SUBMENU:=Netfilter Extensions
  DEPENDS:=+kmod-nft-nat
  TITLE:=nftables fullcone expression support
  FILES:= $(PKG_BUILD_DIR)/src/nft_fullcone.ko
  KCONFIG:= \
    CONFIG_NF_CONNTRACK_EVENTS=y \
    CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y
  AUTOLOAD:=$(call AutoProbe,nft_fullcone)
endef

define KernelPackage/nft-fullcone/Description
  Kernel module adds the fullcone expression that you can use
  to perform NAT in the RFC3489-compatible full cone SNAT flavour.
  Currently only UDP traffic is supported for full-cone NAT.
  For other protos FULLCONENAT is equivalent to MASQUERADE.
endef

define Build/Compile
	+$(KERNEL_MAKE) M="$(PKG_BUILD_DIR)/src" modules
endef

$(eval $(call KernelPackage,nft-fullcone))
