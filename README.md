# PROJECT IMMORTALWRT
## The Core Source Code of ImmortalWrt
### Welcome to our Telegram Group: [@ctcgfw\_openwrt\_discuss](https://t.me/ctcgfw_openwrt_discuss).
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
sudo apt-get install -y build-essential asciidoc binutils bzip2 gawk gettext git libncurses5-dev libz-dev patch unzip zlib1g-dev lib32gcc1 libc6-dev-i386 subversion flex uglifyjs git-core gcc-multilib g++-multilib p7zip p7zip-full msmtp libssl-dev texinfo libreadline-dev libglib2.0-dev xmlto qemu-utils upx libelf-dev autoconf automake libtool autopoint ccache curl wget vim nano python python3 python-pip python3-pip python-ply python3-ply haveged lrzsz device-tree-compiler scons antlr3 gperf intltool mkisofs rsync
```
#### For mainland China & Ubuntu(16.04+) user, you may run the following command to setup quickly:
```bash
sudo bash -c "bash <(curl -s https://build-scripts.project-openwrt.eu.org/init_build_environment.sh)"
```

<details>
  <summary>opde</summary>

  you can also download and use pre-build container directly:

  ```bash
  docker pull immortalwrt/opde:base
  # docker run --rm -it immortalwrt/opde:base
  ```
</details>

## Clone the source
```bash
git clone -b master --single-branch https://github.com/immortalwrt/immortalwrt && cd immortalwrt
./scripts/feeds update -a && ./scripts/feeds install -a
```

<details>
  <summary>opde</summary>
  
  1. For Linux User:
  ```bash
  git clone -b master --single-branch https://github.com/immortalwrt/immortalwrt && cd immortalwrt
  docker run --rm -it \
    -v $PWD:/openwrt \
    immortalwrt/opde:base zsh
  ./scripts/feeds update -a && ./scripts/feeds install -a
  ```
  
  2. For Windows User:

  openwrt source code can not be cloned into NTFS filesystem(symbol link problem during compilation), 
  but docker volume is fine.

  Create a volume 'immortalwrt' and clone immortalwrt source into volume.

  ```bash
  docker run --rm -it -v immortalwrt:/openwrt immortalwrt/opde:base git clone -b master --single-branch  https://github.com/immortalwrt/immortalwrt .
  ```

  Enter docker container and update feeds

  ```bash
  docker run --rm -it -v immortalwrt:/openwrt immortalwrt/opde:base
  ./scripts/feeds update -a ​&&​ ./scripts/feeds install -a
  ```

  Proxy Support:

  ```bash
  docker run --rm -it \
    -e   all_proxy=http://example.com:1081 \
    -e  http_proxy=http://example.com:1081 \
    -e https_proxy=http://example.com:1081 \
    -e   ALL_PROXY=http://example.com:1081 \
    -e  HTTP_PROXY=http://example.com:1081 \
    -e HTTPS_PROXY=http://example.com:1081 \
    -v $PWD:/openwrt \
    immortalwrt/opde:base zsh
  ```

  > recommand `http` rather `socks5` protocol
  >
  > ip can not be `localhost` or `127.0.0.1`


</details>

## Configure your firmware
```bash
make menuconfig
```

## Make it
```bash
make -j$(nproc) V=s
```

<details>
  <summary>opde</summary>
  For Windows User, binary is still in volume. It can be copied to outside via followed command

  ```bash
  docker run --rm -v <D:\path\to\dir>:/dst -v openwrt:/openwrt -w /dst immortalwrt:base cp /openwrt/bin /dst
  ```
  > make sure `D:\path]to\dir` has been appended in [File Sharing](https://docs.docker.com/docker-for-windows/#file-sharing)

</details>

## Tips
You'd better not use **root** to make it, or you may be not able to use.<br/>
Default login address: 192.168.1.1, username is **root** and password is **password**.

# Contributed
### See [CONTRIBUTED.md](https://github.com/immortalwrt/immortalwrt/blob/master/CONTRIBUTED.md).

# License
### [GNU General Public License v3.0](https://github.com/immortalwrt/immortalwrt/blob/master/LICENSES/GPL-3.0).
