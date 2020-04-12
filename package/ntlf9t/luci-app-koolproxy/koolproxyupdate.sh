wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR_rule_list/master/kp.dat' -O root/usr/share/koolproxy/data/rules/kp.dat
wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/rules/yhosts.txt' -O root/usr/share/koolproxy/data/rules/yhosts.txt
wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/rules/fanboy-annoyance.txt' -O root/usr/share/koolproxy/data/rules/fanboy.txt
wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/rules/easylistchina.txt' -O root/usr/share/koolproxy/data/rules/easylistchina.txt

wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/rules/user.txt' -O root/usr/share/koolproxy/data/user.txt
cp root/usr/share/koolproxy/data/user.txt root/usr/share/koolproxy/data/rules/user.txt
wget 'https://raw.githubusercontent.com/user1121114685/koolproxyR/master/koolproxyR/koolproxyR/data/koolproxyR_ipset.conf' -O root/usr/share/koolproxy/koolproxy_ipset.conf

wget https://easylist-downloads.adblockplus.org/easylistchina+easylist.txt -O- | grep ^\|\|[^\*]*\^$ | sed -e 's:||:address\=\/:' -e 's:\^:/0\.0\.0\.0:' > root/usr/share/koolproxy/dnsmasq.adblock
sed -i '/youku/d' root/usr/share/koolproxy/dnsmasq.adblock
sed -i '/[1-9]\{1,3\}\.[1-9]\{1,3\}\.[1-9]\{1,3\}\.[1-9]\{1,3\}/d' root/usr/share/koolproxy/dnsmasq.adblock
