#!/bin/sh

sleeptime=60
logfile="/var/log/dnscache.file"
adg_logfile="/etc/AdGuardHome/data/querylog.json"
dns_caching="$(uci -q get turboacc.config.dns_caching)"
dns_caching_mode="$(uci -q get turboacc.config.dns_caching_mode 2>/dev/null)"
lan_addr="$(uci get network.lan.ipaddr)"

clean_log(){
logrow="$(grep -c "" "${logfile}")"
if [ "${logrow}" -ge "500" ];then
    echo "${curtime} Log条数超限，清空处理！" > "${logfile}"
fi
if [ "${dns_caching_mode}" = "3" ]; then
	adg_logrow="$(grep -c "" "${adg_logfile}")"
	if [ "${adg_logrow}" -ge "500" ];then
		echo "${curtime} Log条数超限，清空处理！" > "${adg_logfile}"
	fi
fi
}

while [ "${dns_caching}" -eq "1" ];
do
	curtime="$(date "+%H:%M:%S")"
	echo "${curtime} online!"
	if [ "${dns_caching_mode}" = "3" ]; then
		pidof AdGuardHome>/dev/null || {
			AdGuardHome -c "/etc/AdGuardHome/AdGuardHome.yaml" -w "/etc/AdGuardHome" -h "${lan_addr}" -p "3001" --no-check-update &
			echo "${curtime} 重启服务！" >> ${logfile}
}
	else
		pidof dnscache>/dev/null || {
			if [ "${dns_caching_mode}" = "1" ]; then
				/var/sbin/dnscache -c "/var/etc/dnscache.conf" &
			elif [ "${dns_caching_mode}" = "2" ]; then
				/var/sbin/dnscache -f "/var/run/dnscache/dnscache.conf" &
			fi
			echo "${curtime} 重启服务！" >> ${logfile}
}
	fi

	clean_log
	sleep "${sleeptime}"
	continue
done