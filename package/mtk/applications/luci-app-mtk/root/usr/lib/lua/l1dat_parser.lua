#!/usr/bin/env lua

--[[
 * A lua library to manipulate mtk's wifi driver. used in luci-app-mtk.
 *
 * Copyright (C) 2016 MTK <support@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 2.1
 * as published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
]]

local l1dat_parser = {
    L1_DAT_PATH = "/etc/wireless/l1profile.dat",
    IF_RINDEX = "ifname_ridx",
    DEV_RINDEX = "devname_ridx",
    MAX_NUM_APCLI = 1,
    MAX_NUM_WDS = 4,
    MAX_NUM_MESH = 1,
    MAX_NUM_EXTIF = 16,
    MAX_NUM_DBDC_BAND = 2,
}

local l1cfg_options = {
            ext_ifname="",
            apcli_ifname="apcli",
            wds_ifname="wds",
            mesh_ifname="mesh"
      }

function l1dat_parser.__trim(s)
  if s then return (s:gsub("^%s*(.-)%s*$", "%1")) end
end

function l1dat_parser.__cfg2list(str)
    -- delimeter == ";"
    local i = 1
    local list = {}
    for k in string.gmatch(str, "([^;]+)") do
        list[i] = k
        i = i + 1
    end
    return list
end

function l1dat_parser.token_get(str, n, v)
    -- n starts from 1
    -- v is the backup in case token n is nil
    if not str then return v end
    local tmp = l1dat_parser.__cfg2list(str)
    return tmp[tonumber(n)] or v
end

function l1dat_parser.add_default_value(l1cfg)
    for k, v in ipairs(l1cfg) do

        for opt, default in pairs(l1cfg_options) do
            if ( opt == "ext_ifname" ) then
                v[opt] = v[opt] or v["main_ifname"].."_"
            else
                v[opt] = v[opt] or default..k.."_"
            end
        end
    end

    return l1cfg
end

function l1dat_parser.get_value_by_idx(devidx, mainidx, subidx, key)
    --print("Enter l1dat_parser.get_value_by_idx("..devidx..","..mainidx..", "..subidx..", "..key..")<br>")
    if not devidx or not mainidx or not key then return end

    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    local dev_ridx = l1dat_parser.DEV_RINDEX
    local sidx = subidx or 1
    local devname1  = devidx.."."..mainidx
    local devname2  = devidx.."."..mainidx.."."..sidx

    --print("devnam1=", devname1, "devname2=", devname2, "<br>")
    return devs[dev_ridx][devname2] and devs[dev_ridx][devname2][key]
           or devs[dev_ridx][devname1] and devs[dev_ridx][devname1][key]
end

-- path to zone is 1 to 1 mapping
function l1dat_parser.l1_path_to_zone(path)
    --print("Enter l1dat_parser.l1_path_to_zone("..path..")<br>")
    if not path then return end

    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    for _, dev in pairs(devs[l1dat_parser.IF_RINDEX]) do
        if dev.profile_path == path then
            return dev.nvram_zone
        end
    end

    return
end

-- zone to path is 1 to n mapping
function l1dat_parser.l1_zone_to_path(zone)
    if not zone then return end

    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    local plist = {}
    for _, dev in pairs(devs[l1dat_parser.IF_RINDEX]) do
        if dev.nvram_zone == zone then
            if not next(plist) then
                table.insert(plist,dev.profile_path)
            else
                local plist_str = table.concat(plist)
                if not plist_str:match(dev.profile_path) then
                    table.insert(plist,dev.profile_path)
                end
            end
        end
    end

    return next(plist) and plist or nil
end

function l1dat_parser.l1_ifname_to_datpath(ifname)
    if not ifname then return end

    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    local ridx = l1dat_parser.IF_RINDEX
    return devs[ridx][ifname] and devs[ridx][ifname].profile_path
end

function l1dat_parser.l1_ifname_to_zone(ifname)
    if not ifname then return end

    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    local ridx = l1dat_parser.IF_RINDEX
    return devs[ridx][ifname] and devs[ridx][ifname].nvram_zone
end

function l1dat_parser.l1_zone_to_ifname(zone)
    if not zone then return end

    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    if not devs then return end

    local zone_dev
    for _, dev in pairs(devs[l1dat_parser.DEV_RINDEX]) do
        if dev.nvram_zone == zone then
            zone_dev = dev
        end
    end

    if not zone_dev  then
        return nil
    else
        return zone_dev.main_ifname, zone_dev.ext_ifname, zone_dev.apcli_ifname, zone_dev.wds_ifname, zone_dev.mesh_ifname
    end
end

-- input: L1 profile path.
-- output A table, devs, contains
--   1. devs[%d] = table of each INDEX# in the L1 profile
--   2. devs.ifname_ridx[ifname]
--         = table of each ifname and point to relevant contain in dev[$d]
--   3. devs.devname_ridx[devname] similar to devs.ifnameridx, but use devname.
--      devname = INDEX#_value.mainidx(.subidx)
-- Using *_ridx do not need to handle name=k1;k2 case of DBDC card.
function l1dat_parser.load_l1_profile(path)
    local devs = setmetatable({}, {__index=
                     function(tbl, key)
                           local util = require("luci.util")
                           --print("metatable function:", util.serialize_data(tbl), key)
                           --print("-----------------------------------------------")
                           if ( string.match(key, "^%d+")) then
                               tbl[key] = {}
                               return tbl[key]
                           end
                     end
                 })
    local nixio = require("nixio")
    local chipset_num = {}
    local dir = io.popen("ls /etc/wireless/")
    if not dir then return end
    local fd = io.open(path, "r")
    if not fd then return end

    -- convert l1 profile into lua table
    for line in fd:lines() do
        line = l1dat_parser.__trim(line)
        if string.byte(line) ~= string.byte("#") then
            local i = string.find(line, "=")
            if i then
                local k, v, k1, k2
                k = l1dat_parser.__trim( string.sub(line, 1, i-1) )
                v = l1dat_parser.__trim( string.sub(line, i+1) )
                k1, k2 = string.match(k, "INDEX(%d+)_(.+)")
                if k1 then
                    k1 = tonumber(k1) + 1
                    if devs[k1][k2] then
                        nixio.syslog("warning", "skip repeated key"..line)
                    end
                    devs[k1][k2] = v or ""
                else
                    k1 = string.match(k, "INDEX(%d+)")
                    k1 = tonumber(k1) + 1
                    devs[k1]["INDEX"] = v

                    chipset_num[v] = (not chipset_num[v] and 1) or chipset_num[v] + 1
                    devs[k1]["mainidx"] = chipset_num[v]
                end
            --else
            --    nixio.syslog("warning", "skip line without '=' "..line)
            end
        --else
        --    nixio.syslog("warning", "skip comment line "..line)
        end
    end

    l1dat_parser.add_default_value(devs)
    --local util = require("luci.util")
    --local seen2 = {}
    -- print("Before setup ridx", util.serialize_data(devs, seen2))

    -- Force to setup reverse indice for quick search.
    -- Benifit: 
    --   1. O(1) search with ifname, devname
    --   2. Seperate DBDC name=k1;k2 format in the L1 profile into each 
    --      ifname, devname.
    local dbdc_if = {}
    local ridx = l1dat_parser.IF_RINDEX
    local dridx = l1dat_parser.DEV_RINDEX
    local band_num = l1dat_parser.MAX_NUM_DBDC_BAND
    local k, v, dev, i , j, last
    local devname
    devs[ridx] = {}
    devs[dridx] = {}
    for _, dev in ipairs(devs) do
        dbdc_if[band_num] = l1dat_parser.token_get(dev.main_ifname, band_num, nil)
        if dbdc_if[band_num] then
            for i = 1, band_num - 1 do
                dbdc_if[i] = l1dat_parser.token_get(dev.main_ifname, i, nil)
            end
            for i = 1, band_num do 
                devs[ridx][dbdc_if[i]] = {}
                devs[ridx][dbdc_if[i]]["subidx"] = i
                
                for k, v in pairs(dev) do
                    if  k == "INDEX" or k == "EEPROM_offset" or k == "EEPROM_size"
                       or k == "mainidx" then
                        devs[ridx][dbdc_if[i]][k] = v
                    else
                        devs[ridx][dbdc_if[i]][k] = l1dat_parser.token_get(v, i, "")
                    end
                end
                devname = dev.INDEX.."."..dev.mainidx.."."..devs[ridx][dbdc_if[i]]["subidx"]
                devs[dridx][devname] = devs[ridx][dbdc_if[i]]
            end

            local apcli_if, wds_if, ext_if, mesh_if = {}, {}, {}, {}

            for i = 1, band_num do
                ext_if[i] = l1dat_parser.token_get(dev.ext_ifname, i, nil)
                apcli_if[i] = l1dat_parser.token_get(dev.apcli_ifname, i, nil)
                wds_if[i] = l1dat_parser.token_get(dev.wds_ifname, i, nil)
                mesh_if[i] = l1dat_parser.token_get(dev.mesh_ifname, i, nil)
            end

            for i = 1, l1dat_parser.MAX_NUM_EXTIF - 1 do -- ifname idx is from 0
                for j = 1, band_num do
                    devs[ridx][ext_if[j]..i] = devs[ridx][dbdc_if[j]]
                end
            end

            for i = 0, l1dat_parser.MAX_NUM_APCLI - 1 do
                for j = 1, band_num do
                    devs[ridx][apcli_if[j]..i] = devs[ridx][dbdc_if[j]]
                end
            end

            for i = 0, l1dat_parser.MAX_NUM_WDS - 1 do
                for j = 1, band_num do
                    devs[ridx][wds_if[j]..i] = devs[ridx][dbdc_if[j]]
                end
            end

            for i = 0, l1dat_parser.MAX_NUM_MESH - 1 do
                for j = 1, band_num do
                    if mesh_if[j] then
                        devs[ridx][mesh_if[j]..i] = devs[ridx][dbdc_if[j]]
                    end
                end
            end

        else
            devs[ridx][dev.main_ifname] = dev

            devname = dev.INDEX.."."..dev.mainidx
            devs[dridx][devname] = dev

            for i = 1, l1dat_parser.MAX_NUM_EXTIF - 1 do  -- ifname idx is from 0
                devs[ridx][dev.ext_ifname..i] = dev
            end

            for i = 0, l1dat_parser.MAX_NUM_APCLI - 1 do  -- ifname idx is from 0
                devs[ridx][dev.apcli_ifname..i] = dev
            end

            for i = 0, l1dat_parser.MAX_NUM_WDS - 1 do  -- ifname idx is from 0
                devs[ridx][dev.wds_ifname..i] = dev
            end

            for i = 0, l1dat_parser.MAX_NUM_MESH - 1 do  -- ifname idx is from 0
                devs[ridx][dev.mesh_ifname..i] = dev
            end
        end
    end

    fd:close()
    return devs
end

function l1dat_parser.creat_link_for_nvram( )
    local devs = l1dat_parser.load_l1_profile(l1dat_parser.L1_DAT_PATH)
    for devname, dev in pairs(devs.devname_ridx) do
        local dev = devs.devname_ridx[devname]
        profile = dev.profile_path
        os.execute("mkdir -p /tmp/mtk/wifi/")
        if dev.nvram_zone == "dev1" then
            os.execute("ln -sf " ..profile.." /tmp/mtk/wifi/2860")
        elseif dev.nvram_zone == "dev2" then
            os.execute("ln -sf " ..profile.." /tmp/mtk/wifi/rtdev")
        elseif dev.nvram_zone == "dev3" then
            os.execute("ln -sf " ..profile.." /tmp/mtk/wifi/wifi3")
        end
    end
end
return l1dat_parser
