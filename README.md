# A OpenWRT firmware based on Lean's source
### Welcome to my Telegram Group: [@ctcgfw\_openwrt\_discuss](https://t.me/ctcgfw_openwrt_discuss).

# Tips
You'd better not use **root** to make it, or you may be not able to use.<br/>
Default username is **root** and password is **password**, login address: 192.168.1.1.

# How to make it
## OS require
Ubuntu 14.04 LTS x86\_64 (16.04 LTS is OK)<br>
At least 2G RAM & 2 CPU Cores<br>
At least 25G HDD<br>

## Install the necessary packages 
```bash
sudo apt-get -y install build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib g++-multilib  p7zip p7zip-full msmtp libssl-dev texinfo libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint
```

## Clone the source
```bash
git clone https://github.com/project-openwrt/lede && cd lede
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

# Origin source
### Based on: [coolsnowwolf/lede](https://github.com/coolsnowwolf/lede).<br/>

Package jsda: [jsda/packages2](https://github.com/jsda/packages2).<br/>
luci-theme-netgearv2 source: [tracemouse/luci-theme-netgear](https://github.com/tracemouse/luci-theme-netgear).<br/>
luci-app-serverchan source: [zxlhhyccc/luci-app-serverchan](https://github.com/zxlhhyccc/luci-app-serverchan).<br/>
OpenAppFilter source: [destan19/OpenAppFilter](https://github.com/destan19/OpenAppFilter).<br/>
luci-app-smartinfo source: [animefansxj/luci-app-smartinfo](https://github.com/animefansxj/luci-app-smartinfo).<br/>
luci-app-koolproxy source: [Baozisoftware/luci-app-koolproxy](https://github.com/Baozisoftware/luci-app-koolproxy).<br/>
luci-app-k3screenctrl source: [Hill-98/luci-app-k3screenctrl](https://github.com/Hill-98/luci-app-k3screenctrl).<br/>
luci-app-smstool source: [f8q8/luci-app-smstool-master](https://github.com/f8q8/luci-app-smstool-master).<br/>
luci-app-airwhu source: [KyleRicardo/luci-app-airwhu](https://github.com/KyleRicardo/luci-app-airwhu).<br/>
Package Lienol: [Lienol/openwrt-package](https://github.com/Lienol/openwrt-package).<br/>
luci-app-bbr-mod source: [ntlf9t/luci-app-bbr-mod](https://github.com/ntlf9t/luci-app-bbr-mod).<br/>
Package Openwrt-BBR: [anshi233/Openwrt-BBR](https://github.com/anshi233/Openwrt-BBR).<br/>
simple-obfs source: [aa65535/openwrt-simple-obfs](https://github.com/aa65535/openwrt-simple-obfs).<br/>
v2ray-plugin source: [honwen/openwrt-v2ray-plugin](https://github.com/honwen/openwrt-v2ray-plugin).<br/>
Package zxlhhyccc: [zxlhhyccc/MY-lede](https://github.com/zxlhhyccc/MY-lede).

# License
### [GPL v3](https://www.gnu.org/licenses/gpl-3.0.html).
