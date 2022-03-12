#!/bin/sh /etc/rc.common

start() {
	iwpriv apcli0 set ApCliEnable=0
	ssid=$(grep -e "ApCliSsid=" /etc/wireless/mt7615/mt7615.1.dat)
	enable=$(grep -e "ApCliEnable=" /etc/wireless/mt7615/mt7615.1.dat)
	iwpriv apcli0 set "$ssid"
	iwpriv apcli0 set ApCliAutoConnect=3
    	iwpriv apcli0 set "$enable"
    	iwpriv apclii0 set ApCliEnable=0
	ssid=$(grep -e "ApCliSsid=" /etc/wireless/mt7615/mt7615.2.dat)
	enable=$(grep -e "ApCliEnable=" /etc/wireless/mt7615/mt7615.2.dat)
	iwpriv apclii0 set "$ssid"
	iwpriv apclii0 set ApCliAutoConnect=3
    	iwpriv apclii0 set "$enable"
}



