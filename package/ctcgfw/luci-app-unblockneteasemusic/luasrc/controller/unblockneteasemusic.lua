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
	entry({"admin", "services", "unblockneteasemusic", "upgrade"},form("unblockneteasemusic_upgrade"), _("更新组件"), 2).leaf = true
	entry({"admin", "services", "unblockneteasemusic", "log"},form("unblockneteasemusiclog"), _("日志"), 3)

	entry({"admin", "services", "unblockneteasemusic", "status"},call("act_status")).leaf=true
	entry({"admin", "services", "unblockneteasemusic", "update_luci"},call("act_update_luci"))
	entry({"admin", "services", "unblockneteasemusic", "update_core"},call("act_update_core"))
end

function act_status()
	local e={}
	e.running=luci.sys.call("ps |grep unblockneteasemusic |grep app.js |grep -v grep >/dev/null")==0
	luci.http.prepare_content("application/json")
	luci.http.write_json(e)
end

function update_luci()
	luci_cloud_ver=luci.sys.exec("curl -s 'https://github.com/project-openwrt/luci-app-unblockneteasemusic/releases/latest'| grep -Eo '[0-9\.]+\-[0-9]+'")
	if not luci_cloud_ver then
		return "1"
	else
		luci_local_ver=luci.sys.exec("opkg info 'luci-app-unblockneteasemusic' |sed -n '2p' |tr -d 'Version: '")
		if not luci_local_ver or (luci_local_ver ~= luci_cloud_ver) then
			luci.sys.call("rm -f /usr/share/unblockneteasemusic/update_luci_successfully")
			luci.sys.call("/bin/bash /usr/share/unblockneteasemusic/update.sh update_luci")
			if not nixio.fs.access("/usr/share/unblockneteasemusic/update_luci_successfully") then
				return "2"
			else
				luci.sys.call("rm -f /usr/share/unblockneteasemusic/update_luci_successfully")
				return luci_cloud_ver
			end
		else
			return "0"
		end
	end
end

function act_update_luci()
	luci.http.prepare_content("application/json")
	luci.http.write_json({
		ret = update_luci();
	})
end

function update_core()
	core_cloud_ver=luci.sys.exec("curl -s 'https://github.com/nondanee/UnblockNeteaseMusic/commits/master' |tr -d '\n' |grep -Eo 'commit\/[0-9a-z]+' |sed -n 1p |sed 's#commit/##g'")
	core_cloud_ver_mini=luci.sys.exec("curl -s 'https://github.com/nondanee/UnblockNeteaseMusic/commits/master' |tr -d '\n' |grep -Eo 'BtnGroup-item.>      [0-9a-z]+' |sed -n 1p |sed 's#BtnGroup-item.>      ##g'")
	if not core_cloud_ver or not core_cloud_ver_mini then
		return "1"
	else
		core_local_ver=luci.sys.exec("cat '/usr/share/unblockneteasemusic/core_local_ver'")
		if not core_local_ver or (core_local_ver ~= core_cloud_ver) then
			luci.sys.call("rm -f /usr/share/unblockneteasemusic/update_core_successfully")
			luci.sys.call("/bin/bash /usr/share/unblockneteasemusic/update.sh update_core_from_luci")
			if not nixio.fs.access("/usr/share/unblockneteasemusic/update_core_successfully") then
				return "2"
			else
				luci.sys.call("rm -f /usr/share/unblockneteasemusic/update_core_successfully")
				return core_cloud_ver_mini
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