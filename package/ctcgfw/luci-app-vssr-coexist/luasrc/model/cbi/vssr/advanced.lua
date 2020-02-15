local vssr = "vssr"
local uci = luci.model.uci.cursor()
local server_table = {}

local gfwmode=0
local gfw_count=0
local ip_count=0
local ad_count = 0

if nixio.fs.access("/etc/dnsmasq.ssr/gfw_list.conf") then
gfwmode=1		
end

local sys = require "luci.sys"

if gfwmode==1 then 
 gfw_count = tonumber(sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l"))/2

end
 
if nixio.fs.access("/etc/china_ssr.txt") then 
 ip_count = sys.exec("cat /etc/china_ssr.txt | wc -l")
end

uci:foreach(vssr, "servers", function(s)
	if s["type"] == "v2ray" then
		if s.alias then
			server_table[s[".name"]] = "[%s]:%s" %{string.upper(s.type), s.alias}
		elseif s.server and s.server_port then
			server_table[s[".name"]] = "[%s]:%s:%s" %{string.upper(s.type), s.server, s.server_port}
		end
	end
end)

local key_table = {}   
for key,_ in pairs(server_table) do 
	
	table.insert(key_table,key)  
	
end 

table.sort(key_table)
m = Map(vssr)

-- [[ 服务器节点故障自动切换设置 ]]--

s = m:section(TypedSection, "global", translate("Server failsafe auto swith settings"))
s.anonymous = true

o = s:option(Flag, "monitor_enable", translate("Enable Process Deamon"))
o.rmempty = false

o = s:option(Flag, "enable_switch", translate("Enable Auto Switch"))
o.rmempty = false

o = s:option(Value, "switch_time", translate("Switch check cycly(second)"))
o.datatype = "uinteger"
o:depends("enable_switch", "1")
o.default = 3600

o = s:option(Value, "switch_timeout", translate("Check timout(second)"))
o.datatype = "uinteger"
o:depends("enable_switch", "1")
o.default = 5

o = s:option(Value, "switch_try_count", translate("Check Try Count"))
o.datatype = "uinteger"
o:depends("enable_switch", "1")
o.default = 3

-- [[ SOCKS5 Proxy ]]--
if nixio.fs.access("/usr/bin/ssr-local") then
s = m:section(TypedSection, "socks5_proxy", translate("SOCKS5 Proxy"))
s.anonymous = true

o = s:option(ListValue, "server", translate("Server"))
o:value("nil", translate("Disable"))
for _,key in pairs(key_table) do o:value(key,server_table[key]) end
o.default = "nil"
o.rmempty = false

o = s:option(Value, "local_port", translate("Local Port"))
o.datatype = "port"
o.default = 1080
o.rmempty = false

end
-- [[ adblock ]]--
s = m:section(TypedSection, "global", translate("adblock settings"))
s.anonymous = true

o = s:option(Flag, "adblock", translate("Enable adblock"))
o.rmempty = false





return m

