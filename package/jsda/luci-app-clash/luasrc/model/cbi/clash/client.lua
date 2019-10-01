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

o = s:option(Flag, "enable", translate("Enable"))
o.default = 0
o.rmempty = false
o.description = translate("Enable")


o = s:option(ListValue, "config_type", translate("Config Type"))
o.default = "sub"
o:value("sub", translate("Subscription Config"))
o:value("upl", translate("Uploaded Config"))
o:value("cus", translate("Custom Config"))
o.description = translate("Select Configuration type")



local apply = luci.http.formvalue("cbi.apply")
if apply then
    uci:commit("clash")
	os.execute("/etc/init.d/clash restart >/dev/null 2>&1 &")
end

return m

