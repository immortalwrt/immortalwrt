-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.233ddns", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/233ddns") then
		return
	end
	local page
	page = entry({"admin", "services", "233ddns"}, cbi("233ddns"), _("233DDNS"), 100)
	page.dependent = true
end