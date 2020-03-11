--Copyright (C) 2019 mleaf <mleaf90@gmail.com>
--微信公众号【WiFi物联网】
module("luci.controller.mwol",package.seeall)

function index()
	if not nixio.fs.access("/etc/config/mwol") then
		return
	end
	local page = entry({"admin", "services", "mwol"}, cbi("mwol"), _("Mwol"))
	page.dependent = true
end
