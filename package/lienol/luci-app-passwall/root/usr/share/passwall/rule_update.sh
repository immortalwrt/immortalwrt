#!/bin/sh

CONFIG=passwall
LOG_FILE=/var/log/$CONFIG.log
RULE_PATH=/etc/config/${CONFIG}_rule
Date=$(date "+%Y-%m-%d %H:%M:%S")

url_main="https://raw.githubusercontent.com/hq450/fancyss/master/rules"

update=$1
gfwlist_update=0
chnroute_update=0
if [ -n "$update" ]; then
	[ -n "$(echo $update | grep "gfwlist_update")" ] && gfwlist_update=1
	[ -n "$(echo $update | grep "chnroute_update")" ] && chnroute_update=1
	[ -n "$(echo $update | grep "chnlist_update")" ] && chnlist_update=1
else
	gfwlist_update=$(uci -q get $CONFIG.@global_rules[0].gfwlist_update)
	chnroute_update=$(uci -q get $CONFIG.@global_rules[0].chnroute_update)
	chnlist_update=$(uci -q get $CONFIG.@global_rules[0].chnlist_update)
fi

if [ "$gfwlist_update" == 0 -a "$chnroute_update" == 0 -a "$chnlist_update" == 0 ]; then
	exit
fi

uci_get_by_type() {
	local index=0
	if [ -n $4 ]; then
		index=$4
	fi
	local ret=$(uci -q get $CONFIG.@$1[$index].$2 2>/dev/null)
	echo ${ret:=$3}
}

get_url() {
	local url=$1
	local save_path=$2
	status=$(/usr/bin/curl -w %{http_code} --connect-timeout 5 --retry 1 $url --silent -o $save_path)
	[ "$?" != 0 ] && {
		status=$(/usr/bin/wget -q --no-check-certificate --timeout=5 --tries 1 $url -O $save_path)
		[ "$?" == 0 ] && status=200
	}
	echo $status
}

# rule update
echo $Date: 开始更新规则，请等待... > $LOG_FILE
status=$(get_url $url_main/version1 /tmp/version1)
[ -z "$status" ] && {
	echo $Date: 无法访问更新接口，请更新接口！ >> $LOG_FILE
	exit
}
online_content=$(cat /tmp/version1 2>/dev/null)
if [ -z "$online_content" ]; then
	rm -rf /tmp/version1
	echo $Date: 没有检测到在线版本，可能是访问github有问题！ >> $LOG_FILE
	exit
fi

# update gfwlist
if [ "$gfwlist_update" == 1 ]; then
	gfwlist=$(cat /tmp/version1 | sed -n 1p)
	version_gfwlist2=$(echo $gfwlist | sed 's/ /\n/g' | sed -n 1p)
	md5sum_gfwlist2=$(echo $gfwlist | sed 's/ /\n/g' | tail -n 2 | head -n 1)
	local_md5sum_gfwlist=$(md5sum $RULE_PATH/gfwlist.conf | awk '{print $1}')
	if [ ! -z "$version_gfwlist2" ]; then
		version_gfwlist1=$(uci_get_by_type global_rules gfwlist_version)
		if [ "$version_gfwlist1" != "$version_gfwlist2" -o "$md5sum_gfwlist2" != "$local_md5sum_gfwlist" ]; then
			echo $Date: 检测到新版本gfwlist，开始更新... >> $LOG_FILE
			echo $Date: 下载gfwlist到临时文件... >> $LOG_FILE
			status=$(get_url $url_main/gfwlist.conf /tmp/gfwlist.conf)
			[ -z "$status" ] && {
				echo $Date: 无法访问更新接口，请更新接口！ >> $LOG_FILE
				exit
			}
			md5sum_gfwlist1=$(md5sum /tmp/gfwlist.conf | sed 's/ /\n/g' | sed -n 1p)
			if [ "$md5sum_gfwlist1"x = "$md5sum_gfwlist2"x ]; then
				echo $Date: 下载完成，校验通过，将临时文件覆盖到原始gfwlist文件 >> $LOG_FILE
				mv /tmp/gfwlist.conf $RULE_PATH/gfwlist.conf
				uci set $CONFIG.@global_rules[0].gfwlist_version=$version_gfwlist2
				rm -rf /tmp/dnsmasq.d/gfwlist.conf
				reboot="1"
				echo $Date: 你的gfwlist已经更新到最新了哦~ >> $LOG_FILE
			else
				echo $Date: 下载完成，但是校验没有通过！ >> $LOG_FILE
			fi
		else
			echo $Date: 检测到gfwlist本地版本号和在线版本号相同，不用更新! >> $LOG_FILE
		fi
	else
		echo $Date: gfwlist文件下载失败！ >> $LOG_FILE
	fi
	rm -rf /tmp/gfwlist.conf
fi

# update chnroute
if [ "$chnroute_update" == 1 ]; then
	chnroute=$(cat /tmp/version1 | sed -n 2p)
	version_chnroute2=$(echo $chnroute | sed 's/ /\n/g' | sed -n 1p)
	md5sum_chnroute2=$(echo $chnroute | sed 's/ /\n/g' | tail -n 2 | head -n 1)
	local_md5sum_chnroute=$(md5sum $RULE_PATH/chnroute | awk '{print $1}')
	if [ ! -z "$version_chnroute2" ]; then
		version_chnroute1=$(uci_get_by_type global_rules chnroute_version)
		if [ "$version_chnroute1" != "$version_chnroute2" -o "$md5sum_chnroute2" != "$local_md5sum_chnroute" ]; then
			echo $Date: 检测到新版本chnroute，开始更新... >> $LOG_FILE
			echo $Date: 下载chnroute到临时文件... >> $LOG_FILE
			status=$(get_url $url_main/chnroute.txt /tmp/chnroute)
			[ -z "$status" ] && {
				echo $Date: 无法访问更新接口，请更新接口！ >> $LOG_FILE
				exit
			}
			md5sum_chnroute1=$(md5sum /tmp/chnroute | sed 's/ /\n/g' | sed -n 1p)
			if [ "$md5sum_chnroute1"x = "$md5sum_chnroute2"x ]; then
				echo $Date: 下载完成，校验通过，将临时文件覆盖到原始chnroute文件 >> $LOG_FILE
				mv /tmp/chnroute $RULE_PATH/chnroute
				uci set $CONFIG.@global_rules[0].chnroute_version=$version_chnroute2
				ipset flush chnroute
				ipset destroy chnroute
				reboot="1"
				echo $Date: 你的chnroute已经更新到最新了哦~ >> $LOG_FILE
			else
				echo $Date: 下载完成，但是校验没有通过！ >> $LOG_FILE
			fi
		else
			echo $Date: 检测到chnroute本地版本号和在线版本号相同，不用更新! >> $LOG_FILE
		fi
	else
		echo $Date: chnroute文件下载失败！ >> $LOG_FILE
	fi
	rm -rf /tmp/chnroute
fi

# update chnlist
if [ "$chnlist_update" == 1 ]; then
	chnlist=$(cat /tmp/version1 | sed -n 4p)
	version_chnlist2=$(echo $chnlist | sed 's/ /\n/g' | sed -n 1p)
	md5sum_chnlist2=$(echo $chnlist | sed 's/ /\n/g' | tail -n 2 | head -n 1)
	local_md5sum_chnlist=$(md5sum $RULE_PATH/chnlist | awk '{print $1}')
	if [ ! -z "$version_chnlist2" ]; then
		version_chnlist1=$(uci_get_by_type global_rules chnlist_version)
		if [ "$version_chnlist1" != "$version_chnlist2" -o "$md5sum_chnlist2" != "$local_md5sum_chnlist" ]; then
			echo $Date: 检测到新版本chnlist，开始更新... >> $LOG_FILE
			echo $Date: 下载chnlist到临时文件... >> $LOG_FILE
			status=$(get_url $url_main/cdn.txt /tmp/chnlist)
			[ -z "$status" ] && {
				echo $Date: 无法访问更新接口，请更新接口！ >> $LOG_FILE
				exit
			}
			md5sum_chnlist1=$(md5sum /tmp/chnlist | sed 's/ /\n/g' | sed -n 1p)
			if [ "$md5sum_chnlist1"x = "$md5sum_chnlist2"x ]; then
				echo $Date: 下载完成，校验通过，将临时文件覆盖到原始chnlist文件 >> $LOG_FILE
				mv /tmp/chnlist $RULE_PATH/chnlist
				uci set $CONFIG.@global_rules[0].chnlist_version=$version_chnlist2
				reboot="1"
				echo $Date: 你的chnlist已经更新到最新了哦~ >> $LOG_FILE
			else
				echo $Date: 下载完成，但是校验没有通过！ >> $LOG_FILE
			fi
		else
			echo $Date: 检测到chnlist本地版本号和在线版本号相同，不用更新! >> $LOG_FILE
		fi
	else
		echo $Date: chnlist文件下载失败！ >> $LOG_FILE
	fi
	rm -rf /tmp/chnlist
fi

echo $Date: 更新进程运行完毕！ >> $LOG_FILE
rm -rf /tmp/version1
# write number
uci set $CONFIG.@global_rules[0].gfwlist_update=$gfwlist_update
uci set $CONFIG.@global_rules[0].chnroute_update=$chnroute_update
uci set $CONFIG.@global_rules[0].chnlist_update=$chnlist_update
uci commit
# reboot
if [ "$reboot" == "1" ]; then
	echo $Date: 重启软件，以应用新的规则文件！请稍后！ >> $LOG_FILE
	/etc/init.d/$CONFIG restart
fi
exit
