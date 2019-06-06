require("luci.sys")
require("luci.util")
require("luci.model.ipkg")
local fs  = require "nixio.fs"

local uci = require "luci.model.uci".cursor()

local m, s

local running=(luci.sys.call("pidof baidupcs > /dev/null") == 0)

local button = ""
local state_msg = ""
local trport = uci:get_first("baidupcs", "config", "port") or 5299
if running  then
	button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"button\" value=\" " .. translate("Open Baidupcs Web Interface") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. trport .. "')\"/>"
end

if running then
        state_msg = "<b><font color=\"green\">" .. translate("Baidupcs running") .. "</font></b>"
else
        state_msg = "<b><font color=\"red\">" .. translate("Baidupcs not running") .. "</font></b>"
end

m = Map("baidupcs", translate("Baidupcs"), translate("Baidupcs is a simple web base baidu netdisk, here you can configure the settings.") .. button
        .. "<br/><br/>" .. translate("Baidupcs Run Status").. " : "  .. state_msg .. "<br/>")
        
s = m:section(TypedSection, "baidupcs", "")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"))
enable.rmempty = false

port=s:option(Value, "port", translate("listen port"))
path=s:option(Value, "path", translate("path"))
return m
