<img src="https://avatars.githubusercontent.com/u/53193414?s=200&v=4" alt="logo" width="200" height="200" align="right">

# Project ImmortalWrt

ImmortalWrt is a fork of [OpenWrt](https://openwrt.org), with more packages ported, more devices supported, better performance, and special optimizations for mainland China users.<br/>
Compared the official one, we allow to use hacks or non-upstreamable patches / modifications to achieve our purpose. Source from anywhere.

Default login address: http://192.168.1.1 or http://immortalwrt.lan, username: __root__, password: __password__.

## Development
To build your own firmware you need a GNU/Linux, BSD or MacOSX system (case sensitive filesystem required). Cygwin is unsupported because of the lack of a case sensitive file system.<br/>

  ### Requirements
  To build with this project, Ubuntu 18.04 LTS is preferred. And you need use the CPU based on AMD64 architecture, with at least 4GB RAM and 25 GB available disk space. Make sure the __Internet__ is accessible.

  The following tools are needed to compile ImmortalWrt, the package names vary between distributions.

  - Here is an example for Ubuntu users:<br/>
    - Method 1:
      <details>
        <summary>Setup dependencies via APT</summary>

        ```bash
        sudo apt update -y
        sudo apt full-upgrade -y
        sudo apt install -y ack antlr3 asciidoc autoconf automake autopoint binutils bison build-essential \
          bzip2 ccache cmake cpio curl device-tree-compiler ecj fastjar flex gawk gettext gcc-multilib g++-multilib \
          git gperf haveged help2man intltool lib32gcc1 libc6-dev-i386 libelf-dev libglib2.0-dev libgmp3-dev libltdl-dev \
          libmpc-dev libmpfr-dev libncurses5-dev libncursesw5 libncursesw5-dev libreadline-dev libssl-dev libtool lrzsz \
          mkisofs msmtp nano ninja-build p7zip p7zip-full patch pkgconf python2.7 python3 python3-pip python3-ply \
          python-docutils qemu-utils re2c rsync scons squashfs-tools subversion swig texinfo uglifyjs upx-ucl unzip \
          vim wget xmlto xxd zlib1g-dev
        ```
      </details>
    - Method 2:
      ```bash
      curl -s https://build-scripts.immortalwrt.eu.org/init_build_environment.sh | sudo bash
      ```

  - You can also download and use prebuilt container directly:<br/>
    See #Quickstart - Build image via OPDE

  Note:
  - For the for love of god please do __not__ use ROOT user to build your image.
  - Using CPUs based on other architectures should be fine to compile ImmortalWrt, but more hacks are needed - No warranty at all.
  - You must __not__ have spaces in PATH or in the work folders on the drive.
  - If you're using Windows Subsystem for Linux (or WSL), removing Windows folders from PATH is required, please see [Build system setup WSL](https://openwrt.org/docs/guide-developer/build-system/wsl) documentation.
  - Using macOS as the host build OS is __not__ recommended. No warranty at all. You can get tips from [Build system setup macOS](https://openwrt.org/docs/guide-developer/build-system/buildroot.exigence.macosx) documentation.
    - As you're building ImmortalWrt, patching or disabling UPX tools is also required.
  - For more details, please see [Build system setup](https://openwrt.org/docs/guide-developer/build-system/install-buildsystem) documentation.

  ### Quickstart
  - Method 1:
    1. Run `git clone -b <branch> --single-branch https://github.com/immortalwrt/immortalwrt` to clone the source code.
    2. Run `cd immortalwrt` to enter source directory.
    3. Run `./scripts/feeds update -a` to obtain all the latest package definitions defined in feeds.conf / feeds.conf.default
    4. Run `./scripts/feeds install -a` to install symlinks for all obtained packages into package/feeds/
    5. Run `make menuconfig` to select your preferred configuration for the toolchain, target system & firmware packages.
    6. Run `make` to build your firmware. This will download all sources, build the cross-compile toolchain and then cross-compile the GNU/Linux kernel & all chosen applications for your target system.

  - Method 2:
    <details>
      <summary>Build image via OPDE</summary>

      - Pull the prebuilt container:
        ```bash
        docker pull immortalwrt/opde:base
        # docker run --rm -it immortalwrt/opde:base
        ```

      - For Linux User:
        ```bash
        git clone -b <branch> --single-branch https://github.com/immortalwrt/immortalwrt && cd immortalwrt
        docker run --rm -it \
            -v $PWD:/openwrt \
          immortalwrt/opde:base zsh
        ./scripts/feeds update -a && ./scripts/feeds install -a
        ```

      - For Windows User:
        1. Create a volume 'immortalwrt' and clone ImmortalWrt source into volume.
          ```bash
          docker run --rm -it -v immortalwrt:/openwrt immortalwrt/opde:base git clone -b <branch> --single-branch https://github.com/immortalwrt/immortalwrt .
          ```
        2. Enter docker container and update feeds.
          ```bash
          docker run --rm -it -v immortalwrt:/openwrt immortalwrt/opde:base
          ./scripts/feeds update -a && ./scripts/feeds install -a
          ```
        - Tips: ImmortalWrt source code can not be cloned into NTFS filesystem (symbol link problem during compilation), but docker volume is fine.

      - Proxy Support:
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

        > Recommand `http` rather `socks5` protocol
        >
        > IP can not be `localhost` or `127.0.0.1`

      - For Windows User, binary is still in volume. It can be copied to outside via followed command:
        ```bash
        docker run --rm -v <D:\path\to\dir>:/dst -v openwrt:/openwrt -w /dst immortalwrt:base cp /openwrt/bin /dst
        ```
        > Make sure `D:\path\to\dir` has been appended in [File Sharing](https://docs.docker.com/docker-for-windows/#file-sharing).

    </details>

  ### Related Repositories
  The main repository uses multiple sub-repositories to manage packages of different categories. All packages are installed via the ImmortalWrt package manager called opkg. If you're looking to develop the web interface or port packages to ImmortalWrt, please find the fitting repository below.
  - [LuCI Web Interface](https://github.com/immortalwrt/luci): Modern and modular interface to control the device via a web browser.
  - [ImmortalWrt Packages](https://github.com/immortalwrt/packages): Community repository of ported packages.
  - [OpenWrt Routing](https://github.com/openwrt/routing): Packages specifically focused on (mesh) routing.
  - [CONTRIBUTED.md](https://github.com/immortalwrt/immortalwrt/blob/master/CONTRIBUTED.md): the 3rd-party packages we introduced.

## Support Information
For a list of supported devices see the [OpenWrt Hardware Database](https://openwrt.org/supported_devices)
  ### Documentation
  - [Quick Start Guide](https://openwrt.org/docs/guide-quick-start/start)
  - [User Guide](https://openwrt.org/docs/guide-user/start)
  - [Developer Documentation](https://openwrt.org/docs/guide-developer/start)
  - [Technical Reference](https://openwrt.org/docs/techref/start)

  ### Support Community
  - Support Chat: group [@ctcgfw_openwrt_discuss](https://t.me/ctcgfw_openwrt_discuss) on [Telegram](https://telegram.org/).
  - Support Chat: group [#immortalwrt](https://matrix.to/#/#immortalwrt:matrix.org) on [Matrix](https://matrix.org/).

## License
ImmortalWrt is licensed under [GPL-3.0-only](https://spdx.org/licenses/GPL-3.0-only.html).
