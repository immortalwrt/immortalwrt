# PROJECT OPENWRT
## The Source of OpenWrt Modified by CTCGFW
### Welcome to my Telegram Group: [@ctcgfw\_openwrt\_discuss](https://t.me/ctcgfw_openwrt_discuss).

# Tips
You'd better not use **root** to make it, or you may be not able to use.<br/>
Default username is **root** and password is **password**, login address: 192.168.1.1.

# How to make it
## OS require
Ubuntu 18.04 LTS x86\_64 (16.04 LTS is OK)<br/>
At least 2G RAM & 2 CPU Cores<br/>
At least 25G HDD<br/>

## Install the necessary packages 
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
git clone https://github.com/project-openwrt/openwrt -b master --depth 1 && cd openwrt
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

## Contributed
### See [CONTRIBUTED.md](https://github.com/project-openwrt/openwrt/blob/master/CONTRIBUTED.md).

## License
### [GNU General Public License v3.0](https://github.com/project-openwrt/openwrt/blob/master/LICENSE).
