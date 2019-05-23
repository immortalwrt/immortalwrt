# A OpenWRT firmware based on Lean's source
Origin source: [coolsnowwolf/lede](https://github.com/coolsnowwolf/lede)

# How to make it
## OS require:
### Ubuntu 14.04 LTS x86_64 (16.04 LTS is OK)
### At least 2G RAM & 2 CPU CORES
### At least 25G HDD

## Install the necessary software
```bash
sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint
```

## Clone the source
```bash
git clone https://github.com/shell-script/lede-openwrt && cd lede-openwrt
./scripts/feeds update -a && ./scripts/feeds install -a
```

## Configure your firmware
```bash
make menuconfig
```

## Make it
```bash
make -j1 V=s
```

# TIPS
## You'd better not use root to make it, or you may be not able to use it.
## Default username is root and password is password, login address: 192.168.1.1

# License
[GPL v3](https://github.com/shell-script/lede-openwrt/blob/master/LICENSE)<br>
Origin source: [coolsnowwolf/lede](https://github.com/coolsnowwolf/lede)
