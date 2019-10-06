--[[
LuCI - Lua Configuration Interface - smartctl support

Script by animefans_xj @ nvacg.org (af_xj@hotmail.com , xujun@smm.cn)

Licensed under the GNU GPL License, Version 3 (the "license");
you may not use this file except in compliance with the License.
you may obtain a copy of the License at

	http://www.gnu.org/licenses/gpl.txt

$Id$
]]--


require("luci.sys")
require("luci.util")

m=Map("smartinfo",translate("S.M.A.R.T Info"),translate("This widget allow you to monitor your storage devices which support S.M.A.R.T technology."))

s=m:section(TypedSection,"smartinfo",translate("S.M.A.R.T Report Settings"))
s.addremove=false
s.anonymous=true

m:section(SimpleSection).template="smartinfo/smart_status"

s:tab("general",translate("Global"))
--s:tab("detail",translate("Detail"))

enable=s:taboption("general",Flag,"enabled",translate("Enabled"))
enable.rmempty=false
function enable.cfgvalue(self,section)
	return luci.sys.init.enabled("smartinfo") and self.enabled or self.disabled
end
function enable.write(self,section,value)
	if value == "1" then
		luci.sys.call("/etc/init.d/smartinfo stop > /dev/null")
		luci.sys.call("/etc/init.d/smartinfo enable > /dev/null")
		luci.sys.call("/etc/init.d/smartinfo start > /dev/null")
	else
		luci.sys.call("/etc/init.d/smartinfo stop > /dev/null")
		luci.sys.call("/etc/init.d/smartinfo disable > /dev/null")
	end
end
check_interval=s:taboption("general",Value,"time_step",translate("Check Interval"))
check_interval.rmempty=false
check_interval.placeholder="1"
check_interval.datatype="range(1,60)"
check_interval.template="smartinfo/detail_begin_value"
time_unit=s:taboption("general",ListValue,"time_unit",translate("Unit"))
time_unit.rmempty=false
time_unit.template="smartinfo/detail_end_listvalue"
time_unit:value("minute",translate("Minutes"))
time_unit:value("hour",translate("Hours"))
time_unit:value("day",translate("Days"))
time_unit:value("week",translate("Weeks"))

log_enable=s:taboption("general",Flag,"log_enabled",translate("Log to file"))
log_enable.template="smartinfo/detail_begin_flag"
log_enable.rmempty=false
log_path=s:taboption("general",Value,"log_path",translate("at"))
log_path.template="smartinfo/detail_end_value"
log_path.placeholder="/tmp/log"
log_path.rmempty=false

touch_enable=s:taboption("general",Flag,"touch_enabled",translate("Touch a warning file when disk fail"))
touch_enable.template="smartinfo/detail_begin_flag"
touch_enable.rmempty=false
touch_path=s:taboption("general",Value,"touch_path",translate("at"))
touch_path.template="smartinfo/detail_end_value"
touch_path.placeholder="/tmp"
touch_path.rmempty=false

devices=s:taboption("general",DynamicList,"device",translate("Devices to monitor"))
local device_name = nil
local device_model = nil
for d in nixio.fs.glob("/dev/sd?") do
	device_name=nixio.fs.basename(d)
	device_model="%s %s" % {nixio.fs.readfile("/sys/class/block/%s/device/vendor" % device_name), nixio.fs.readfile("/sys/class/block/%s/device/model" % device_name)}
	devices:value(d, "%s: %s" % {device_name, device_model})
end


return m
