# PROJECT OPENWRT
## The Source Code of OpenWrt Modified by CTCGFW
### Welcome to my Telegram Group: [@ctcgfw\_openwrt\_discuss](https://t.me/ctcgfw_openwrt_discuss).
- - -

# How to make it
## Minimum requirements
Linux with case sensitive<br/>
2G DDR2 RAM<br/>
2 CPU Cores (AMD64, 1.4Ghz)<br/>
25G disk space left<br/>
Has access to both ChinaNet & Internet

## Install the necessary packages (for Ubuntu user)
```bash
sudo apt-get update -y
sudo apt-get full-upgrade -y
sudo apt-get install -y build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib g++-multilib p7zip p7zip-full msmtp libssl-dev texinfo libreadline-dev libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint ccache curl wget vim nano python python3 python-pip python3-pip python-ply python3-ply haveged lrzsz device-tree-compiler scons
```
#### For mainland China & Ubuntu(16.04+) user, you may run the following command to setup quickly:
```bash
sudo bash -c "bash <(curl -s https://build-scripts.project-openwrt.eu.org/init_build_environment.sh)"
```

## Clone the source
```bash
git clone https://github.com/project-openwrt/openwrt && cd openwrt
./scripts/feeds update -a && ./scripts/feeds install -a
```
#### For developer, you may use `dev` branch instead of `master`.

## Configure your firmware
```bash
make menuconfig
```

## Make it
```bash
make -j1 V=s
```

## Tips
You'd better not use **root** to make it, or you may be not able to use.<br/>
Default login address: 192.168.1.1, username is **root** and password is **password**.

# Contributed
### See [CONTRIBUTED.md](https://github.com/project-openwrt/openwrt/blob/master/CONTRIBUTED.md).

# License
### [GNU General Public License v3.0](https://github.com/project-openwrt/openwrt/blob/master/LICENSE).
