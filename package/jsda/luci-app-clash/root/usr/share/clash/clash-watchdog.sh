#!/bin/sh

sleeptime=300
logfile="/tmp/clash.log"
CLASH="/etc/clash/clash"
CLASH_CONFIG="/etc/clash"
enable=$(uci get clash.config.enable 2>/dev/null)

clean_log(){
	logrow=$(grep -c "" ${logfile})
	logrow1=$(grep -c "" ${logfile1})
	if [ $logrow -ge 1000 ];then
		cat /dev/null > ${logfile}
		echo "$curtime Logs exceeded limit，cleaning logs now..！" >> ${logfile}
	fi
	

}


if [ -f /usr/share/clashbackup/history ];then
HISTORY_PATH="/usr/share/clashbackup/history"
SECRET=$(uci get clash.config.dash_pass 2>/dev/null)
LAN_IP=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
PORT=$(uci get clash.config.dash_port 2>/dev/null)

if [ ! -z "$(grep "#*#" "$HISTORY_PATH")" ]; then
   cat $HISTORY_PATH |while read line
   do
	    GORUP_NAME=$(echo $line |awk -F '#*#' '{print $1}')
	    NOW_NAME=$(echo $line |awk -F '#*#' '{print $3}')
      curl -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X PUT -d '{"name":"'"$NOW_NAME"'"}' http://"$LAN_IP":"$PORT"/proxies/"$GORUP_NAME" >/dev/null 2>&1 
   done
fi 
fi


while [ $enable -eq 1 ];
do
	curtime=`date "+%H:%M:%S"`
	if pidof clash>/dev/null; then
		clean_log
	fi
	if ! pidof clash>/dev/null; then
		/etc/init.d/clash restart 2>&1 &
		echo "$curtime Clash Is Restarting！" >> ${logfile}
	fi

sleep ${sleeptime}
continue
done


