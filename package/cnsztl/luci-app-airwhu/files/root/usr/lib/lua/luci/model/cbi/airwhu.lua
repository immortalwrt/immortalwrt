--[[

LuCI - Lua Configuration Interface
LICENSE under GPLv3.
Copyright 2017 KyleRicardo[W.B.L.E. TeAm] <shaoyz714@126.com>

]]
--
require("luci.sys")
require("luci.tools.webadmin")

local IsOnAir = (luci.sys.call("pidof mentohust > /dev/null") == 0)  
if IsOnAir then      
    state_msg = "<b><font color=\"green\">" .. translate("Running") .. "</font></b>"  
else  
    state_msg = "<b><font color=\"red\">" .. translate("Not Running") .. "</font></b>"  
end  

m = Map("airwhu", translate("AirWHU"), translate("Configure Ruijie 802.1X client with IPv6 NAT based on Masquerade.") .. "<br /><br />" .. translate("Status") .. " : " .. state_msg)

s = m:section(TypedSection, "switch", translate("Global Switch"), translate("Configure global 802.1X Authentication and IPv6-NAT on-off."))
s.addremove = false
s.anonymous = true

s:option(Flag, "enableauth", translate("Enable 802.1X Auth"), translate("Enable or disable Ruijie 802.1X authentication."))
tmp = s:option(Flag, "startwithboot", translate("Start with boot"), translate("Start Ruijie 802.1X Authentication based on MentoHUST when the router is booting."))
tmp:depends("enableauth","1")
s:option(Flag, "enableipv6", translate("Enable IPv6 NAT"), translate("Enable IPv6 NAT pass-through based on ip6tables MASQUERADE."))

s = m:section(TypedSection, "auth", translate("Config Authentication"), translate("The options below are all of MentoHUST's arguments."))
s.anonymous = true
s.addremove = false

ur = s:option(Value, "Username", translate("Username"))
ur.rmempty = false

pw = s:option(Value, "Password", translate("Password"))
pw.password = true
pw.rmempty = false

wan_dev = luci.sys.exec("uci get network.wan.ifname")
wan_dev = string.sub(wan_dev,1,string.len(wan_dev)-1)
ifname = s:option(ListValue, "ifname", translate("Interfaces"))
ifname:value(wan_dev)

s:option(Value, "IP", translate("IP"), translate("default to localhost's IP")).default = "0.0.0.0"

s:option(Value, "Mask", translate("Netmask"), translate("default to localhost's netmask")).default = "255.255.255.0"

s:option(Value, "Gateway", translate("Gateway"), translate("default to 0.0.0.0")).default = "0.0.0.0"

s:option(Value, "DNS", translate("DNS"), translate("default to 0.0.0.0")).default = "0.0.0.0"

s:option(Value, "PingHost", translate("Ping host"), translate("default to 0.0.0.0,i.e. disable this function")).default = "0.0.0.0"

s:option(Value, "Timeout", translate("Authenticate timeout(s)"), translate("default to 8s")).default = "8"

s:option(Value, "EchoInterval", translate("Heartbeat timeout(s)"), translate("default to 30s")).default = "30"

s:option(Value, "RestartWait", translate("Timeout on failure(s)"), translate("default to 15s")).default = "15"

s:option(Value, "MaxFail", translate("Max failure times"), translate("0 means no limit,default to 8")).default = "8"

t = s:option(ListValue, "StartMode", translate("mulcast address"), translate("default to 1"))
t:value("0", translate("0(standard)"))
t:value("1", translate("1(ruijjie)"))
t:value("2", translate("2(saier)"))
t.default = "1"

t = s:option(ListValue, "DHCPMode", translate("DHCP type"), translate("default to 1"))
t:value("0", translate("0(not in used)"))
t:value("1", translate("1(secondary authenticate)"))
t:value("2", translate("2(post authenticate)"))
t:value("3", translate("3(pre authenticate)"))
t.default = "1"

t = s:option(ListValue, "DaemonMode", translate("run in daemon mode"), translate("default to 3"))
t:value("0", translate("0(no)"))
t:value("1", translate("1(yes,turn off output)"))
t:value("2", translate("2(yes,persevere output)"))
t:value("3", translate("3(yes,output to file)"))
t.default = "3"

s:option(Value, "ShowNotify", translate("display notification"), translate("0(no),1-20(yes),default to 0")).default = "0"

s:option(Value, "Version", translate("client version"), translate("default to 0.00,compatible with xrgsu")).default = "0.00"

tmp = s:option(Value, "DataFile", translate("customized data file"), translate("not in used by default"))
tmp.rmempty = true

script = s:option(Value, "DHCPScript", translate("DHCP script"), translate("use dhclient by default"))
script.default = "udhcpc -i "..wan_dev

local apply = luci.http.formvalue("cbi.apply")
if apply then
	if luci.sys.exec("uci get airwhu.@switch[0].enableauth") then
		luci.sys.exec("/etc/init.d/mentohust restart")
	else
        	luci.sys.exec("/etc/init.d/mentohust stop")
	end

	if luci.sys.exec("uci get airwhu.@switch[0].startwithboot") then
		luci.sys.exec("/etc/init.d/mentohust enable")
	else
        	luci.sys.exec("/etc/init.d/mentohust disable")
	end
	
	if luci.sys.exec("uci get airwhu.@switch[0].enableipv6") then
		luci.sys.exec("sh /bin/ipv6masq.sh install")
	else
		luci.sys.exec("sh /bin/ipv6masq.sh uninstall")
	end
end

return m
