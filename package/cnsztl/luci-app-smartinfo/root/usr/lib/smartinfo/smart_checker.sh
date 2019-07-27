#!/bin/sh
# Copyright (C) 2016 NVACG , 穿越蓝天/animefans_xj
# af_xj@hotmail.com
#
#Licensed under the GNU GPL License, Version 3 (the "license");
#you may not use this file except in compliance with the License.
#you may obtain a copy of the License at
#
#	http://www.gnu.org/licenses/gpl.txt

. /lib/functions.sh
. /usr/lib/smartinfo/smart_functions.sh

config_load "smartinfo"

local enable time time_step time_unit log_enabled log_path touch_enable touch_path

config_get  enabled  main enabled
config_get  time_step  main  time_step
config_get  time_unit  main  time_unit
config_get  log_enabled main  log_enabled
config_get  log_path  main  log_path
config_get  touch_enabled main  touch_enabled
config_get  touch_path  main  touch_path

echo "NVACG SMART CHECK SCRIPT"
echo "Paramter ==================="
echo "Enabled:  $enabled"
echo "Time Step:  $time_step"
echo "Time Unit:  $time_unit"
echo "Log:  $log_enabled"
echo "Path: $log_path"
echo "Touch:  $touch_enabled"
echo "Path: $touch_path"

if [ $enabled -ne 1 ]; then
  exit 0
fi


# 根据选定的时间间隔生成秒数
case $time_unit in
  "minute")
    let time=time_step*60
  ;;
  "hour")
    let time=time_step*3600
  ;;
  "day")
    let time=time_step*86400
  ;;
   "week")
    let time=time_step*604800
  ;;
  *)
    let time=time_step*60
  ;;
esac

echo "Calculated Time: $time"
echo ""


main_process() {
  do_smart_check $1
  local RETVAL=$?
  local Device=`echo $1 |awk -F/ '{print $NF}`
  local Model
  # 如果指定的磁盘在线则获取型号
  [ -d /sys/class/block/$Device ] && Model=`cat /sys/class/block/$Device/device/vendor``cat /sys/class/block/$Device/device/model`
  # 如果路径不存在则创建目录
  [ $log_enabled ] && ! [ -d $log_path ] && mkdir -p $log_path
  [ $touch_enabled ] && ! [ -d $touch_path ] && mkdir -p $touch_path
  case $RETVAL in
    0)
      # 设备健康
      [ $log_enabled ] && [ -d $log_path ] && echo `date '+%Y-%m-%d %H:%M'`" [$Model]: Passed Disk Check." >> $log_path/smartinfo_$Device.log
      [ -f $touch_path/$Device-DiskDamaged ] && rm $touch_path/$Device-DiskDamaged
    ;;
    1)
      # 设备故障
      logger -p daemon.warn -t smartinfo "Device $1 S.M.A.R.T status Failed!"
      [ $log_enabled ] && [ -d $log_path ] && echo `date '+%Y-%m-%d %H:%M'`" [$Model]: Disk Damaged." >> $log_path/smartinfo_$Device.log
      [ $touch_enabled ] && [ -d $touch_path ] && touch $touch_path/$Device-DiskDamaged
    ;;
    2)
      # 设备离线
      logger -p daemon.warn -t smartinfo "Device $1 Offline."
      [ $log_enabled ] && [ -d $log_path ] && echo `date '+%Y-%m-%d %H:%M'`" : Disk Offline." >> $log_path/smartinfo_$Device.log
      [ -f $touch_path/$Device-DiskDamaged ] && rm $touch_path/$Device-DiskDamaged
    ;;
    3)
      # 不支持S.M.A.R.T
      [ $log_enabled ] && [ -d $log_path ] && echo `date '+%Y-%m-%d %H:%M'`" [$Model]: Unsupport S.M.A.R.T." >> $log_path/smartinfo_$Device.log
      [ -f $touch_path/$Device-DiskDamaged ] && rm $touch_path/$Device-DiskDamaged
    ;;
    *)
    ;;
  esac
}


while [ true ]
do
  echo `date '+%Y-%m-%d %H:%M'`" Execute the S.M.A.R.T Check."
  # 执行S.M.A.R.T检测
  config_list_foreach main device main_process

  # 循环延迟
  sleep $time
done


