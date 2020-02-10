-- Copyright 2008 Yanira <forum-2008@email.de>
-- Copyright 2020 KFERMercer <KFER.Mercer@gmail.com>
-- Copyright 2020 [CTCGFW]Project-OpenWrt
-- Licensed to the public under the GPL v3.0.

local uci = require "luci.model.uci".cursor()
local running=(luci.sys.call("pgrep verysync >/dev/null") == 0)
local button = ""
local trport = uci:get("verysync", "config", "port")
if running then
	button = "<br/><br/><input type=\"button\" value=\" " .. translate("Open Verysync page") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. trport .. "')\"/>"
end

m = Map("verysync", translate("Verysync"), translate("Simple and easy-to-use multi-platform file synchronization software, astonishing transmission speed is different from the greatest advantage of other products. Micro-force synchronization of intelligent P2P technology to accelerate synchronization, will split the file into several KB-only data synchronization, and the file will be AES encryption processing.") .. button)

m:section(SimpleSection).template  = "verysync/verysync_status"

s = m:section(TypedSection, "verysync")
s.addremove = false
s.anonymous = true

o = s:option(Flag, "enabled", translate("Enable"))
o.rmempty = false

o = s:option(Value, "port", translate("Port"))
o.datatype = "port"
o.placeholder = "8886"
o.default = "8886"
o.rmempty = false

return m
