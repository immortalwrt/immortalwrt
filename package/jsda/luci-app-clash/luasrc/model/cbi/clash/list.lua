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
--m.pageaction = false
s.anonymous = true


y = s:option(ListValue, "cus_list", translate("Status"))
y.default = 0
y:value("0", translate("disabled"))
y:value("1", translate("enabled"))



o = s:option(Value, "custom_dns", translate("Custom List DNS"))
o:value("114.114.114.114", "114.114.114.114")
o:value("114.114.115.115", "114.114.115.115")
o:value("119.29.29.29", "119.29.29.29")
o:value("4.2.2.1", "4.2.2.1")
o:value("4.2.2.2", "4.2.2.2")
o:value("4.2.2.3", "4.2.2.3")
o:value("4.2.2.4", "4.2.2.4")
o:depends("cus_list", "1")



local conffile = "/usr/share/clash/server.list"
sev = s:option(TextValue, "conffile")
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conffile) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(conffile, value:gsub("\r\n", "\n"))
end



local apply = luci.http.formvalue("cbi.apply")
if apply then
	uci:commit("clash")
	if luci.sys.call("pidof clash >/dev/null") == 0 then
	SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
	end
end

return m
