#!/usr/bin/lua
-- Alternative for OpenWrt's /sbin/wifi.
-- Copyright Not Reserved.
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path

local dbdc_init_ifname = "ra0"
-- initial ifname after load mtwifi kmod driver

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
    local brvifs = string.split(mtkwifi.__trim((mtkwifi.read_pipe("ls /sys/class/net/br-lan/brif/"))))

    for _,v in ipairs(brvifs) do
        if v == vif then
            nixio.syslog("debug", vif.." is already added into lan")
            return
        end
    end

    nixio.syslog("debug", "add "..vif.." into lan")
    os.execute("brctl addif br-lan "..vif)
	if mtkwifi.exists("/proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6") then
        os.execute("echo 1 > /proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6")
	end

    brvifs = string.split(mtkwifi.__trim((mtkwifi.read_pipe("ls /sys/class/net/br-lan/brif/"))))
    for _,v in ipairs(brvifs) do
        nixio.syslog("debug", "brvif = "..v)
    end
end

function del_vif_from_lan(vif)
    local mtkwifi = require("mtkwifi")

    if mtkwifi.exists("/sys/class/net/br-lan/brif/") == false then
        return
    end

    local brvifs = string.split(mtkwifi.__trim((mtkwifi.read_pipe("ls /sys/class/net/br-lan/brif/"))))
    for _,v in ipairs(brvifs) do
        if v == vif then
            nixio.syslog("debug", "del "..vif.." from lan")
            if mtkwifi.exists("/proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6") then
                os.execute("echo 0 > /proc/sys/net/ipv6/conf/"..vif.."/disable_ipv6")
            end
            os.execute("brctl delif br-lan "..vif)
        end
    end
    brvifs = string.split(mtkwifi.__trim((mtkwifi.read_pipe("ls /sys/class/net/br-lan/brif/"))))
    for _,v in ipairs(brvifs) do
        nixio.syslog("debug", "brvif = "..v)
    end
end

function d8021xd_chk(devname, prefix, vif, enable)
    local mtkwifi = require("mtkwifi")
    if mtkwifi.exists("/tmp/run/8021xd_"..vif..".pid") then
        os.execute("cat /tmp/run/8021xd_"..vif..".pid | xargs kill -9")
        os.execute("rm /tmp/run/8021xd_"..vif..".pid")
        nixio.syslog("debug", "mtwifi: stop 8021xd")
    end

    if enable and mtkwifi.exists("/usr/bin/8021xd") then
        local profile = mtkwifi.search_dev_and_profile()[devname]
        local cfgs = mtkwifi.load_profile(profile)
        local auth_mode = cfgs.AuthMode
        local ieee8021x = cfgs.IEEE8021X
        local pat_auth_mode = {"WPA$", "WPA;", "WPA2$", "WPA2;", "WPA1WPA2$", "WPA1WPA2;", "WPA3$", "WPA3;", "192$", "192;", "WPA2-Ent-OSEN$", "WPA2-Ent-OSEN;"}
        local pat_ieee8021x = {"1$", "1;"}
        local apd_en = false

        for _, pat in ipairs(pat_auth_mode) do
            if string.find(auth_mode, pat) then
                apd_en = true
            end
        end

        for _, pat in ipairs(pat_ieee8021x) do
            if string.find(ieee8021x, pat) then
                apd_en = true
            end
        end

        if apd_en then
            nixio.syslog("debug", "mtwifi: start 8021xd")
            os.execute("8021xd -p "..prefix.. " -i "..vif)
        end
    end
end

function __exec_iwpriv_cmd(ifname, key, val)
    local cmd = string.format("iwpriv %s set %s=%s", ifname, key, tostring(val))
    nixio.syslog("info", "wifi-profile: iwpriv cmd: "..cmd)
    os.execute(cmd)
end

function mtwifi_iwpriv_hook(devname)
    local mtkwifi = require("mtkwifi")
    local devs = mtkwifi.get_all_devs()
    local dev = devs[devname]

    if dev then
        for _,vif in ipairs(dev.vifs) do
            __exec_iwpriv_cmd(vif.vifname, "KickStaRssiLow", vif.__kickrssi or "0")
            __exec_iwpriv_cmd(vif.vifname, "AssocReqRssiThres", vif.__assocthres or "0")
        end
    end
end

function mtwifi_up(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")

    nixio.syslog("debug", "mtwifi called!")

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mtwifi: dev "..devname.." not found!")
            return
        end
        local profile = mtkwifi.search_dev_and_profile()[devname]
        local cfgs = mtkwifi.load_profile(profile)

        if string.find(dev.profile_path, "dbdc") then
            -- for dbdc mode, first bring up dbdc_init_ifname, it will create all other vifs
            if  dev.main_ifname == dbdc_init_ifname then
                -- current main ifname = ra0
                if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
                    nixio.syslog("info", "mtwifi_up: dbdc init: ifconfig "..dbdc_init_ifname.." up")
                    os.execute("ifconfig "..dbdc_init_ifname.." up")
                else
                    nixio.syslog("err", "mtwifi_up: dbdc init: "..dbdc_init_ifname.." missing, quit!")
                    return
                end
            end
            if cfgs.DevEnable ~= nil and cfgs.DevEnable == "0" then
                nixio.syslog("info", "mtwifi_up: "..devname.." is disabled")
                if dev.main_ifname == dbdc_init_ifname then
                    -- current main ifname = ra0, down ra0
                    os.execute("ifconfig "..dbdc_init_ifname.." down")
                end
                return
            end
            if dev.main_ifname ~= dbdc_init_ifname then
                if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
                    nixio.syslog("info", "mtwifi_up: ifconfig "..dev.main_ifname.." up")
                    os.execute("ifconfig "..dev.main_ifname.." up")
                else
                    nixio.syslog("err", "mtwifi_up: main_ifname "..dev.main_ifname.." missing, quit!")
                    return
                end
            end
            add_vif_into_lan(dev.main_ifname)
        else
            -- for non-dbdc mode,
            -- we have to bring up main_ifname first, main_ifname will create all other vifs.
            if mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
                if cfgs.DevEnable ~= nil and cfgs.DevEnable == "0" then
                    nixio.syslog("info", "mtwifi_up: "..devname.." is disabled")
                    return
                end
                nixio.syslog("info", "mtwifi_up: ifconfig "..dev.main_ifname.." up")
                os.execute("ifconfig "..dev.main_ifname.." up")
                add_vif_into_lan(dev.main_ifname)
            else
                nixio.syslog("err", "mtwifi_up: main_ifname "..dev.main_ifname.." missing, quit!")
                return
            end
        end
        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif ~= dev.main_ifname and
            (  string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or (string.match(vif, esc(dev.apcli_ifname).."[0-9]+") and
                cfgs.ApCliEnable ~= "0" and cfgs.ApCliEnable ~= "")
            or (string.match(vif, esc(dev.wds_ifname).."[0-9]+") and
                cfgs.WdsEnable ~= "0" and cfgs.WdsEnable ~= "")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+"))
            then
                nixio.syslog("debug", "mtwifi_up: ifconfig "..vif.." up")
                os.execute("ifconfig "..vif.." up")
            end

            if vif ~= dev.main_ifname and (string.match(vif, esc(dev.ext_ifname).."[0-9]+")) then
                -- add ra1..rax1 to br-lan
                add_vif_into_lan(vif)
            end

            if string.match(vif, esc(dev.apcli_ifname).."[0-9]+") and
                cfgs.ApCliEnable ~= "0" and cfgs.ApCliEnable ~= "" then
                -- enable apcli auto connect by default
                -- ApCliAutoConnect: 1=User Trigger Scan Mode 2=Partial Scan Mode 3=Driver Trigger Scan ModeW
                os.execute("iwpriv "..vif.." set ApCliEnable=1")
                os.execute("iwpriv "..vif.." set ApCliAutoConnect=3")
            end
        end
        d8021xd_chk(devname, dev.ext_ifname, dev.main_ifname, true)
    else
        nixio.syslog("debug", "mtwifi_up: skip "..devname..", config(l1profile) not exist")
    end

    mtwifi_iwpriv_hook(devname)

    os.execute(" rm -rf /tmp/mtk/wifi/mtwifi*.need_reload")
end

function mtwifi_down(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")

    nixio.syslog("debug", "mtwifi_down called!")

    -- M.A.N service
    if mtkwifi.exists("/etc/init.d/man") then
        os.execute("/etc/init.d/man stop")
    end

    local devs, l1parser = mtkwifi.__get_l1dat()
    -- l1 profile present, good!
    if l1parser and devs then
        dev = devs.devname_ridx[devname]
        if not dev then
            nixio.syslog("err", "mtwifi_down: dev "..devname.." not found!")
            return
        end
        if not mtkwifi.exists("/sys/class/net/"..dev.main_ifname) then
            -- nixio.syslog("err", "mtwifi_down: main_ifname "..dev.main_ifname.." missing, quit!")
            return
        end
        -- hw_nat_register is only compatible with legacy WHNAT
        -- os.execute("iwpriv "..dev.main_ifname.." set hw_nat_register=0")
        d8021xd_chk(devname, dev.ext_ifname, dev.main_ifname, false)
        for _,vif in ipairs(string.split(mtkwifi.read_pipe("ls /sys/class/net"), "\n"))
        do
            if vif == dev.main_ifname
            or string.match(vif, esc(dev.ext_ifname).."[0-9]+")
            or string.match(vif, esc(dev.apcli_ifname).."[0-9]+")
            or string.match(vif, esc(dev.wds_ifname).."[0-9]+")
            or string.match(vif, esc(dev.mesh_ifname).."[0-9]+")
            then
                nixio.syslog("debug", "mtwifi_down: ifconfig "..vif.." down")
                os.execute("ifconfig "..vif.." down")
                del_vif_from_lan(vif)
            -- else nixio.syslog("debug", "mtwifi_down: skip "..vif..", prefix not match "..pre)
            end
        end
    else
        nixio.syslog("debug", "mtwifi_down: skip "..devname..", config not exist")
    end

    os.execute(" rm -rf /tmp/mtk/wifi/mtwifi*.need_reload")
end

function mtwifi_reload(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    local normal_reload = true
    local qsetting = false
    local path, profiles
    local devs, l1parser = mtkwifi.__get_l1dat()
    nixio.syslog("debug", "mtwifi_reload called!")

    if mtkwifi.exists("/lib/wifi/quick_setting.lua") then
        qsetting = true
        profiles = mtkwifi.search_dev_and_profile()
    end

    -- For one card , all interface should be down, then up
    if not devname then
        for devname, dev in pairs(devs.devname_ridx) do
                mtwifi_down(devname)
        end
        for devname, dev in mtkwifi.__spairs(devs.devname_ridx) do
            if qsetting then
                -- Create devname.last for quick setting
                path = profiles[devname]
                if not mtkwifi.exists("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".applied") then
                    os.execute("cp -f "..path.." "..mtkwifi.__profile_previous_settings_path(path))
                else
                    os.execute("cp -f "..mtkwifi.__profile_applied_settings_path(path)..
                        " "..mtkwifi.__profile_previous_settings_path(path))
                end
            end
            mtwifi_up(devname)
        end
    else
        if qsetting then
            path = profiles[devname]
            normal_reload = quick_settings(devname, path)
        end

        if normal_reload then
            local dev = devs.devname_ridx[devname]
            assert(mtkwifi.exists(dev.init_script))
            local compatname = dev.init_compatible
            -- Different cards do not affect each other
            if not string.find(dev.profile_path, "dbdc") then
                if dev.init_compatible == compatname then
                    mtwifi_down(devname)
                    mtwifi_up(devname)
                end
            --If the reloaded device belongs to dbdc, then another device on dbdc also need to be reloaded
            else
                for devname, dev in pairs(devs.devname_ridx) do
                    if dev.init_compatible == compatname then
                        mtwifi_down(devname)
                    end
                end
                for devname, dev in mtkwifi.__spairs(devs.devname_ridx) do
                    if dev.init_compatible == compatname then
                        mtwifi_up(devname)
                    end
                end
            end
        else
            mtwifi_up(devname)
        end
    end
    -- for ax7800 project, close the ra0.
    if string.find(dev.profile_path, "ax7800") then
    	os.execute("ifconfig ra0 down")
    end
end

function mtwifi_restart(devname)
    local nixio = require("nixio")
    local uci  = require "luci.model.uci".cursor()
    local mtkwifi = require("mtkwifi")
    local devs, l1parser = mtkwifi.__get_l1dat()

    nixio.syslog("debug", "mtwifi_restart called!")

    -- if wifi driver is built-in, it's necessary action to reboot the device
    if mtkwifi.exists("/sys/module/mt_wifi") == false then
        os.execute("echo reboot_required > /tmp/mtk/wifi/reboot_required")
        return
    end

    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mtwifi_down(devname)
            end
        end
    else
         for devname, dev in pairs(devs.devname_ridx) do
             mtwifi_down(devname)
         end
    end
    os.execute("rmmod mt_whnat")
    if mtkwifi.exists("/etc/init.d/fwdd") then
        os.execute("/etc/init.d/fwdd stop")
    end
    os.execute("rmmod mtfwd")
    os.execute("rmmod mtk_warp_proxy")
    os.execute("rmmod mtk_warp")
    -- mt7915_mt_wifi is for dual ko only
    os.execute("rmmod mt7915_mt_wifi")
    os.execute("rmmod mt_wifi")

    os.execute("modprobe mt_wifi")
    os.execute("modprobe mt7915_mt_wifi")
    os.execute("modprobe mtk_warp")
    os.execute("modprobe mtk_warp_proxy")
    os.execute("modprobe mtfwd")
    if mtkwifi.exists("/etc/init.d/fwdd") then
        os.execute("/etc/init.d/fwdd start")
    end
    os.execute("modprobe mt_whnat")
    if devname then
        local dev = devs.devname_ridx[devname]
        assert(mtkwifi.exists(dev.init_script))
        local compatname = dev.init_compatible
        for devname, dev in pairs(devs.devname_ridx) do
            if dev.init_compatible == compatname then
                mtwifi_up(devname)
            end
        end
    else
        for devname, dev in pairs(devs.devname_ridx) do
            mtwifi_up(devname)
        end
    end
end

function mtwifi_reset(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mtwifi_reset called!")
    if mtkwifi.exists("/rom/etc/wireless/mediatek/") then
        os.execute("rm -rf /etc/wireless/mediatek/")
        os.execute("cp -rf /rom/etc/wireless/mediatek/ /etc/wireless/")
        mtwifi_reload(devname)
    else
        nixio.syslog("debug", "mtwifi_reset: /rom"..profile.." missing, unable to reset!")
    end
end

function mtwifi_status(devname)
    return wifi_common_status()
end

function mtwifi_hello(devname)
   os.execute("echo mtwifi_hello: "..devname)
end


function mtwifi_detect(devname)
    local nixio = require("nixio")
    local mtkwifi = require("mtkwifi")
    nixio.syslog("debug", "mtwifi_detect called!")

    for _,dev in ipairs(mtkwifi.get_all_devs()) do
        local relname = string.format("%s%d%d",dev.maindev,dev.mainidx,dev.subidx)
        print([[
config wifi-device ]]..relname.."\n"..[[
    option type mtwifi
    option vendor ralink
]])
        for _,vif in ipairs(dev.vifs) do
            print([[
config wifi-iface
    option device ]]..relname.."\n"..[[
    option ifname ]]..vif.vifname.."\n"..[[
    option network lan
    option mode ap
    option ssid ]]..vif.__ssid.."\n")
        end
    end
end
