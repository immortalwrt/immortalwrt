-- This is a free software, use it under GNU General Public License v3.0.
-- Created By [CTCGFW]Project OpenWRT
-- https://github.com/project-openwrt

module("luci.controller.unblockneteasemusic", package.seeall)

function index()
	if not nixio.fs.access("/etc/config/unblockneteasemusic") then
		return
	end

	entry({"admin", "services", "unblockneteasemusic"},firstchild(), _("解除网易云音乐播放限制"), 50).dependent = false

	entry({"admin", "services", "unblockneteasemusic", "general"},cbi("unblockneteasemusic"), _("基本设定"), 1)
	entry({"admin", "services", "unblockneteasemusic", "upgrade_core"},form("unblockneteasemusic_upcore"), _("更新核心"), 2).leaf = true
	entry({"admin", "services", "unblockneteasemusic", "log"},form("unblockneteasemusiclog"), _("日志"), 3)

	entry({"admin", "services", "unblockneteasemusic", "status"},call("act_status")).leaf=true
	entry({"admin", "services", "unblockneteasemusic", "update_core"},call("act_update_core"))
end

function act_status()
	local e={}
	e.running=luci.sys.call("ps |grep unblockneteasemusic |grep app.js |grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function update_core()
	cloud_ver=luci.sys.exec("curl -s 'https://github.com/nondanee/UnblockNeteaseMusic/commits/master' |tr -d '\n' |grep -Eo 'commit\/[0-9a-z]+' |sed -n 1p |sed 's#commit/##g'")
	cloud_ver_mini=luci.sys.exec("curl -s 'https://github.com/nondanee/UnblockNeteaseMusic/commits/master' |tr -d '\n' |grep -Eo 'BtnGroup-item.>      [0-9a-z]+' |sed -n 1p |sed 's#BtnGroup-item.>      ##g'")
	if not cloud_ver or not cloud_ver_mini then
		return "1"
	else
		local_ver=luci.sys.exec("cat '/usr/share/unblockneteasemusic/local_ver'")
		if not local_ver or (local_ver ~= cloud_ver) then
			luci.sys.call("rm -f /usr/share/unblockneteasemusic/update_successfully")
			luci.sys.call("/bin/bash /usr/share/unblockneteasemusic/update_core.sh luci_update")
			if not nixio.fs.access("/usr/share/unblockneteasemusic/update_successfully") then
				return "2"
			else
				luci.sys.call("rm -f /usr/share/unblockneteasemusic/update_successfully")
				return cloud_ver_mini
			end
		else
			return "0"
		end
	end
end

function act_update_core()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		ret = update_core();
	})
end
