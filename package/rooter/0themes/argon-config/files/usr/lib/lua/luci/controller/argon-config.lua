--[[
luci-app-argon-config
]]--

module("luci.controller.argon-config", package.seeall)

function index()
	if not nixio.fs.access('/www/luci-static/argon/css/cascade.css') then
        return
    end
	entry({"admin", "theme", "argon-config"}, form("argon-config/configuration"), _("Argon Theme Settings"),30)
end
