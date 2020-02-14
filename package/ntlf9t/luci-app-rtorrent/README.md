# luci-app-rtorrent
rTorrent client for OpenWrt's LuCI web interface

## Features
- List all torrent downloads
- Add new torrent by url/magnet uri/file
- Stop/start/delete torrents
- Mark torrents with tags
- Set priority per file
- Enable/disable and add trackers to torrent
- Detailed peer listing
- Completely LuCI based interface
- OpenWrt device independent (written in lua)
- Opkg package manager support
- RSS feed downloader (automatically download torrents that match the specified criteria)
- Support for [Transdroid](https://www.transdroid.org/) ([Transdrone](https://play.google.com/store/apps/details?id=org.transdroid.lite))

## Screenshots
[luci-app-rtorrent 0.1.3](https://github.com/wolandmaster/luci-app-rtorrent/wiki/Screenshots)

## Install instructions
(for Openwrt 15.05.1 Chaos Calmer)

### Install rtorrent-rpc
```
opkg update
opkg install rtorrent-rpc
```
(Note: If you going to install rtorrent-rpc to an own [opkg destination](https://wiki.openwrt.org/doc/techref/opkg#installation_destinations) then you have to install libopenssl to the root destination before)

### Create rTorrent config file

#### Minimal _/root/.rtorrent.rc_ file:
```
directory = /path/to/downloads/
session = /path/to/session/

scgi_port = 127.0.0.1:5000

schedule = rss_downloader,300,300,"execute=/usr/lib/lua/rss_downloader.lua"
```
#### Sample _/root/.rtorrent.rc_ file:
http://pissedoffadmins.com/os/linux/sample-rtorrent-rc-file.html

#### Recommended kernel parameters to avoid low memory issues:
```
cat /etc/sysctl.conf
...
# handle rtorrent related low memory issues
vm.swappiness=95
vm.vfs_cache_pressure=200
vm.min_free_kbytes=4096
vm.overcommit_memory=2
vm.overcommit_ratio=60
```

### Create init.d script (optional)

#### Install screen
```
opkg install screen
```

#### Create _/etc/init.d/rtorrent_ script
```
#!/bin/sh /etc/rc.common

START=99
STOP=99

SCREEN=/usr/sbin/screen
PROG=/usr/bin/rtorrent

start() {
  sleep 3
  $SCREEN -dm -t rtorrent nice -19 $PROG
}

stop() {
  killall rtorrent
}
```

#### Start rtorrent
```
chmod +x /etc/init.d/rtorrent
/etc/init.d/rtorrent enable
/etc/init.d/rtorrent start
```

### Install wget
(the wget in busybox does not support https)
```
opkg install ca-certificates
opkg install wget
ln -sf $(which wget-ssl) /usr/bin/wget
```

### Install luci-app-rtorrent
```
wget -nv https://github.com/wolandmaster/luci-app-rtorrent/releases/download/latest/e1a1ba8004c4220f -O /etc/opkg/keys/e1a1ba8004c4220f
echo 'src/gz luci_app_rtorrent https://github.com/wolandmaster/luci-app-rtorrent/releases/download/latest' >> /etc/opkg.conf
opkg update
opkg install luci-app-rtorrent
```

### Upgrade already installed version
```
opkg update
opkg upgrade luci-app-rtorrent
```

### References
<https://www.pcsuggest.com/openwrt-torrent-download-box-luci/>

<https://medium.com/openwrt-iot/lede-openwrt-setting-up-torrent-downloading-a06fe37a1ea2>
