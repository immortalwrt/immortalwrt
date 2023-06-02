#!/bin/sh

. /lib/functions.sh
. /etc/openwrt_release

wifi_password="TPTlam011205@!"
ten_wifi="TPT Lam"
hostname="DOANDUY"

sed -i -re 's/^(option check_signature.*)/#\1/g' /etc/opkg.conf

uci set 'network.lan.ipv6=0'
uci set 'network.wan.ipv6=0'
uci set 'dhcp.lan.dhcpv6=disabled'
/etc/init.d/odhcpd disable
/etc/init.d/odhcpd stop

uci -q delete dhcp.lan.dhcpv6
uci -q delete dhcp.lan.ra
uci commit dhcp
/etc/init.d/odhcpd restart

uci set network.lan.delegate="0"
uci commit network

uci set base_config.@status[0].SSID=${ten_wifi}
uci set base_config.@status[0].SSID_PASSWD=${wifi_password}
uci set base_config.@status[0].country="CN"
uci commit base_config

for radio in 'radio0' 'radio1'
do
    uci set wireless."$radio".country="CN"
    uci set wireless."$radio".disabled="0"
    uci set wireless.default_"$radio".ssid=${ten_wifi}
    uci set wireless.default_"$radio".encryption="psk2"
    uci set wireless.default_"$radio".key=${wifi_password}
    uci -q commit wireless
done

uci commit wireless

wifi reload
/sbin/wifi reload

uci delete system.ntp.server
uci add_list system.ntp.server='0.vn.pool.ntp.org'
uci add_list system.ntp.server='2.asia.pool.ntp.org'
uci add_list system.ntp.server='1.asia.pool.ntp.org'
uci add_list system.ntp.server='125.235.4.198'
uci add_list system.ntp.server='115.73.220.183'
uci add_list system.ntp.server='222.255.146.26'

uci set system.@system[0]=system 
uci set system.@system[0].hostname="${hostname}"
uci set system.@system[0].zonename='Asia/Ho Chi Minh'
uci set system.@system[0].timezone='<+07>-7'
uci commit system

cat << EOI >> /etc/dnsmasq.conf

ipset=/youtube.com/youtube
ipset=/googlevideo.com/youtube
ipset=/ytimg.com/youtube
ipset=/zalo.vn/zalo
ipset=/zaloapp.com/zalo
ipset=/zalo.me/zalo
ipset=/zadn.vn/zalo
ipset=/tiktok.com/tiktok
ipset=/tiktokv.com/tiktok
ipset=/tiktokcdn.com/tiktok
ipset=/byteicdn.com/tiktok

EOI

/etc/init.d/dnsmasq restart



cat << EOI >> /etc/firewall.include

nft add rule inet fw4 mangle_prerouting iifname wwan0 ip ttl set 65
nft add rule inet fw4 mangle_postrouting oifname wwan0 ip ttl set 64

nft add rule inet fw4 mangle_prerouting iifname wwan1 ip ttl set 65
nft add rule inet fw4 mangle_postrouting oifname wwan1 ip ttl set 64

nft add rule inet fw4 mangle_prerouting iifname usb0 ip ttl set 65
nft add rule inet fw4 mangle_postrouting oifname usb0 ip ttl set 65

EOI

cat << EOI >> /etc/firewall.user

iptables -t mangle -I POSTROUTING -o usb0 -j TTL --ttl-set 65
iptables -t mangle -I POSTROUTING -o wwan0 -j TTL --ttl-set 64
iptables -t mangle -I POSTROUTING -o wwan1 -j TTL --ttl-set 64

EOI

uci add firewall include
uci set firewall.@include[0].path='/etc/firewall.include'
uci set firewall.@include[0].fw4_compatible='1'
uci commit firewall
service firewall restart

# /etc/init.d/network restart

exit 0
