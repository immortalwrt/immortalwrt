#!/bin/sh

sleeptime=60
logfile="/var/log/dnscache.file"
dns_enable=$(uci get flowoffload.@flow[0].dns 2>/dev/null)
dnscache_enable=$(uci get flowoffload.@flow[0].dnscache_enable 2>/dev/null)

clean_log(){
logrow=$(grep -c "" ${logfile})
if [ $logrow -ge 500 ];then
    cat /dev/null > ${logfile}
    echo "$curtime Log条数超限，清空处理！" >> ${logfile}
fi

}

while [ $dns_enable -eq 1 ];
do
curtime=`date "+%H:%M:%S"`
echo "$curtime online! "
if [ $dns_enable -eq 1 ]; then
	if [ $dnscache_enable = "3" ];  then
		if ! pidof AdGuardHome>/dev/null; then
			AdGuardHome -c /etc/AdGuardHome/AdGuardHome.yaml -w /etc/AdGuardHome >/dev/null 2>&1 &
			echo "$curtime 重启服务！" >> ${logfile}
		fi
		if ! pidof dnscache>/dev/null; then
			dnscache -f /var/run/dnscache/dnscache.conf -d
		fi
	else
		if ! pidof dnscache>/dev/null; then
			if [ $dnscache_enable = "1" ];  then
			/usr/sbin/dnscache -c /var/etc/dnscache.conf -d
			elif [ $dnscache_enable = "2" ] || [ $dnscache_enable = "3" ];  then
			dnscache -f /var/run/dnscache/dnscache.conf -d
			fi
			echo "$curtime 重启服务！" >> ${logfile}
		fi
	fi
fi

sleep ${sleeptime}
continue
done

