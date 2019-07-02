
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"


m = Map("clash", translate("Clash Client"))
m:section(SimpleSection).template  = "clash/status"
s = m:section(TypedSection, "clash")
s.anonymous = true


o = s:option( Flag, "enable")
o.title = translate("Enable Clash")
o.default = 0
o.rmempty = false
o.description = translate("After clash start running, wait a moment for servers to resolve,enjoy")


o = s:option(Value, "proxy_port")
o.title = translate("* Clash Redir Port")
o.default = 7892
o.datatype = "port"
o.rmempty = false
o.description = translate("Clash config redir-port")

o = s:option(Value, "cn_port")
o.title = translate("Dashboard Port")
o.default = 9090
o.datatype = "port"
o.rmempty = false
o.description = translate("Dashboard hostname is Your router local address. eg, 192.168.1.1")

o = s:option(Value, "dashboard_password")
o.title = translate("Dashboard Secret")
o.default = 123456
o.rmempty = false
o.description = translate("Dashboard Secret")

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.rmempty = false
o.description = translate("Auto Update Server subscription")


o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.rmempty = false

o = s:option(Value, "subscribe_url")
o.title = translate("Subcription Url")
o.description = translate("Server Subscription Address")
o.rmempty = true

o = s:option(Button,"update")
o.title = translate("Update Subcription")
o.inputtitle = translate("Update Configuration")
o.inputstyle = "reload"
o.write = function()
  os.execute("mv /etc/clash/config.yml /etc/clash/config.bak")
  SYS.call("bash /usr/share/clash/clash.sh >>/tmp/clash.log 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end


local apply = luci.http.formvalue("cbi.apply")
if apply then
	os.execute("/etc/init.d/clash restart >/dev/null 2>&1 &")
end





return m


