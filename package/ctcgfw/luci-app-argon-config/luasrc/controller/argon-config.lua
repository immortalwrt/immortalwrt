--[[
luci-app-argon-config
]]--

module("luci.controller.argon-config", package.seeall)

function index()
	if not nixio.fs.access('/etc/config/argon') then
		return
	end

	local page
	page = entry({"admin", "system", "argon-config"}, form("argon-config/configuration"), _("Argon Config"), 90)
	page.acl_depends = { "luci-app-argon-config" }
end
