require("luci.sys")
require("luci.util")
require("luci.model.ipkg")
local fs  = require "nixio.fs"

local uci = require "luci.model.uci".cursor()

local m, s

local running=(luci.sys.call("pidof filebrowser > /dev/null") == 0)

local button = ""
local state_msg = ""
local trport = uci:get_first("filebrowser", "config", "port") or 8989
if running  then
	button = "&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<input type=\"button\" value=\" " .. translate("Open FileBrowser Web Interface") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. trport .. "')\"/>"
end

if running then
        state_msg = "<b><font color=\"green\">" .. translate("FileBrowser running") .. "</font></b>"
else
        state_msg = "<b><font color=\"red\">" .. translate("FileBrowser not running") .. "</font></b>"
end

m = Map("filebrowser", translate("FileBrowser"), translate("FileBrowser is a simple web base file browser, here you can configure the settings.") .. button
        .. "<br/><br/>" .. translate("FileBrowser Run Status").. " : "  .. state_msg .. "<br/>")
        
s = m:section(TypedSection, "filebrowser", "")
s.addremove = false
s.anonymous = true

enable = s:option(Flag, "enabled", translate("Enable"))
enable.rmempty = false

port=s:option(Value, "port", translate("listen port"))
path=s:option(Value, "path", translate("path"))
scope_dir=s:option(Value, "scope", translate("scope dir"))
return m
