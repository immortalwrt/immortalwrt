local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()

ful = Form("upload", nil)
ful.reset = false
ful.submit = false

m = Map("clash")
s = m:section(TypedSection, "clash")
s.anonymous = true


o = s:option(Flag, "auto_update", translate("Auto Update"))
o.rmempty = false
o.description = translate("Auto Update Server subscription")


o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.rmempty = false
o.description = translate("Daily Server subscription update time")

o = s:option(ListValue, "subcri", translate("Subcription Type"))
o.default = clash
o:value("clash", translate("clash"))
o:value("v2rayn2clash", translate("v2rayn2clash"))
o:value("surge2clash", translate("surge2clash"))
o.description = translate("Select Subcription Type, enter only your subcription url without https://tgbot.lbyczf.com/*?")


o = s:option(Value, "subscribe_url")
o.title = translate("Subcription Url")
o.description = translate("Server Subscription Address")
o.rmempty = true

o = s:option(Button,"update")
o.title = translate("Update Subcription")
o.inputtitle = translate("Update")
o.description = translate("Update Config")
o.inputstyle = "reload"
o.write = function()
  os.execute("sed -i '/enable/d' /etc/config/clash")
  uci:commit("clash")
  os.execute("mv /etc/clash/config.yaml /etc/clash/config.bak")
  os.execute("rm -rf /tmp/clash.log")
  SYS.call("bash /usr/share/clash/clash.sh >>/tmp/clash.log 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash", "client"))
end


o = s:option(Button,"enable")
o.title = translate("Start Client")
o.inputtitle = translate("Start Client")
o.description = translate("Enable/Start/Restart Client")
o.inputstyle = "apply"
o.write = function()
  uci:set("clash", "config", "enable", 1)
  uci:commit("clash")
  SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
end


o = s:option(Button,"disable")
o.title = translate("Stop Client")
o.inputtitle = translate("Stop Client")
o.description = translate("Disable/Stop Client")
o.inputstyle = "reset"
o.write = function()
  uci:set("clash", "config", "enable", 0)
  uci:commit("clash")
  SYS.call("/etc/init.d/clash stop >/dev/null 2>&1 &")
end

return m, ful

