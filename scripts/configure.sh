#!/bin/bash

set -e

echo "开始配置AR9331编译..."

# 检查当前目录
if [ -f "feeds.conf.default" ]; then
    echo "当前在openwrt目录中"
    OPENWRT_DIR="."
elif [ -f "openwrt/feeds.conf.default" ]; then
    echo "当前在仓库根目录中，切换到openwrt目录"
    cd openwrt
    OPENWRT_DIR="."
else
    echo "错误：找不到openwrt目录"
    exit 1
fi

# 清理之前的配置
echo "清理旧配置..."
rm -f .config .config.old

# 更新feeds
echo "更新feeds..."
./scripts/feeds update -a
./scripts/feeds install -a

# 取消luci相关包，避免依赖问题
echo "清理不需要的包..."
for pkg in luci luci-ssl luci-theme-bootstrap luci-app-firewall luci-proto-ipv6 ipv6helper; do
    ./scripts/feeds uninstall $pkg 2>/dev/null || true
done

# 首先设置目标架构
echo "设置目标架构为ar71xx/tiny..."
cat > .config << 'EOF'
CONFIG_TARGET_ar71xx=y
CONFIG_TARGET_ar71xx_tiny=y
CONFIG_TARGET_ar71xx_tiny_DEVICE_generic=y
CONFIG_TARGET_ar71xx_tiny_DEVICE_generic_ar9331=y
CONFIG_TARGET_BOARD="ar71xx"
CONFIG_TARGET_SUBTARGET="tiny"
CONFIG_TARGET_ARCH_PACKAGES="mips_24kc"
EOF

# 生成默认配置
echo "生成默认配置..."
make defconfig

# 现在添加我们的配置
echo "添加自定义配置..."
cat >> .config << 'EOF'

# 镜像配置
CONFIG_TARGET_ROOTFS_SQUASHFS=y
CONFIG_TARGET_IMAGES_GZIP=y
CONFIG_TARGET_ROOTFS_PARTSIZE=48

# 内核配置
CONFIG_KERNEL_BUILD_USER="AR9331-Custom"
CONFIG_KERNEL_BUILD_DOMAIN="github.com"
CONFIG_KERNEL_DEBUG_INFO=n
CONFIG_KERNEL_DEBUG_KERNEL=n

# 基本内核模块
CONFIG_PACKAGE_kmod-ath9k=y
CONFIG_PACKAGE_kmod-ath9k-common=y
CONFIG_PACKAGE_kmod-ath=y
CONFIG_PACKAGE_kmod-gpio-button-hotplug=y
CONFIG_PACKAGE_kmod-leds-gpio=y
CONFIG_PACKAGE_kmod-ledtrig-default-on=y
CONFIG_PACKAGE_kmod-ledtrig-timer=y
CONFIG_PACKAGE_kmod-nls-base=y

# 基础系统
CONFIG_PACKAGE_base-files=y
CONFIG_PACKAGE_busybox=y
CONFIG_PACKAGE_dropbear=y
CONFIG_PACKAGE_fstools=y
CONFIG_PACKAGE_fwtool=y
CONFIG_PACKAGE_jshn=y
CONFIG_PACKAGE_jsonfilter=y
CONFIG_PACKAGE_libc=y
CONFIG_PACKAGE_libgcc=y
CONFIG_PACKAGE_libustream-wolfssl=y
CONFIG_PACKAGE_logd=y
CONFIG_PACKAGE_mtd=y
CONFIG_PACKAGE_netifd=y
CONFIG_PACKAGE_opkg=y
CONFIG_PACKAGE_procd=y
CONFIG_PACKAGE_swconfig=y
CONFIG_PACKAGE_ubox=y
CONFIG_PACKAGE_ubus=y
CONFIG_PACKAGE_ubusd=y
CONFIG_PACKAGE_uci=y
CONFIG_PACKAGE_urandom-seed=y
CONFIG_PACKAGE_usign=y

# 网络和防火墙
CONFIG_PACKAGE_dnsmasq=y
# CONFIG_PACKAGE_dnsmasq-full is not set  # 避免冲突
CONFIG_PACKAGE_firewall=y
CONFIG_PACKAGE_iptables=y
CONFIG_PACKAGE_iptables-mod-conntrack-extra=y
CONFIG_PACKAGE_iptables-mod-ipopt=y
CONFIG_PACKAGE_kmod-ipt-offload=y
CONFIG_PACKAGE_odhcp6c=y
CONFIG_PACKAGE_odhcpd-ipv6only=y

# 无线
CONFIG_PACKAGE_w
