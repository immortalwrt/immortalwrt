local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local fs = require "luci.clash"
local clash = "clash"


m = Map("clash")
s = m:section(TypedSection, "clash")
s.anonymous = true
m.pageaction = false

o = s:option(Flag, "enable", translate("Enable"))
o.default = 0
o.rmempty = false
o.description = translate("Enable")



o = s:option(ListValue, "core", translate("Core"))
o.default = "clashcore"
if nixio.fs.access("/etc/clash/clash") then
o:value("1", translate("Clash"))
end
if nixio.fs.access("/usr/bin/clash") then
o:value("2", translate("Clashr"))
end
if nixio.fs.access("/etc/clash/clashtun/clash") then
o:value("3", translate("ClashTun"))
luci.sys.exec('uci set clash.config.mode="0"')
luci.sys.exec('uci commit clash')
end
o.description = translate("Select core, clashr support ssr while clash does not.")


o = s:option(ListValue, "tun_mode", translate("Tun Mode"))
o.default = "1"
o:value("0", translate("Disable"))
o:value("1", translate("Fake-IP(Dreamacro Tun)"))
o:value("2", translate("Fake-IP(comzyh Tun)"))
o:value("3", translate("Redir-Host(comzyh Tun)"))
o.description = translate("Select Tun Mode")
o:depends("core",3)

o = s:option(ListValue, "config_type", translate("Config Type"))
o.default = "sub"
o:value("sub", translate("Subscription Config"))
o:value("upl", translate("Uploaded Config"))
o:value("cus", translate("Custom Config"))
o.description = translate("Select Configuration type")

o = s:option(Button, "Apply")
o.title = translate("Save & Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  	m.uci:commit("clash")
end

o = s:option(Button,"action")
o.title = translate("Action")
o.template = "clash/start_stop"





return m

