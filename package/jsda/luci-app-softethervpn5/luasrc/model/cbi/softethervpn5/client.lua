-- Copyright (C) 2018 dz <dingzhong110@gmail.com>
-- Licensed to the public under the GNU General Public License v3.

local sys = require "luci.sys"
local fs = require "nixio.fs"
local conffile = "/usr/libexec/softethervpn/vpn_client.config"

f = SimpleForm("client", translate("VPN Client Settings"))

t = f:field(TextValue, "conf")
t.rmempty = true
t.rows = 20
function t.cfgvalue()
	return fs.readfile(conffile) or ""
end

function f.handle(self, state, data)
	if state == FORM_VALID then
		if data.conf then
			fs.writefile(conffile, data.conf:gsub("\r\n", "\n"))
			luci.sys.call("/etc/init.d/softethervpnclient restart")
		end
	end
	return true
end

return f