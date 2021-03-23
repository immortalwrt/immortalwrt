#!/bin/sh

sleeptime=60
logfile="/var/log/dnscache.file"
dns_caching="$(uci -q get turboacc.config.dns_caching)"
dns_caching_mode="$(uci -q get turboacc.config.dns_caching_mode 2>/dev/null)"

clean_log(){
logrow="$(grep -c "" "${logfile}")"
if [ "${logrow}" -ge "500" ];then
    echo "${curtime} Log条数超限，清空处理！" > "${logfile}"
fi
}

while [ "${dns_caching}" -eq "1" ];
do
	curtime="$(date "+%H:%M:%S")"
	echo "${curtime} online!"
	pidof dnscache>/dev/null || {
		if [ "${dns_caching_mode}" = "1" ]; then
			/var/sbin/dnscache -c "/var/etc/dnscache.conf" &
		elif [ "${dns_caching_mode}" = "2" ]; then
			/var/sbin/dnscache -f "/var/run/dnscache/dnscache.conf" &
		fi
		echo "${curtime} 重启服务！" >> ${logfile}
}

	clean_log
	sleep "${sleeptime}"
	continue
done
