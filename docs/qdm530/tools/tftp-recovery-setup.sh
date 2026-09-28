#!/bin/bash
# Set up a USB-Ethernet interface + dnsmasq TFTP server for booting the QDM530 over
# TFTP from U-Boot. Plug the USB-Ethernet adapter into the board's LAN port FIRST,
# then run this with sudo. TFTP root = /tmp/tftp (place the image, e.g. imm.itb,
# there first). Matches the board's factory U-Boot env (serverip 192.168.10.19).
set -u
SRV=192.168.10.19

# find a USB-ethernet interface (enpXsYfZuW pattern, or any non-wifi/non-virtual
# ethernet that is not lo)
IF=$(ls /sys/class/net | while read n; do
  [ -e "/sys/class/net/$n/device" ] || continue
  case "$n" in lo|wl*|docker*|virbr*|veth*|br-*) continue;; esac
  echo "$n"
done | grep -E '^en' | head -1)
[ -z "$IF" ] && IF=$(ls /sys/class/net | grep -E '^enp.*u' | head -1)
if [ -z "$IF" ]; then echo "ERROR: no USB-LAN adapter found. Plug it in first."; ip -br link; exit 1; fi
echo "interface = $IF"

pkill -f 'dnsmasq.*tftp' 2>/dev/null; sleep 1

# Stop NetworkManager from managing this adapter, otherwise it strips the static IP
# every time the board reboots and flaps the link - which kills TFTP after the first
# block (the "Loading: *" stall).
nmcli device set "$IF" managed no 2>/dev/null || true
sleep 1

ip addr flush dev "$IF" 2>/dev/null
ip addr add "$SRV/24" dev "$IF"
ip link set "$IF" up
sleep 1
ip -br addr show "$IF"

mkdir -p /tmp/tftp
dnsmasq --port=0 --enable-tftp --tftp-root=/tmp/tftp --interface="$IF" --bind-interfaces --listen-address="$SRV"
sleep 1
pgrep -a dnsmasq | head -1 && echo "dnsmasq TFTP up on $SRV ($IF)" || echo "dnsmasq FAILED to start"
ss -ulnp 2>/dev/null | grep "$SRV:69" && echo "port 69 listening OK"
echo "contents of /tmp/tftp:"; ls -l /tmp/tftp/ 2>/dev/null
