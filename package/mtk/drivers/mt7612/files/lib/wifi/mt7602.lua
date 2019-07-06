#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>


local function esc(x)
   return (x:gsub('%%', '%%%%')
            :gsub('^%^', '%%^')
            :gsub('%$$', '%%$')
            :gsub('%(', '%%(')
            :gsub('%)', '%%)')
            :gsub('%.', '%%.')
            :gsub('%[', '%%[')
            :gsub('%]', '%%]')
            :gsub('%*', '%%*')
            :gsub('%+', '%%+')
            :gsub('%-', '%%-')
            :gsub('%?', '%%?'))
end


function add_vif_into_lan(vif)
    local mtkwifi = require("mtkwifi")
    local brvifs = mtkwifi.__trim(
        mtkwifi.read_pipe("uci get network.lan.ifname"))
    if not string.match(brvifs, esc(vif)) then
        nixio.syslog("debug", "add "..vif.." into lan")
        brvifs = brvifs.." "..vif
        os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
        os.execute("uci commit")
        -- os.execute("brctl addif br-lan "..vif)
        os.execute("ubus call network.interface.lan add_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
    end
end

function mt7602_up(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7602_up called!")

	-- typically mt7602 is the 1st card and it takes "ra" and "apcli" prefix.
	-- for multi-bssid, we must bring up ra0 first. ra0 will create ra1, ra2,...
	if not mtkwifi.exists("/sys/class/net/ra0") then
		nixio.syslog("error", "unable to detect ra0, abort!")
		return
	end
	os.execute("ifconfig ra0 up")
	add_vif_into_lan("ra0")
	-- then we bring up ra1, ra2,...
	for _,vif in mtkwifi.__spairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
	do
		if string.match(vif, "ra%d+") then
			os.execute("ifconfig "..vif.." up")
			add_vif_into_lan(vif)
		-- else nixio.syslog("debug", "skip "..vif..", prefix not match "..pre[1])
		end
	end

	-- then we bring up apcli0, apcli1,...
	for _,vif in mtkwifi.__spairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
	do
		if string.match(vif, "apcli%d+") then
			os.execute("ifconfig "..vif.." up")
		end
	end

	os.execute(" rm -rf /tmp/mtk/wifi/mt7602*.need_reload")
end

function mt7602_down(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7602_down called!")

	for _,vif in mtkwifi.__spairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
	do
		if string.match(vif, "apcli%d+") then
			os.execute("ifconfig "..vif.." down")
		end
	end

	for _,vif in mtkwifi.__spairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
	do
		if string.match(vif, "ra%d+") then
			os.execute("ifconfig "..vif.." down")
			local brvifs = mtkwifi.read_pipe("uci get network.lan.ifname")
			if string.match(brvifs, vif) then
				brvifs = mtkwifi.__trim(string.gsub(brvifs, vif, ""))
				nixio.syslog("debug", "remove "..vif.." from lan")
				os.execute("uci set network.lan.ifname=\""..brvifs.."\"")
				os.execute("uci commit")
				os.execute("ubus call network.interface.lan remove_device \"{\\\"name\\\":\\\""..vif.."\\\"}\"")
			end
		-- else nixio.syslog("debug", "skip "..vif..", prefix not match "..pre[1])
		end
	end

	os.execute(" rm -rf /tmp/mtk/wifi/mt7602*.need_reload")
end

function mt7602_reload()
	local nixio = require("nixio")
	nixio.syslog("debug", "mt7602_reload called!")
	mt7602_down()
	mt7602_up()
end

function mt7602_restart()
	local nixio = require("nixio")
	nixio.syslog("debug", "mt7602_restart called!")
	mt7602_down()
	-- 7602 shares driver with 7612
	os.execute("rmmod mt7612")
	os.execute("modprobe mt7612")
	mt7602_up()
end

function mt7602_reset(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7602_reset called!")
	if mtkwifi.exists("/rom/etc/wireless/mt7602/") then
		os.execute("rm -rf /etc/wireless/mt7602/")
		os.execute("cp -rf /rom/etc/wireless/mt7602/ /etc/wireless/")
		mt7602_reload()
	else
		nixio.syslog("debug", "/rom"..profile.." missing, unable to reset!")
	end
end

function mt7602_status()
	return wifi_common_status()
end

function mt7602_detect(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7602_detect called!")

	for _,dev in ipairs(mtkwifi.get_all_devs()) do
		print([[
config wifi-device ]]..dev.maindev.."\n"..[[
	option type mt7602
	option vendor ralink
]])
		for _,vif in ipairs(dev.vifs) do
			print([[
config wifi-iface
	option device ]]..dev.maindev.."\n"..[[
	option ifname ]]..vif.vifname.."\n"..[[
	option network lan
	option mode ap
	option ssid ]]..vif.__ssid.."\n")
		end
	end
end
