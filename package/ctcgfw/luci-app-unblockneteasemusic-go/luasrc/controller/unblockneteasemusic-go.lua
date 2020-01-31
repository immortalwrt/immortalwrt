-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.unblockneteasemusic-go", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/unblockneteasemusic-go") then
		return
	end

	entry({"admin", "services", "unblockneteasemusic-go"},firstchild(), _("解除网易云音乐播放限制 (Golang)"), 50).dependent = false

	entry({"admin", "services", "unblockneteasemusic-go", "general"},cbi("unblockneteasemusic-go"), _("基本设定"), 1)
	entry({"admin", "services", "unblockneteasemusic-go", "log"},form("unblockneteasemusicgo_log"), _("日志"), 2)

	entry({"admin", "services", "unblockneteasemusic-go", "status"},call("act_status")).leaf=true
end

function act_status()
	local e={}
	e.running=luci.sys.call("pidof UnblockNeteaseMusic >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end
