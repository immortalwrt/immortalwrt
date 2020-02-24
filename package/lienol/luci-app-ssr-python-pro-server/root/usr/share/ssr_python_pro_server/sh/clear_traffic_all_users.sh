#!/bin/sh

cd /usr/share/ssr_python_pro_server
user_total=$(./mujson_mgr.py -l | wc -l)
[ $user_total -eq 0 ] && echo -e "没有发现用户，请检查 !" && exit 1
for i in `seq 1 $user_total`
do
	user_id=$(./mujson_mgr.py -l | sed -n ${i}p | awk '{print $2}')
	match_clear=$(./mujson_mgr.py -c -I "${user_id}" | grep 'clear')
	if [ -z "$match_clear" ]; then
		echo -e "$user_id已使用流量清零失败"
	else
		echo -e "$user_id已使用流量清零成功"
	fi
done
exit
