
cat << EOI >> /etc/firewall.include

nft add rule inet fw4 mangle_prerouting iifname wwan0 ip ttl set 65
nft add rule inet fw4 mangle_postrouting oifname wwan0 ip ttl set 64
nft add rule inet fw4 mangle_prerouting iifname wwan1 ip ttl set 65
nft add rule inet fw4 mangle_postrouting oifname wwan1 ip ttl set 64

EOI

# /etc/init.d/network restart

exit 0
