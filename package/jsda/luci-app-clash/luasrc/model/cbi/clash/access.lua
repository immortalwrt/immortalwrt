
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require "luci.model.uci".cursor()


m = Map("clash")
s = m:section(TypedSection, "clash")
--m.pageaction = false
s.anonymous = true
s.addremove=false

md = s:option(Flag, "proxylan", translate("Proxy Lan IP"))
md.default = 1
md.rmempty = false
md.description = translate("Only selected IPs will be proxied if enabled. fake-ip mode not supported")
md:depends("rejectlan", 0)


o = s:option(DynamicList, "lan_ac_ips", translate("Proxy Lan List"))
o.datatype = "ipaddr"
o.description = translate("Only selected IPs will be proxied")
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o:depends("proxylan", 1)



update_time = SYS.exec("ls -l --full-time /etc/clash/Country.mmdb|awk '{print $6,$7;}'")
o = s:option(Button,"update",translate("Update GEOIP Database")) 
o.title = translate("GEOIP Database")
o.inputtitle = translate("Update GEOIP Database")
o.description = update_time
o.inputstyle = "reload"
o.write = function()
  SYS.call("bash /usr/share/clash/ipdb.sh >>/tmp/clash.log 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash","settings", "access"))
end




md = s:option(Flag, "rejectlan", translate("Bypass Lan IP"))
md.default = 1
md.rmempty = false
md.description = translate("Selected IPs will not be proxied if enabled. fake-ip mode not supported")
md:depends("proxylan", 0)


o = s:option(DynamicList, "lan_ips", translate("Bypass Lan List"))
o.datatype = "ipaddr"
o.description = translate("Selected IPs will not be proxied")
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o:depends("rejectlan", 1)



local apply = luci.http.formvalue("cbi.apply")
if apply then
	uci:commit("clash")
	if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
	end
end

return m
