#!/bin/bash /etc/rc.common
. /lib/functions.sh

del_address(){
countt=$(grep -c '' /usr/share/clashbackup/address.list)
count_nums=1
while [[ $count_nums -le $countt ]]
do
line=$(sed -n "$count_nums"p /usr/share/clashbackup/address.list)
check_addr=$(grep -F "$line" "/usr/share/clash/server.list") 
line_no=$(grep -n "$check_addr" /usr/share/clash/server.list |awk -F ':' '{print $1}')
if [ ! -z $check_addr ];then
sed -i "${line_no}d" /usr/share/clash/server.list	
fi	
count_nums=$(( $count_nums + 1))	
done
rm -rf /usr/share/clashbackup/address.list >/dev/null 2>&1		
}
del_address >/dev/null 2>&1