#!/bin/sh /etc/rc.common

sleeptime=150
enable=$(uci get clash.config.enable 2>/dev/null)

if [ -f /usr/share/clashbackup/history ];then

HISTORY_PATH="/usr/share/clashbackup/history"
SECRET=$(uci get clash.config.dash_pass 2>/dev/null)
LAN_IP=$(uci get network.lan.ipaddr 2>/dev/null |awk -F '/' '{print $1}' 2>/dev/null)
PORT=$(uci get clash.config.dash_port 2>/dev/null)

cat $HISTORY_PATH |while read line
do
    if [ -z "$(echo $line |grep "#*#")" ]; then
      continue
    else
      GORUP_NAME=$(echo $line |awk -F '#*#' '{print $1}')
      NOW_NAME=$(echo $line |awk -F '#*#' '{print $3}')
      curl -H "Authorization: Bearer ${SECRET}" -H "Content-Type:application/json" -X PUT -d '{"name":"'"$NOW_NAME"'"}' http://"$LAN_IP":"$PORT"/proxies/"$GORUP_NAME" >/dev/null 2>&1
    fi
done >/dev/null 2>&1 
  
fi 

while [ $enable -eq 1 ];
do
	if ! pidof clash>/dev/null; then
		/etc/init.d/clash restart 2>&1 &
	fi

sleep ${sleeptime}
continue
done


