require("luci.sys")
require("luci.util")
local fs=require"nixio.fs"
local uci=require"luci.model.uci".cursor()
local m,s,o
m = Map("autoipsetadder", translate("ipsetautoadder"))
m.description = translate("自动将联不通的域名加入ipset")
m:section(SimpleSection).template  = "autoipsetadder/status"

s = m:section(TypedSection, "autoipsetadder")
s.anonymous=true
s.addremove=false
---- enable
o = s:option(Flag, "enabled", translate("启用"))
o.default = 0
o.rmempty = false
---- logview
o=s:option(TextValue, "show", "日志")
o.template = "autoipsetadder/check"
---- logpath
o = s:option(Value, "logfile", translate("Runtime log file"))
o.datatype    = "string"
o.default="/tmp/addlist.log"
o.optional = false
o.validate=function(self, value)
if fs.stat(value,"type")=="dir" then
	fs.rmdir(value)
end
if fs.stat(value,"type")=="dir" then
	if m.message then
	m.message =m.message.."\nerror!log file is a dir"
	else
	m.message ="error!log file is a dir"
	end
	return nil
end 
return value
end
---- dnsmasq log
o = s:option(Value, "dnslogfile", translate("dnsmasq log file"))
o.datatype    = "string"
o.optional = false
o.default="/tmp/dnsmasq.log"
o.validate=function(self, value)
if fs.stat(value,"type")=="dir" then
	fs.rmdir(value)
end
if fs.stat(value,"type")=="dir" then
	if m.message then
	m.message =m.message.."\nerror!log file is a dir"
	else
	m.message ="error!log file is a dir"
	end
	return nil
end 
return value
end
---- crontab 
o = s:option(MultiValue, "crontab", translate("Crontab task"),translate("Please change time and args in crontab"))
o:value("autodeldnslog",translate("Auto del dnsmasq log"))
o:value("autotaillog",translate("Auto tail runtime log"))
o.widget = "checkbox"
o.default = "autodeldnslog autotaillog"
o.rmempty= true

o = s:option(MultiValue, "config", translate("the way add to gfwlist"))
o:value("nochina",translate("no china ip"))
o:value("pingadd",translate("5ping loss1-4"))
o:value("packetpass",translate("packet >12 pass"))
o.widget = "checkbox"
o.default = "nochina pingadd packetpass"
o.rmempty=true
---- apply
nixio.fs.writefile("/var/run/lucilogreload","")
function m.on_commit(map)
	local ucitracktest=uci:get("autoipsetadder","autoipsetadder","ucitracktest")
	if ucitracktest=="1" then
		return
	elseif ucitracktest=="0" then
		io.popen("/etc/init.d/autoipsetadder reload &")
	else
		if (fs.access("/var/run/AdGucitest")) then
			uci:set("autoipsetadder","autoipsetadder","ucitracktest","0")
			io.popen("/etc/init.d/autoipsetadder reload &")
		else
			fs.writefile("/var/run/AdGucitest","")
			if (ucitracktest=="2") then
				uci:set("autoipsetadder","autoipsetadder","ucitracktest","1")
			else
				uci:set("autoipsetadder","autoipsetadder","ucitracktest","2")
			end
		end
		uci:save("autoipsetadder")
		uci:commit("autoipsetadder")
	end
end
return m
