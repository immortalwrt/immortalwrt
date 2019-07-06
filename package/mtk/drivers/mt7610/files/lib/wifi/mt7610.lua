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

function mt7610_up(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7610_up called!")

	-- 7610 is always the 2nd card and it takes "rai" and "apclii" prefix.
	-- for multi-bssid, we must bring up rai0 first. rai0 will create rai1, rai2,...
	if not mtkwifi.exists("/sys/class/net/rai0") then
		nixio.syslog("err", "unable to detect rai0, abort!")
		return
	end
	os.execute("ifconfig rai0 up")
	add_vif_into_lan("rai0")

	-- then we bring up ra1, ra2,...
	for _,vif in mtkwifi.__spairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
	do
		if string.match(vif, "ra[1-9][0-9]*") then
			os.execute("ifconfig "..vif.." up")
			if not string.find(vif, "apcli") then
				add_vif_into_lan(vif)
			end
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

	os.execute(" rm -rf /tmp/mtk/wifi/mt7610*.need_reload")
end

function mt7610_down(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7610_down called!")

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

	os.execute(" rm -rf /tmp/mtk/wifi/mt7610*.need_reload")
end

function mt7610_reload(devname)
	local nixio = require("nixio")
	nixio.syslog("debug", "mt7610_reload called!")
	mt7610_down()
	mt7610_up()
end

function mt7610_restart(devname)
	local nixio = require("nixio")
	nixio.syslog("debug", "mt7610_restart called!")
	mt7610_down()
	os.execute("rmmod mt7610")
	os.execute("modprobe mt7610")
	mt7610_up()
end

function mt7610_reset(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7610_reset called!")
	if mtkwifi.exists("/rom/etc/wireless/mt7610/") then
		os.execute("rm -rf /etc/wireless/mt7610/")
		os.execute("cp -rf /rom/etc/wireless/mt7610/ /etc/wireless/")
		mt7610_reload()
	else
		nixio.syslog("debug", "/rom"..profile.." missing, unable to reset!")
	end
end

function mt7610_status(devname)
	return wifi_common_status()
end

function mt7610_detect(devname)
	local nixio = require("nixio")
	local mtkwifi = require("mtkwifi")
	nixio.syslog("debug", "mt7610_detect called!")

	for _,dev in ipairs(mtkwifi.get_all_devs()) do
		print([[
config wifi-device ]]..dev.maindev.."\n"..[[
	option type mt7610
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
