#!/bin/bash /etc/rc.common
. /lib/functions.sh

enable_list=$(uci get clash.config.cus_list 2>/dev/null)
if [  $enable_list -eq 1 ];then 

add_address(){
	servers_get()
	{
	   local section="$1"
	   config_get "server" "$section" "server" ""
	   echo "$server" >>/tmp/server.conf
	}
	config_load clash
	config_foreach servers_get "servers"

	count=$(grep -c '' /tmp/server.conf)
	count_num=1
	while [[ $count_num -le $count ]]
	do
	line=$(sed -n "$count_num"p /tmp/server.conf)
	check_addr=$(grep -F "$line" "/usr/share/clash/server.list")
	if [ -z $check_addr ];then
	echo $line >>/usr/share/clashbackup/address.list
	fi	
	count_num=$(( $count_num + 1))	
	done

	sed -i "1i\#START" /usr/share/clashbackup/address.list 2>/dev/null
	sed -i -e "\$a#END" /usr/share/clashbackup/address.list 2>/dev/null
			
	cat /usr/share/clashbackup/address.list /usr/share/clash/server.list > /usr/share/clash/me.list
	rm -rf /usr/share/clash/server.list 
	mv /usr/share/clash/me.list /usr/share/clash/server.list
	chmod 755 /usr/share/clash/server.list
	rm -rf /tmp/server.conf /usr/share/clashbackup/address.list >/dev/null 2>&1
}

add_address >/dev/null 2>&1

if [  -d /tmp/dnsmasq.clash ];then 
 rm -rf /tmp/dnsmasq.clash
fi

if [  -f /tmp/dnsmasq.d/custom_list.conf ];then 
  rm -rf /tmp/dnsmasq.d/custom_list.conf
fi

cutom_dns=$(uci get clash.config.custom_dns 2>/dev/null)

if [ ! -d /tmp/dnsmasq.d ];then 
 mkdir -p /tmp/dnsmasq.d
fi 

if [ ! -d /tmp/dnsmasq.clash ];then 
	mkdir -p /tmp/dnsmasq.clash
fi

awk '!/^$/&&!/^#/{printf("server=/%s/'"$cutom_dns"'\n",$0)}' /usr/share/clash/server.list >> /tmp/dnsmasq.clash/custom_list.conf
ln -s /tmp/dnsmasq.clash/custom_list.conf /tmp/dnsmasq.d/custom_list.conf
fi


