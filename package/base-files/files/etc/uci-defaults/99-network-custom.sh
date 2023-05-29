
cat << EOI >> /etc/nftables.d/10-custom-filter-chains.nft

chain mangle_ttl_out { 
    type filter hook postrouting priority mangle; 
 oifname eth1 ip ttl set 65 
    oifname eth0.2 ip ttl set 65
    oifname usb0 ip ttl set 65
    oifname wan ip ttl set 65 
    oifname usb1 ip ttl set 65 
    oifname wwan0 ip ttl set 64     
}

EOI

# /etc/init.d/network restart

exit 0
