#!/usr/bin/env lua

--[[
 * A lua library to manipulate mtk's wifi driver. used in luci-app-mtk.
 *
 * Copyright (C) 2016 Hua Shao <nossiac@163.com>
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

require("datconf")
local inspect = require "inspect"
local ioctl_help = require "ioctl_helper"
local mtkwifi = {}
local logDisable = 0

function debug_write(...)
    -- luci.http.write(...)
    if logDisable == 1 then
         return
    end
    local syslog_msg = "";
    local nargs = select('#',...)

    for n=1, nargs do
      local v = select(n,...)
      if (type(v) == "string" or type(v) == "number") then
        syslog_msg = syslog_msg..v.." ";
      elseif (type(v) == "boolean") then
        if v then
          syslog_msg = syslog_msg.."true ";
        else
          syslog_msg = syslog_msg.."false ";
        end
      elseif (type(v) == "nil") then
        syslog_msg = syslog_msg.."nil ";
      else
        syslog_msg = syslog_msg.."<Non-printable data type = "..type(v).."> ";
      end
    end
    nixio.syslog("debug", syslog_msg)
end

function mtkwifi.get_table_length(T)
    local count = 0
    for _ in pairs(T) do
        count = count + 1
    end
    return count
end

function mtkwifi.get_file_lines(fileName)
    local fd = io.open(fileName, "r")
    if not fd then return end
    local content = fd:read("*all")
    fd:close()
    return mtkwifi.__lines(content)
end

function mtkwifi.__split(s, delimiter)
    if s == nil then s = "0" end
    local result = {};
    for match in (s..delimiter):gmatch("(.-)"..delimiter) do
        table.insert(result, match);
    end
    return result;
end

function string:split(sep)
    local sep, fields = sep or ":", {}
    local pattern = string.format("([^%s]+)", sep)
    self:gsub(pattern, function(c) fields[#fields+1] = c end)
    return fields
end

function mtkwifi.__trim(s)
  if s then return (s:gsub("^%s*(.-)%s*$", "%1")) end
end

function mtkwifi.__handleSpecialChars(s)
    s = s:gsub("\\", "\\\\")
    s = s:gsub("\"", "\\\"")
    return s
end

function mtkwifi.__spairs(t, order)
    -- collect the keys
    local keys = {}
    for k in pairs(t) do keys[#keys+1] = k end
    -- if order function given, sort by it by passing the table and keys a, b,
    -- otherwise just sort the keys 
    --[[
    if order then
        table.sort(keys, function(a,b) return order(t, a, b) end)
        -- table.sort(keys, order)
    else
        table.sort(keys)
    end
    ]]
    table.sort(keys, order)
    -- return the iterator function
    local i = 0
    return function()
        i = i + 1
        if keys[i] then
            return keys[i], t[keys[i]]
        end
    end
end

function mtkwifi.__lines(str)
    local t = {}
    local function helper(line) table.insert(t, line) return "" end
    helper((str:gsub("(.-)\r?\n", helper)))
    return t
end

function mtkwifi.__get_l1dat()
    if not pcall(require, "l1dat_parser") then
        return
    end

    local parser = require("l1dat_parser")
    local l1dat = parser.load_l1_profile(parser.L1_DAT_PATH)

    return l1dat, parser
end

function mtkwifi.sleep(s)
    local ntime = os.clock() + s
    repeat until os.clock() > ntime
end

function mtkwifi.deepcopy(orig)
    local orig_type = type(orig)
    local copy
    if orig_type == 'table' then
        copy = {}
        for orig_key, orig_value in next, orig, nil do
            copy[mtkwifi.deepcopy(orig_key)] = mtkwifi.deepcopy(orig_value)
        end
        setmetatable(copy, mtkwifi.deepcopy(getmetatable(orig)))
    else -- number, string, boolean, etc
        copy = orig
    end
    return copy
end

function mtkwifi.read_pipe(pipe)
    local retry_count = 10
    local fp, txt, err
    repeat  -- fp:read() may return error, "Interrupted system call", and can be recovered by doing it again
        fp = io.popen(pipe)
        txt, err = fp:read("*a")
        fp:close()
        retry_count = retry_count - 1
    until err == nil or retry_count == 0
    return txt
end

function mtkwifi.detect_triband()
    local devs = mtkwifi.get_all_devs()
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1.DEV_RINDEX
    local main_ifname
    local bands = 0
    for _,dev in ipairs(devs) do
        main_ifname = l1dat and l1dat[dridx][dev.devname].main_ifname or dbdc_prefix[mainidx][subidx].."0"
        if mtkwifi.exists("/sys/class/net/"..main_ifname) then
            bands = bands + 1
        end
    end
    return bands
end

function mtkwifi.detect_first_card()
    local devs = mtkwifi.get_all_devs()
    local first_card_profile

        for i,dev in ipairs(devs) do
            first_card_profile = dev.profile
            if i == 1 then break end
        end

        return first_card_profile
end

function mtkwifi.load_profile(path, raw)
    local cfgs = {}

    cfgobj = datconf.openfile(path)
    if cfgobj then
        cfgs = cfgobj:getall()
        cfgobj:close()
    elseif raw then
        cfgs = datconf.parse(raw)
    end

    return cfgs
end

function mtkwifi.save_profile(cfgs, path)

    if not cfgs then
        debug_write("configuration was empty, nothing saved")
        return
    end

    local diff = mtkwifi.diff_profile(path)

    if next(diff) then
        nixio.syslog("info", "mtkwifi wifi profile diff:")
        nixio.syslog("info", inspect(diff))
    end

    -- Keep a backup of last profile settings
    -- if string.match(path, "([^/]+)\.dat") then
       -- os.execute("cp -f "..path.." "..mtkwifi.__profile_previous_settings_path(path))
    -- end
    local datobj = datconf.openfile(path)
    datobj:merge(cfgs)
    datobj:close(true) -- means close and commit

    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.split_profile(path, path_2g, path_5g)
    assert(path)
    assert(path_2g)
    assert(path_5g)
    local cfgs = mtkwifi.load_profile(path)
    local dirty = {
        "Channel",
        "WirelessMode",
        "TxRate",
        "WmmCapable",
        "NoForwarding",
        "HideSSID",
        "IEEE8021X",
        "PreAuth",
        "AuthMode",
        "EncrypType",
        "RekeyMethod",
        "RekeyInterval",
        "PMKCachePeriod",
        "DefaultKeyId",
        "Key{n}Type",
        "HT_EXTCHA",
        "RADIUS_Server",
        "RADIUS_Port",
    }
    local cfg5g = mtkwifi.deepcopy(cfgs)
    for _,v in ipairs(dirty) do
        cfg5g[v] = mtkwifi.token_get(cfgs[v], 1, 0)
        assert(cfg5g[v])
    end
    mtkwifi.save_profile(cfg5g, path_5g)

    local cfg2g = mtkwifi.deepcopy(cfgs)
    for _,v in ipairs(dirty) do
        cfg2g[v] = mtkwifi.token_get(cfgs[v], 1, 0)
        assert(cfg2g[v])
    end
    mtkwifi.save_profile(cfg2g, path_2g)
end

function mtkwifi.merge_profile(path, path_2g, path_5g)
    local cfg2g = mtkwifi.load_profile(path_2g)
    local cfg5g = mtkwifi.load_profile(path_5g)
    local dirty = {
        "Channel",
        "WirelessMode",
        "TxRate",
        "WmmCapable",
        "NoForwarding",
        "HideSSID",
        "IEEE8021X",
        "PreAuth",
        "AuthMode",
        "EncrypType",
        "RekeyMethod",
        "RekeyInterval",
        "PMKCachePeriod",
        "DefaultKeyId",
        "Key{n}Type",
        "HT_EXTCHA",
        "RADIUS_Server",
        "RADIUS_Port",
    }
    local cfgs = mtkwifi.deepcopy(cfg2g)
    for _,v in dirty do
        -- TODO
    end
    mtkwifi.save_profile(cfgs, path)
end

-- update path1 by path2
function mtkwifi.update_profile(path1, path2)
    local cfg1 = datconf.openfile(path1)
    local cfg2 = datconf.openfile(path2)

    cfg1:merge(cfg2:getall())
    cfg1:close(true)
    cfg2:close()
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.__child_info_path()
    local path = "/tmp/mtk/wifi/child_info.dat"
    os.execute("mkdir -p /tmp/mtk/wifi")
    return path
end

function mtkwifi.__profile_previous_settings_path(profile)
    assert(type(profile) == "string")
    local bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)\.dat")..".last"
    os.execute("mkdir -p /tmp/mtk/wifi")
    return bak
end

function mtkwifi.__profile_applied_settings_path(profile)
    assert(type(profile) == "string")
    local bak
    if string.match(profile, "([^/]+)\.dat") then
        os.execute("mkdir -p /tmp/mtk/wifi")
        bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)\.dat")..".applied"
    elseif string.match(profile, "([^/]+)\.txt") then
        os.execute("mkdir -p /tmp/mtk/wifi")
        bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)\.txt")..".applied"
    elseif string.match(profile, "([^/]+)$") then
        os.execute("mkdir -p /tmp/mtk/wifi")
        bak = "/tmp/mtk/wifi/"..string.match(profile, "([^/]+)$")..".applied"
    else
        bak = ""
    end

    return bak
end

function mtkwifi.get_txpwr(devname)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1.DEV_RINDEX
    local devs = mtkwifi.get_all_devs()

    if devs and devs[devname] then
        local ifname = l1dat and l1dat[dridx][devname].main_ifname
        if ifname then
            return c_getTxPower(ifname)['txpower']
        end
    end

    return nil
end

function mtkwifi.get_temp(devname)
    local vif_name = nil
    local devs = mtkwifi.get_all_devs()
    local dev = {}
    dev = devs and devs[devname]

    if dev.apcli and dev.apcli["state"] == "up" then
        vif_name = dev.apcli["vifname"]
    elseif dev.vifs then
        for _,vif in ipairs(dev.vifs) do
            if vif["state"] == "up" then
                vif_name = vif["vifname"]
                break
            end
        end
    end

    if vif_name then
        return c_getTempature(vif_name)['tempature']
    end
        return nil
end

-- if path2 is not given, use backup of path1.
function mtkwifi.diff_profile(path1, path2)
    assert(path1)
    if not path2 then
        path2 = mtkwifi.__profile_applied_settings_path(path1)
        if not mtkwifi.exists(path2) then
            return {}
        end
    end
    assert(path2)

    local cfg1
    local cfg2
    local diff = {}

    cfg1 = mtkwifi.load_profile(path1) or {}
    cfg2 = mtkwifi.load_profile(path2) or {}

    for k,v in pairs(cfg1) do
        if cfg2[k] ~= cfg1[k] then
            diff[k] = {cfg1[k] or "", cfg2[k] or ""}
        end
    end

    for k,v in pairs(cfg2) do
        if cfg2[k] ~= cfg1[k] then
            diff[k] = {cfg1[k] or "", cfg2[k] or ""}
        end
    end

    return diff
end

function mtkwifi.__fork_exec(command)
    if type(command) ~= type("") or command == "" then
        debug_write("__fork_exec : Incorrect command! Expected non-empty string type, got ",type(command))
        nixio.syslog("err", "__fork_exec : Incorrect command! Expected non-empty string type, got "..type(command))
    else
        local nixio = require("nixio")
        -- If nixio.exec() fails, then child process will be reaped automatically and
        -- it will be achieved by ignoring SIGCHLD signal here in parent process!
        if not nixio.signal(17,"ign") then
            nixio.syslog("warning", "__fork_exec : Failed to set SIG_IGN for SIGCHLD!")
            debug_write("__fork_exec : Failed to set SIG_IGN for SIGCHLD!")
        end
        local pid = nixio.fork()
        if pid < 0 then
            nixio.syslog("err", "__fork_exec : [Fork Failure] "..command)
            debug_write("__fork_exec : [Fork Failure] "..command)
        elseif pid == 0 then
            -- change to root dir to flush out any opened directory streams of parent process.
            nixio.chdir("/")

            -- As file descriptors are inherited by child process, all unused file descriptors must be closed.
            -- Make stdin, out, err file descriptors point to /dev/null using dup2.
            -- As a result, it will not corrupt stdin, out, err file descriptors of parent process.
            local null = nixio.open("/dev/null", "w+")
            if null then
                nixio.dup(null, nixio.stderr)
                nixio.dup(null, nixio.stdout)
                nixio.dup(null, nixio.stdin)
                if null:fileno() > 2 then
                    null:close()
                end
            end
            debug_write("__fork_exec : cmd = "..command)
            -- replaces the child process image with the new process image generated by provided command
            nixio.exec("/bin/sh", "-c", command)
            os.exit(true)
        end
    end
end

function mtkwifi.is_child_active()
    local fd = io.open(mtkwifi.__child_info_path(), "r")
    if not fd then
        os.execute("rm -f "..mtkwifi.__child_info_path())
        return false
    end
    local content = fd:read("*all")
    fd:close()
    if not content then
        os.execute("rm -f "..mtkwifi.__child_info_path())
        return false
    end
    local active_pid_list = {}
    for _,pid in ipairs(mtkwifi.__lines(content)) do
        pid = pid:match("CHILD_PID=%s*(%d+)%s*")
        if pid then
            if tonumber(mtkwifi.read_pipe("ps | grep -v grep | grep -cw "..pid)) == 1 then
                table.insert(active_pid_list, pid)
            end
        end
    end
    if next(active_pid_list) ~= nil then
        return true
    else
        os.execute("rm -f "..mtkwifi.__child_info_path())
        return false
    end
    os.execute("sync >/dev/null 2>&1")
end

function mtkwifi.__run_in_child_env(cbFn,...)
    if type(cbFn) ~= "function" then
        debug_write("__run_in_child_env : Function type expected, got ", type(cbFn))
        nixio.syslog("err", "__run_in_child_env : Function type expected, got "..type(cbFn))
    else
        local unpack = unpack or table.unpack
        local cbArgs = {...}
        local nixio = require("nixio")
        -- Let child process reap automatically!
        if not nixio.signal(17,"ign") then
            nixio.syslog("warning", "__run_in_child_env : Failed to set SIG_IGN for SIGCHLD!")
            debug_write("__run_in_child_env : Failed to set SIG_IGN for SIGCHLD!")
        end
        local pid = nixio.fork()
        if pid < 0 then
            debug_write("__run_in_child_env : Fork failure")
            nixio.syslog("err", "__run_in_child_env : Fork failure")
        elseif pid == 0 then
            -- Change to root dir to flush out any opened directory streams of parent process.
            nixio.chdir("/")

            -- As file descriptors are inherited by child process, all unnecessary file descriptors must be closed.
            -- Make stdin, out, err file descriptors point to /dev/null using dup2.
            -- As a result, it will not corrupt stdin, out, err file descriptors of parent process.
            local null = nixio.open("/dev/null", "w+")
            if null then
                nixio.dup(null, nixio.stderr)
                nixio.dup(null, nixio.stdout)
                nixio.dup(null, nixio.stdin)
                if null:fileno() > 2 then
                    null:close()
                end
            end
            local fd = io.open(mtkwifi.__child_info_path(), "a")
            if fd then
                fd:write("CHILD_PID=",nixio.getpid(),"\n")
                fd:close()
            end
            cbFn(unpack(cbArgs))
            os.exit(true)
        end
    end
    os.execute("sync >/dev/null 2>&1")
end

-- Mode 12 and 13 are only available for STAs.
local WirelessModeList = {
    [0] = "B/G mixed",
    [1] = "B only",
    [2] = "A only",
    -- [3] = "A/B/G mixed",
    [4] = "G only",
    -- [5] = "A/B/G/GN/AN mixed",
    [6] = "N in 2.4G only",
    [7] = "G/GN", -- i.e., no CCK mode
    [8] = "A/N in 5 band",
    [9] = "B/G/GN",
    -- [10] = "A/AN/G/GN mode", --not support B mode
    [11] = "only N in 5G band",
    -- [12] = "B/G/GN/A/AN/AC mixed",
    -- [13] = "G/GN/A/AN/AC mixed", -- no B mode
    [14] = "A/AC/AN mixed",
    [15] = "AC/AN mixed", --but no A mode
    [16] = "HE_2G", --HE Wireless Mode
    [17] = "HE_5G", --HE Wireless Mode
    [18] = "HE_6G", --HE Wireless Mode
}

local DevicePropertyMap = {
    -- 2.4G
    {
        device="MT7622",
        band={"0", "1", "4", "9"},
        isPowerBoostSupported=true,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7620",
        band={"0", "1", "4", "9"},
        maxTxStream=2,
        maxRxStream=2,
        maxVif=8
    },

    {
        device="MT7628",
        band={"0", "1", "4", "6", "7", "9"},
        maxTxStream=2,
        maxRxStream=2,
        maxVif=8,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7603",
        band={"0", "1", "4", "6", "7", "9"},
        maxTxStream=2,
        maxRxStream=2,
        maxVif=8,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    -- 5G
    {
        device="MT7612",
        band={"2", "8", "11", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
    },

    {
        device="MT7662",
        band={"2", "8", "11", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
    },

    -- Mix
    {
        device="MT7615",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=8
    },

    {
        device="MT7915",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161}
    },

    {
        device="MT7916",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=2,
        maxRxStream=2,
    },

    {
        device="MT7981",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=2,
        maxRxStream=2,
    },

    {
        device="MT7986",
        band={"0", "1", "4", "9", "2", "8", "14", "15", "16", "17", "18"},
        isPowerBoostSupported=false,
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true,
        maxVif=16,
        maxDBDCVif=16,
        invalidChBwList={161},
        maxTxStream=4,
        maxRxStream=4,
    },

    {
        device="MT7663",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
        invalidChBwList={160,161},
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7613",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=2,
        maxRxStream=2,
        invalidChBwList={160,161},
        isMultiAPSupported=true,
        isWPA3_192bitSupported=true
    },

    {
        device="MT7626",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=3,
        maxRxStream=3,
        invalidChBwList={160,161},
        wdsBand="2.4G",
        mimoBand="5G",
        maxDBDCVif=8
    },

    {
        device="MT7629",
        band={"0", "1", "4", "9", "2", "8", "14", "15"},
        maxTxStream=3,
        maxRxStream=3,
        invalidChBwList={160,161},
        wdsBand="2.4G",
        mimoBand="5G",
        maxDBDCVif=8,
        isMultiAPSupported=true
    }
}

mtkwifi.CountryRegionList_6G_All = {
    {region=0, text="0: Ch1~233"},
    {region=1, text="1: Ch1~97"},
    {region=2, text="2: Ch101~117"},
    {region=3, text="3: Ch121~185"},
    {region=4, text="4: Ch189~233"},
    {region=5, text="5: Ch1~97"},
    {region=6, text="6: Ch1~97"},
    {region=7, text="7: Ch1~97, Ch101~109"},
}

mtkwifi.CountryRegionList_5G_All = {
    {region=0, text="0: Ch36~64, Ch149~165"},
    {region=1, text="1: Ch36~64, Ch100~140"},
    {region=2, text="2: Ch36~64"},
    {region=3, text="3: Ch52~64, Ch149~161"},
    {region=4, text="4: Ch149~165"},
    {region=5, text="5: Ch149~161"},
    {region=6, text="6: Ch36~48"},
    {region=7, text="7: Ch36~64, Ch100~140, Ch149~165"},
    {region=8, text="8: Ch52~64"},
    {region=9, text="9: Ch36~64, Ch100~116, Ch132~140, Ch149~165"},
    {region=10, text="10: Ch36~48, Ch149~165"},
    {region=11, text="11: Ch36~64, Ch100~120, Ch149~161"},
    {region=12, text="12: Ch36~64, Ch100~144"},
    {region=13, text="13: Ch36~64, Ch100~144, Ch149~165"},
    {region=14, text="14: Ch36~64, Ch100~116, Ch132~144, Ch149~165"},
    {region=15, text="15: Ch149~173"},
    {region=16, text="16: Ch52~64, Ch149~165"},
    {region=17, text="17: Ch36~48, Ch149~161"},
    {region=18, text="18: Ch36~64, Ch100~116, Ch132~140"},
    {region=19, text="19: Ch56~64, Ch100~140, Ch149~161"},
    {region=20, text="20: Ch36~64, Ch100~124, Ch149~161"},
    {region=21, text="21: Ch36~64, Ch100~140, Ch149~161"},
    {region=22, text="22: Ch100~140"},
    {region=30, text="30: Ch36~48, Ch52~64, Ch100~140, Ch149~165"},
    {region=31, text="31: Ch52~64, Ch100~140, Ch149~165"},
    {region=32, text="32: Ch36~48, Ch52~64, Ch100~140, Ch149~161"},
    {region=33, text="33: Ch36~48, Ch52~64, Ch100~140"},
    {region=34, text="34: Ch36~48, Ch52~64, Ch149~165"},
    {region=35, text="35: Ch36~48, Ch52~64"},
    {region=36, text="36: Ch36~48, Ch100~140, Ch149~165"},
    {region=37, text="37: Ch36~48, Ch52~64, Ch149~165, Ch173"}
}

mtkwifi.CountryRegionList_2G_All = {
    {region=0, text="0: Ch1~11"},
    {region=1, text="1: Ch1~13"},
    {region=2, text="2: Ch10~11"},
    {region=3, text="3: Ch10~13"},
    {region=4, text="4: Ch14"},
    {region=5, text="5: Ch1~14"},
    {region=6, text="6: Ch3~9"},
    {region=7, text="7: Ch5~13"},
    {region=31, text="31: Ch1~11, Ch12~14"},
    {region=32, text="32: Ch1~11, Ch12~13"},
    {region=33, text="33: Ch1~14"}
}

mtkwifi.ChannelList_6G_All = {
    {channel= 0  , text="Channel Auto", region={}},
    {channel= 1  , text="Channel  1   (5.955 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 5  , text="Channel  5   (5.975 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 9  , text="Channel  9   (5.995 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 13 , text="Channel  13  (6.015 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 17 , text="Channel  17  (6.035 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 21 , text="Channel  21  (6.055 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 25 , text="Channel  25  (6.075 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 29 , text="Channel  29  (6.095 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 33 , text="Channel  33  (6.115 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 37 , text="Channel  37  (6.135 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 41 , text="Channel  41  (6.155 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 45 , text="Channel  45  (6.175 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 49 , text="Channel  49  (6.195 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 53 , text="Channel  53  (6.215 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 57 , text="Channel  57  (6.235 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 61 , text="Channel  61  (6.255 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 65 , text="Channel  65  (6.275 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 69 , text="Channel  69  (6.295 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 73 , text="Channel  73  (6.315 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 77 , text="Channel  77  (6.335 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 81 , text="Channel  81  (6.355 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 85 , text="Channel  85  (6.375 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 89 , text="Channel  89  (6.395 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 93 , text="Channel  93  (6.415 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 97 , text="Channel  97  (6.435 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1}},
    {channel= 101, text="Channel  101 (6.455 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 105, text="Channel  105 (6.475 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 109, text="Channel  109 (6.495 GHz)", region={[0]=1, [2]=1, [7]=1}},
    {channel= 113, text="Channel  113 (6.515 GHz)", region={[0]=1, [2]=1}},
    {channel= 117, text="Channel  117 (6.535 GHz)", region={[0]=1, [2]=1}},
    {channel= 121, text="Channel  121 (6.555 GHz)", region={[0]=1, [3]=1}},
    {channel= 125, text="Channel  125 (6.575 GHz)", region={[0]=1, [3]=1}},
    {channel= 129, text="Channel  129 (6.595 GHz)", region={[0]=1, [3]=1}},
    {channel= 133, text="Channel  133 (6.615 GHz)", region={[0]=1, [3]=1}},
    {channel= 137, text="Channel  137 (6.635 GHz)", region={[0]=1, [3]=1}},
    {channel= 141, text="Channel  141 (6.655 GHz)", region={[0]=1, [3]=1}},
    {channel= 145, text="Channel  145 (6.675 GHz)", region={[0]=1, [3]=1}},
    {channel= 149, text="Channel  149 (6.695 GHz)", region={[0]=1, [3]=1}},
    {channel= 153, text="Channel  153 (6.715 GHz)", region={[0]=1, [3]=1}},
    {channel= 157, text="Channel  157 (6.735 GHz)", region={[0]=1, [3]=1}},
    {channel= 161, text="Channel  161 (6.755 GHz)", region={[0]=1, [3]=1}},
    {channel= 165, text="Channel  165 (6.775 GHz)", region={[0]=1, [3]=1}},
    {channel= 169, text="Channel  169 (6.795 GHz)", region={[0]=1, [3]=1}},
    {channel= 173, text="Channel  173 (6.815 GHz)", region={[0]=1, [3]=1}},
    {channel= 177, text="Channel  177 (6.835 GHz)", region={[0]=1, [3]=1}},
    {channel= 181, text="Channel  181 (6.855 GHz)", region={[0]=1, [3]=1}},
    {channel= 185, text="Channel  185 (6.875 GHz)", region={[0]=1, [3]=1}},
    {channel= 189, text="Channel  189 (6.895 GHz)", region={[0]=1, [4]=1}},
    {channel= 193, text="Channel  193 (6.915 GHz)", region={[0]=1, [4]=1}},
    {channel= 197, text="Channel  197 (6.935 GHz)", region={[0]=1, [4]=1}},
    {channel= 201, text="Channel  201 (6.955 GHz)", region={[0]=1, [4]=1}},
    {channel= 205, text="Channel  205 (6.975 GHz)", region={[0]=1, [4]=1}},
    {channel= 209, text="Channel  209 (6.995 GHz)", region={[0]=1, [4]=1}},
    {channel= 213, text="Channel  213 (7.015 GHz)", region={[0]=1, [4]=1}},
    {channel= 217, text="Channel  217 (7.035 GHz)", region={[0]=1, [4]=1}},
    {channel= 221, text="Channel  221 (7.055 GHz)", region={[0]=1, [4]=1}},
    {channel= 225, text="Channel  225 (7.075 GHz)", region={[0]=1, [4]=1}},
    {channel= 229, text="Channel  229 (7.095 GHz)", region={[0]=1, [4]=1}},
    {channel= 233, text="Channel  233 (7.115 GHz)", region={[0]=1, [4]=1}},
}

mtkwifi.ChannelList_5G_All = {
    {channel=0,  text="Channel Auto", region={}},
    {channel= 36, text="Channel  36 (5.180 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 40, text="Channel  40 (5.200 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 44, text="Channel  44 (5.220 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 48, text="Channel  48 (5.240 GHz)", region={[0]=1, [1]=1, [2]=1, [6]=1, [7]=1, [9]=1, [10]=1, [11]=1, [12]=1, [13]=1, [14]=1, [17]=1, [18]=1, [20]=1, [21]=1, [30]=1, [32]=1, [33]=1, [34]=1, [35]=1, [36]=1, [37]=1}},
    {channel= 52, text="Channel  52 (5.260 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel= 56, text="Channel  56 (5.280 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel= 60, text="Channel  60 (5.300 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel= 64, text="Channel  64 (5.320 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [7]=1, [8]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [16]=1, [18]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [33]=1, [34]=1, [35]=1, [37]=1}},
    {channel=100, text="Channel 100 (5.500 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=104, text="Channel 104 (5.520 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=108, text="Channel 108 (5.540 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=112, text="Channel 112 (5.560 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=116, text="Channel 116 (5.580 GHz)", region={[1]=1, [7]=1, [9]=1, [11]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=120, text="Channel 120 (5.600 GHz)", region={[1]=1, [7]=1, [11]=1, [12]=1, [13]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=124, text="Channel 124 (5.620 GHz)", region={[1]=1, [7]=1, [12]=1, [13]=1, [19]=1, [20]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=128, text="Channel 128 (5.640 GHz)", region={[1]=1, [7]=1, [12]=1, [13]=1, [19]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=132, text="Channel 132 (5.660 GHz)", region={[1]=1, [7]=1, [9]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=136, text="Channel 136 (5.680 GHz)", region={[1]=1, [7]=1, [9]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=140, text="Channel 140 (5.700 GHz)", region={[1]=1, [7]=1, [9]=1, [12]=1, [13]=1, [14]=1, [18]=1, [19]=1, [21]=1, [22]=1, [30]=1, [31]=1, [32]=1, [33]=1, [36]=1}},
    {channel=144, text="Channel 144 (5.720 GHz)", region={[12]=1, [13]=1, [14]=1}},
    {channel=149, text="Channel 149 (5.745 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=153, text="Channel 153 (5.765 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=157, text="Channel 157 (5.785 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=161, text="Channel 161 (5.805 GHz)", region={[0]=1, [3]=1, [4]=1, [5]=1, [7]=1, [9]=1, [10]=1, [11]=1, [13]=1, [14]=1, [15]=1, [16]=1, [17]=1, [19]=1, [20]=1, [21]=1, [30]=1, [31]=1, [32]=1, [34]=1, [36]=1, [37]=1}},
    {channel=165, text="Channel 165 (5.825 GHz)", region={[0]=1, [4]=1, [7]=1, [9]=1, [10]=1, [13]=1, [14]=1, [15]=1, [16]=1, [30]=1, [31]=1, [34]=1, [36]=1, [37]=1}},
    {channel=169, text="Channel 169 (5.845 GHz)", region={[15]=1}},
    {channel=173, text="Channel 173 (5.865 GHz)", region={[15]=1, [37]=1}}
}

mtkwifi.ChannelList_2G_All = {
    {channel=0, text="Channel Auto", region={}},
    {channel= 1, text="Channel  1 (2.412 GHz)", region={[0]=1, [1]=1, [5]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 2, text="Channel  2 (2.417 GHz)", region={[0]=1, [1]=1, [5]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 3, text="Channel  3 (2.422 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 4, text="Channel  4 (2.427 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 5, text="Channel  5 (2.432 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 6, text="Channel  6 (2.437 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 7, text="Channel  7 (2.442 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 8, text="Channel  8 (2.447 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel= 9, text="Channel  9 (2.452 GHz)", region={[0]=1, [1]=1, [5]=1, [6]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=10, text="Channel 10 (2.457 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=11, text="Channel 11 (2.462 GHz)", region={[0]=1, [1]=1, [2]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=12, text="Channel 12 (2.467 GHz)", region={[1]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=13, text="Channel 13 (2.472 GHz)", region={[1]=1, [3]=1, [5]=1, [7]=1, [31]=1, [32]=1, [33]=1}},
    {channel=14, text="Channel 14 (2.477 GHz)", region={[4]=1, [5]=1, [31]=1, [33]=1}}
}

mtkwifi.ChannelList_5G_2nd_80MHZ_ALL = {
    {channel=36, text="Ch36(5.180 GHz) - Ch48(5.240 GHz)", chidx=2},
    {channel=52, text="Ch52(5.260 GHz) - Ch64(5.320 GHz)", chidx=6},
    {channel=-1, text="Channel between 64 100",  chidx=-1},
    {channel=100, text="Ch100(5.500 GHz) - Ch112(5.560 GHz)", chidx=10},
    {channel=112, text="Ch116(5.580 GHz) - Ch128(5.640 GHz)", chidx=14},
    {channel=-1, text="Channel between 128 132", chidx=-1},
    {channel=132, text="Ch132(5.660 GHz) - Ch144(5.720 GHz)", chidx=18},
    {channel=-1, text="Channel between 144 149", chidx=-1},
    {channel=149, text="Ch149(5.745 GHz) - Ch161(5.805 GHz)", chidx=22}
}

local AuthModeList = {
    "Disable",
    "OPEN",--OPENWEP
    "Enhanced Open",
    "SHARED",--SHAREDWEP
    "WEPAUTO",
    "WPA2",
    "WPA3",
    "WPA3-192-bit",
    "WPA2PSK",
    "WPA3PSK",
    "WPAPSKWPA2PSK",
    "WPA2PSKWPA3PSK",
    "WPA1WPA2",
    "IEEE8021X"
}

local AuthModeList_6G = {
    "Enhanced Open",
    "WPA3PSK"
}

local WpsEnableAuthModeList = {
    "Disable",
    "OPEN",--OPENWEP
    "WPA2PSK",
    "WPAPSKWPA2PSK"
}

local WpsEnableAuthModeList_6G = {
    "Disable",
    "WPA2PSK",
    "WPAPSKWPA2PSK"
}

local ApCliAuthModeList = {
    "Disable",
    "OPEN",
    "SHARED",
    "Enhanced Open",
    "WPAPSK",
    "WPA2PSK",
    "WPA3PSK",
    -- "WPAPSKWPA2PSK",
    -- "WPA2PSKWPA3PSK",
    -- "WPA",
    -- "WPA2",
    -- "WPAWPA2",
    -- "8021X",
}

local EncryptionTypeList = {
    "AES",
    "GCMP256",
    "TKIPAES",
    "TKIP",
    "WEP"
}

local EncryptionTypeList_6G = {
    "AES",
    "GCMP256",
    "WEP"
}

local dbdc_prefix = {
    {"ra",  "rax"},
    {"rai", "ray"},
    {"rae", "raz"}
}

local dbdc_apcli_prefix = {
    {"apcli",  "apclix"},
    {"apclii", "apcliy"},
    {"apclie", "apcliz"}
}

function mtkwifi.band(mode)
    local i = tonumber(mode)
    if i == 0
    or i == 1
    or i == 4
    or i == 6
    or i == 7
    or i == 9
    or i == 16 then
        return "2.4G"
    elseif i == 18 then
        return "6G"
    else
        return "5G"
    end
end


function mtkwifi.__cfg2list(str)
    -- delimeter == ";"
    local i = 1
    local list = {}
    for k in string.gmatch(str, "([^;]+)") do
        list[i] = k
        i = i + 1
    end
    return list
end

function mtkwifi.token_set(str, n, v)
    -- n start from 1
    -- delimeter == ";"
    if not str then return end
    local tmp = mtkwifi.__cfg2list(str)
    if type(v) ~= type("") and type(v) ~= type(0) then
        nixio.syslog("err", "invalid value type in token_set, "..type(v))
        return
    end
    if #tmp < tonumber(n) then
        for i=#tmp, tonumber(n) do
            if not tmp[i] then
                tmp[i] = v -- pad holes with v !
            end
        end
    else
        tmp[n] = v
    end
    return table.concat(tmp, ";"):gsub("^;*(.-);*$", "%1"):gsub(";+",";")
end


function mtkwifi.token_get(str, n, v)
    -- n starts from 1
    -- v is the backup in case token n is nil
    if not str then return v end
    local tmp = mtkwifi.__cfg2list(str)
    return tmp[tonumber(n)] or v
end

function mtkwifi.search_dev_and_profile_orig()
    local nixio = require("nixio")
    local dir = io.popen("ls /etc/wireless/")
    if not dir then return end
    local result = {}
    -- case 1: mt76xx.dat (best)
    -- case 2: mt76xx.n.dat (multiple card of same dev)
    -- case 3: mt76xx.n.nG.dat (case 2 plus dbdc and multi-profile, bloody hell....)
    for line in dir:lines() do
        -- nixio.syslog("debug", "scan "..line)
        local tmp = io.popen("find /etc/wireless/"..line.." -type f -name \"*.dat\"")
        for datfile in tmp:lines() do
            -- nixio.syslog("debug", "test "..datfile)

            repeat do
            -- for case 1
            local devname = string.match(datfile, "("..line..").dat")
            if devname then
                result[devname] = datfile
                -- nixio.syslog("debug", "yes "..devname.."="..datfile)
                break
            end
            -- for case 2
            local devname = string.match(datfile, "("..line.."%.%d)%.dat")
            if devname then
                result[devname] = datfile
                -- nixio.syslog("debug", "yes "..devname.."="..datfile)
                break
            end
            -- for case 3
            local devname = string.match(datfile, "("..line.."%.%d%.%dG)%.dat")
            if devname then
                result[devname] = datfile
                -- nixio.syslog("debug", "yes "..devname.."="..datfile)
                break
            end
            end until true
        end
    end

    for k,v in pairs(result) do
        nixio.syslog("debug", "search_dev_and_profile_orig: "..k.."="..v)
    end

    return result
end

function mtkwifi.search_dev_and_profile_l1()
    local l1dat = mtkwifi.__get_l1dat()

    if not l1dat then return end

    local nixio = require("nixio")
    local result = {}
    local dbdc_2nd_if = ""

    for k, dev in ipairs(l1dat) do
        dbdc_2nd_if = mtkwifi.token_get(dev.main_ifname, 2, nil)
        if dbdc_2nd_if then
            result[dev["INDEX"].."."..dev["mainidx"]..".1"] = mtkwifi.token_get(dev.profile_path, 1, nil)
            result[dev["INDEX"].."."..dev["mainidx"]..".2"] = mtkwifi.token_get(dev.profile_path, 2, nil)
        else
            result[dev["INDEX"].."."..dev["mainidx"]] = dev.profile_path
        end
    end

--    for k,v in pairs(result) do
--        nixio.syslog("debug", "search_dev_and_profile_l1: "..k.."="..v)
--    end

    return result
end

function mtkwifi.search_dev_and_profile()
    return mtkwifi.search_dev_and_profile_l1() or mtkwifi.search_dev_and_profile_orig()
end

function mtkwifi.__setup_vifs(cfgs, devname, mainidx, subidx)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX

    local prefix
    local main_ifname
    local vifs = {}
    local dev_idx = ""


    prefix = l1dat and l1dat[dridx][devname].ext_ifname or dbdc_prefix[mainidx][subidx]

    dev_idx = string.match(devname, "(%w+)")

    vifs["__prefix"] = prefix
    if (cfgs.BssidNum == nil) then
        debug_write("BssidNum configuration value not found.")
        nixio.syslog("debug","BssidNum configuration value not found.")
        return
    end

    for j=1,tonumber(cfgs.BssidNum) do
        vifs[j] = {}
        vifs[j].vifidx = j -- start from 1
        dev_idx = string.match(devname, "(%w+)")
        main_ifname = l1dat and l1dat[dridx][devname].main_ifname or dbdc_prefix[mainidx][subidx].."0"
        vifs[j].vifname = j == 1 and main_ifname or prefix..(j-1)
        if mtkwifi.exists("/sys/class/net/"..vifs[j].vifname) then
            local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..vifs[j].vifname.."/flags 2>/dev/null")) or 0
            vifs[j].state = flags%2 == 1 and "up" or "down"
        end
        vifs[j].__ssid = cfgs["SSID"..j]
        local rd_pipe_output = mtkwifi.read_pipe("cat /sys/class/net/"..prefix..(j-1).."/address 2>/dev/null")
        vifs[j].__bssid = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or "?"

        vifs[j].__temp_ssid = mtkwifi.__trim(mtkwifi.read_pipe("iwconfig "..vifs[j].vifname.." | grep ESSID | cut -d : -f 2"))
        if vifs[j].state == "up" then
            vifs[j].__wirelessmode_table = c_getWMode(vifs[j].vifname)
        else
            vifs[j].__wirelessmode_table = { ["getwmode"] = "" }
        end
        vifs[j].__temp_wirelessmode = vifs[j].__wirelessmode_table['getwmode']

        if (vifs[j].__temp_ssid ~= "") then
            vifs[j].__ssid = vifs[j].__temp_ssid:gsub("^\"(.-)\"$","%1")
        else
            vifs[j].__ssid = cfgs["SSID"..j]
        end

        vifs[j].__channel = tonumber(c_getChannel(vifs[j].vifname)['channel'])
        if (vifs[j].__channel < 0) then
            vifs[j].__channel = cfgs.Channel
        end

        if (vifs[j].__temp_wirelessmode ~= "" and vifs[j].__temp_wirelessmode ~= "0") then
            vifs[j].__wirelessmode = vifs[j].__temp_wirelessmode
        else
            vifs[j].__wirelessmode = mtkwifi.token_get(cfgs.WirelessMode, j, 0)
        end

        vifs[j].__authmode = mtkwifi.token_get(cfgs.AuthMode, j, mtkwifi.__split(cfgs.AuthMode,";")[1])
        vifs[j].__encrypttype = mtkwifi.token_get(cfgs.EncrypType, j, mtkwifi.__split(cfgs.EncrypType,";")[1])
        vifs[j].__hidessid = mtkwifi.token_get(cfgs.HideSSID, j, mtkwifi.__split(cfgs.HideSSID,";")[1])
        vifs[j].__noforwarding = mtkwifi.token_get(cfgs.NoForwarding, j, mtkwifi.__split(cfgs.NoForwarding,";")[1])
        vifs[j].__wmmcapable = mtkwifi.token_get(cfgs.WmmCapable, j, mtkwifi.__split(cfgs.WmmCapable,";")[1])
        vifs[j].__txrate = mtkwifi.token_get(cfgs.TxRate, j, mtkwifi.__split(cfgs.TxRate,";")[1])
        vifs[j].__ieee8021x = mtkwifi.token_get(cfgs.IEEE8021X, j, mtkwifi.__split(cfgs.IEEE8021X,";")[1])
        vifs[j].__preauth = mtkwifi.token_get(cfgs.PreAuth, j, mtkwifi.__split(cfgs.PreAuth,";")[1])
        vifs[j].__rekeymethod = mtkwifi.token_get(cfgs.RekeyMethod, j, mtkwifi.__split(cfgs.RekeyMethod,";")[1])
        vifs[j].__rekeyinterval = mtkwifi.token_get(cfgs.RekeyInterval, j, mtkwifi.__split(cfgs.RekeyInterval,";")[1])
        vifs[j].__pmkcacheperiod = mtkwifi.token_get(cfgs.PMKCachePeriod, j, mtkwifi.__split(cfgs.PMKCachePeriod,";")[1])
        vifs[j].__ht_extcha = mtkwifi.token_get(cfgs.HT_EXTCHA, j, mtkwifi.__split(cfgs.HT_EXTCHA,";")[1])
        vifs[j].__radius_server = mtkwifi.token_get(cfgs.RADIUS_Server, j, mtkwifi.__split(cfgs.RADIUS_Server,";")[1])
        vifs[j].__radius_port = mtkwifi.token_get(cfgs.RADIUS_Port, j, mtkwifi.__split(cfgs.RADIUS_Port,";")[1])
        vifs[j].__wepkey_id = mtkwifi.token_get(cfgs.DefaultKeyID, j, mtkwifi.__split(cfgs.DefaultKeyID,";")[1])
        vifs[j].__wscconfmode = mtkwifi.token_get(cfgs.WscConfMode, j, mtkwifi.__split(cfgs.WscConfMode,";")[1])
        vifs[j].__wepkeys = {
            cfgs["Key1Str"..j],
            cfgs["Key2Str"..j],
            cfgs["Key3Str"..j],
            cfgs["Key4Str"..j],
        }
        vifs[j].__wpapsk = cfgs["WPAPSK"..j]
        vifs[j].__ht_stbc = mtkwifi.token_get(cfgs.HT_STBC, j, mtkwifi.__split(cfgs.HT_STBC,";")[1])
        vifs[j].__ht_ldpc = mtkwifi.token_get(cfgs.HT_LDPC, j, mtkwifi.__split(cfgs.HT_LDPC,";")[1])
        vifs[j].__vht_stbc = mtkwifi.token_get(cfgs.VHT_STBC, j, mtkwifi.__split(cfgs.VHT_STBC,";")[1])
        vifs[j].__vht_ldpc = mtkwifi.token_get(cfgs.VHT_LDPC, j, mtkwifi.__split(cfgs.VHT_LDPC,";")[1])
        vifs[j].__dls_capable = mtkwifi.token_get(cfgs.DLSCapable, j, mtkwifi.__split(cfgs.DLSCapable,";")[1])
        vifs[j].__rrmenable = mtkwifi.token_get(cfgs.RRMEnable, j, mtkwifi.__split(cfgs.RRMEnable,";")[1])
        vifs[j].__apsd_capable = mtkwifi.token_get(cfgs.APSDCapable, j, mtkwifi.__split(cfgs.APSDCapable,";")[1])
        vifs[j].__frag_threshold = mtkwifi.token_get(cfgs.FragThreshold, j, mtkwifi.__split(cfgs.FragThreshold,";")[1])
        vifs[j].__kickrssi = mtkwifi.token_get(cfgs.KickStaRssiLow, j, mtkwifi.__split(cfgs.KickStaRssiLow,";")[1])
        vifs[j].__assocthres = mtkwifi.token_get(cfgs.AssocReqRssiThres, j, mtkwifi.__split(cfgs.AssocReqRssiThres,";")[1])
        vifs[j].__rts_threshold = mtkwifi.token_get(cfgs.RTSThreshold, j, mtkwifi.__split(cfgs.RTSThreshold,";")[1])
        vifs[j].__vht_sgi = mtkwifi.token_get(cfgs.VHT_SGI, j, mtkwifi.__split(cfgs.VHT_SGI,";")[1])
        vifs[j].__vht_bw_signal = mtkwifi.token_get(cfgs.VHT_BW_SIGNAL, j, mtkwifi.__split(cfgs.VHT_BW_SIGNAL,";")[1])
        vifs[j].__ht_protect = mtkwifi.token_get(cfgs.HT_PROTECT, j, mtkwifi.__split(cfgs.HT_PROTECT,";")[1])
        vifs[j].__ht_gi = mtkwifi.token_get(cfgs.HT_GI, j, mtkwifi.__split(cfgs.HT_GI,";")[1])
        vifs[j].__ht_opmode = mtkwifi.token_get(cfgs.HT_OpMode, j, mtkwifi.__split(cfgs.HT_OpMode,";")[1])
        vifs[j].__ht_amsdu = mtkwifi.token_get(cfgs.HT_AMSDU, j, mtkwifi.__split(cfgs.HT_AMSDU,";")[1])
        vifs[j].__ht_autoba = mtkwifi.token_get(cfgs.HT_AutoBA, j, mtkwifi.__split(cfgs.HT_AutoBA,";")[1])
        vifs[j].__igmp_snenable = mtkwifi.token_get(cfgs.IgmpSnEnable, j, mtkwifi.__split(cfgs.IgmpSnEnable,";")[1])
        vifs[j].__wdsenable = mtkwifi.token_get(cfgs.WdsEnable, j, mtkwifi.__split(cfgs.WdsEnable,";")[1])

        -- VoW
        vifs[j].__atc_tp     = mtkwifi.token_get(cfgs.VOW_Rate_Ctrl_En,    j, mtkwifi.__split(cfgs.VOW_Rate_Ctrl_En,";")[1])
        vifs[j].__atc_min_tp = mtkwifi.token_get(cfgs.VOW_Group_Min_Rate,  j, mtkwifi.__split(cfgs.VOW_Group_Min_Rate,";")[1])
        vifs[j].__atc_max_tp = mtkwifi.token_get(cfgs.VOW_Group_Max_Rate,  j, mtkwifi.__split(cfgs.VOW_Group_Max_Rate,";")[1])
        vifs[j].__atc_at     = mtkwifi.token_get(cfgs.VOW_Airtime_Ctrl_En, j, mtkwifi.__split(cfgs.VOW_Airtime_Ctrl_En,";")[1])
        vifs[j].__atc_min_at = mtkwifi.token_get(cfgs.VOW_Group_Min_Ratio, j, mtkwifi.__split(cfgs.VOW_Group_Min_Ratio,";")[1])
        vifs[j].__atc_max_at = mtkwifi.token_get(cfgs.VOW_Group_Max_Ratio, j, mtkwifi.__split(cfgs.VOW_Group_Max_Ratio,";")[1])

        -- TODO index by vifname
        vifs[vifs[j].vifname] = vifs[j]

        -- OFDMA and MU-MIMO
        vifs[j].__muofdma_dlenable = mtkwifi.token_get(cfgs.MuOfdmaDlEnable, j, mtkwifi.__split(cfgs.MuOfdmaDlEnable,";")[1])
        vifs[j].__muofdma_ulenable = mtkwifi.token_get(cfgs.MuOfdmaUlEnable, j, mtkwifi.__split(cfgs.MuOfdmaUlEnable,";")[1])
        vifs[j].__mumimo_dlenable = mtkwifi.token_get(cfgs.MuMimoDlEnable, j, mtkwifi.__split(cfgs.MuMimoDlEnable,";")[1])
        vifs[j].__mumimo_ulenable = mtkwifi.token_get(cfgs.MuMimoUlEnable, j, mtkwifi.__split(cfgs.MuMimoUlEnable,";")[1])

    end

    return vifs
end

function mtkwifi.__setup_apcli(cfgs, devname, mainidx, subidx)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX

    local apcli = {}
    local dev_idx = string.match(devname, "(%w+)")
    local apcli_prefix = l1dat and l1dat[dridx][devname].apcli_ifname or
                         dbdc_apcli_prefix[mainidx][subidx]

    local apcli_name = apcli_prefix.."0"

    if mtkwifi.exists("/sys/class/net/"..apcli_name) then
        apcli.vifname = apcli_name
        apcli.devname = apcli_name
        apcli.vifidx = "1"
        local rd_pipe_output = mtkwifi.read_pipe("iwconfig "..apcli_name.." | grep ESSID 2>/dev/null")
        local ssid = rd_pipe_output and string.match(rd_pipe_output, "ESSID:\"(.*)\"")
        if not ssid or ssid == "" then
            apcli.status = "Disconnected"
        else
            apcli.ssid = ssid
            apcli.status = "Connected"
        end
        local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..apcli_name.."/flags 2>/dev/null")) or 0
        apcli.state = flags%2 == 1 and "up" or "down"
        rd_pipe_output = mtkwifi.__trim(mtkwifi.read_pipe("cat /sys/class/net/"..apcli_name.."/address 2>/dev/null"))
        apcli.mac_addr = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or "?"
        rd_pipe_output = mtkwifi.__trim(mtkwifi.read_pipe("iwconfig "..apcli_name.." | grep 'Access Point' 2>/dev/null"))
        apcli.bssid = rd_pipe_output and string.match(rd_pipe_output, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x") or "Not-Associated"
        return apcli
    else
        return
    end
end

function mtkwifi.__setup_eths()
    local etherInfo = {}
    local all_eth_devs = mtkwifi.read_pipe("ls /sys/class/net/ | grep eth | grep -v grep")
    if not all_eth_devs or all_eth_devs == "" then
        return
    end
    for ethName in string.gmatch(all_eth_devs, "(eth%d)") do
        local ethInfo = {}
        ethInfo['ifname'] = ethName
        local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..ethName.."/flags 2>/dev/null")) or 0
        ethInfo['state'] = flags%2 == 1 and "up" or "down"
        ethInfo['mac_addr'] = mtkwifi.__trim(mtkwifi.read_pipe("cat /sys/class/net/"..ethName.."/address 2>/dev/null")) or "?"
        table.insert(etherInfo,ethInfo)
    end
    return etherInfo
end

function mtkwifi.get_all_devs()
    local nixio = require("nixio")
    local devs = {}
    local i = 1 -- dev idx
    local profiles = mtkwifi.search_dev_and_profile()
    local wpa_support = 0
    local wapi_support = 0
    local wifi_driver_version = ""

    for devname,profile in mtkwifi.__spairs(profiles, function(a,b) return string.upper(a) < string.upper(b) end) do
        local fd = io.open(profile,"r")
        if not fd then
            nixio.syslog("debug", "cannot find "..profile)
        else
            fd:close()
            local cfgs = mtkwifi.load_profile(profile)
            if not cfgs then
                debug_write("error loading profile"..profile)
                nixio.syslog("err", "error loading "..profile)
                return
            end
            devs[i] = {}
            devs[i].vifs = {}
            devs[i].apcli = {}
            devs[i].devname = devname
            devs[i].profile = profile
            local tmp = ""
            tmp = string.split(devname, ".")
            devs[i].maindev = tmp[1]
            devs[i].mainidx = tonumber(tmp[2]) or 1
            devs[i].subdev = devname
            devs[i].subidx = string.match(tmp[3] or "", "(%d+)")=="2" and 2 or 1
            devs[i].devband = tonumber(tmp[3])
            if devs[i].devband then
                devs[i].multiprofile = true
                devs[i].dbdc = true
                devs[i].dbdcBandName = (profile:match("2[gG]") and "2.4G") or (profile:match("5[gG]") and "5G")
                if not devs[i].dbdcBandName then
                    -- Make 1st band as 2.4G and 2nd band as 5G.
                    devs[i].dbdcBandName = (devs[i].devband == 1) and "2.4G" or "5G"
                end
            end

            if cfgs.DevEnable ~= nil then
                devs[i].Enable = cfgs.DevEnable
            else
                devs[i].Enable = "1"
            end

            devs[i].ApCliEnable = cfgs.ApCliEnable
            devs[i].WirelessMode = string.split(cfgs.WirelessMode,";")[1]
            devs[i].WirelessModeList = {}
            for key, value in pairs(DevicePropertyMap) do
                local found = string.find(string.upper(devname), string.upper(value.device))
                if found then
                    for k=1,#value.band do
                        devs[i].WirelessModeList[tonumber(value.band[k])] = WirelessModeList[tonumber(value.band[k])]
                    end

                    if devs[i].dbdc == true then
                        devs[i].maxVif = value.maxDBDCVif or value.maxVif/2
                    else
                        devs[i].maxVif = value.maxVif or 16
                    end

                    devs[i].maxTxStream = value.maxTxStream
                    devs[i].maxRxStream = value.maxRxStream
                    devs[i].invalidChBwList = value.invalidChBwList
                    devs[i].isPowerBoostSupported = value.isPowerBoostSupported
                    devs[i].wdsBand = value.wdsBand
                    devs[i].mimoBand = value.mimoBand
                    devs[i].isMultiAPSupported = value.isMultiAPSupported
                    devs[i].isWPA3_192bitSupported = value.isWPA3_192bitSupported
                end
            end
            devs[i].WscConfMode = cfgs.WscConfMode
            devs[i].AuthModeList = AuthModeList
            devs[i].AuthModeList_6G = AuthModeList_6G
            devs[i].WpsEnableAuthModeList = WpsEnableAuthModeList
            devs[i].WpsEnableAuthModeList_6G = WpsEnableAuthModeList_6G

            if wpa_support == 1 then
                table.insert(devs[i].AuthModeList,"WPAPSK")
                table.insert(devs[i].AuthModeList,"WPA")
            end

            if wapi_support == 1 then
                table.insert(devs[i].AuthModeList,"WAIPSK")
                table.insert(devs[i].AuthModeList,"WAICERT")
            end
            devs[i].ApCliAuthModeList = ApCliAuthModeList
            devs[i].EncryptionTypeList = EncryptionTypeList
            devs[i].EncryptionTypeList_6G = EncryptionTypeList_6G
            devs[i].Channel = tonumber(cfgs.Channel)
            devs[i].DBDC_MODE = tonumber(cfgs.DBDC_MODE)
            devs[i].band = devs[i].devband or mtkwifi.band(string.split(cfgs.WirelessMode,";")[1])

            if cfgs.MUTxRxEnable then
                if tonumber(cfgs.ETxBfEnCond)==1
                    and tonumber(cfgs.MUTxRxEnable)==0
                    and tonumber(cfgs.ITxBfEn)==0
                    then devs[i].__mimo = 0
                elseif tonumber(cfgs.ETxBfEnCond)==0
                    and tonumber(cfgs.MUTxRxEnable)==0
                    and tonumber(cfgs.ITxBfEn)==1
                    then devs[i].__mimo = 1
                elseif tonumber(cfgs.ETxBfEnCond)==1
                    and tonumber(cfgs.MUTxRxEnable)==0
                    and tonumber(cfgs.ITxBfEn)==1
                    then devs[i].__mimo = 2
                elseif tonumber(cfgs.ETxBfEnCond)==1
                    and tonumber(cfgs.MUTxRxEnable)>0
                    and tonumber(cfgs.ITxBfEn)==0
                    then devs[i].__mimo = 3
                elseif tonumber(cfgs.ETxBfEnCond)==1
                    and tonumber(cfgs.MUTxRxEnable)>0
                    and tonumber(cfgs.ITxBfEn)==1
                    then devs[i].__mimo = 4
                else devs[i].__mimo = 5
                end
            end

            if cfgs.HT_BW == "0" or not cfgs.HT_BW then
                devs[i].__bw = "20"
            elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "0" or not cfgs.VHT_BW then
                if cfgs.HT_BSSCoexistence == "0" or not cfgs.HT_BSSCoexistence then
                    devs[i].__bw = "40"
                else
                    devs[i].__bw = "60" -- 20/40 coexist
                end
            elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "1" then
                devs[i].__bw = "80"
            elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "2" then
                devs[i].__bw = "160"
            elseif cfgs.HT_BW == "1" and cfgs.VHT_BW == "3" then
                devs[i].__bw = "161"
            end

            devs[i].vifs = mtkwifi.__setup_vifs(cfgs, devname, devs[i].mainidx, devs[i].subidx)
            devs[i].apcli = mtkwifi.__setup_apcli(cfgs, devname, devs[i].mainidx, devs[i].subidx)

            local vif_name = nil
            if devs[i].apcli and devs[i].apcli["state"] == "up" then
                vif_name = devs[i].apcli["vifname"]
            elseif devs[i].vifs then
                for _,vif in ipairs(devs[i].vifs) do
                    if vif["state"] == "up" then
                        vif_name = vif["vifname"]
                        break
                    end
                end
            end
            if not vif_name then
                devs[i].version = nil
            else
                local version = mtkwifi.read_pipe("iwpriv "..vif_name.." get_driverinfo")
                version = version and version:match("Driver version: (.-)\n") or ""
                devs[i].version = version ~= "" and version or nil
            end

            if devs[i].version ~= nil then
                wifi_driver_version = devs[i].version
            end

            -- Setup reverse indices by devname
            devs[devname] = devs[i]

            if devs[i].apcli then
                devs[i][devs[i].apcli.devname] = devs[i].apcli
            end

            i = i + 1
        end
    end
    for _,dev in ipairs(devs) do
        if dev.version == nil and dev.dbdc == true then
            dev.version = wifi_driver_version
        end
    end
    devs['etherInfo'] = mtkwifi.__setup_eths()
    return devs
end

function mtkwifi.exists(path)
    local fp = io.open(path, "rb")
    if fp then fp:close() end
    return fp ~= nil
end

function mtkwifi.parse_mac(str)
    local macs = {}
    local pat = "^[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]:[0-9a-fA-F][0-9a-fA-F]$"

    local function ismac(str)
        if str:match(pat) then return str end
    end

    if not str then return macs end
    local t = str:split("\n")
    for _,v in pairs(t) do
            local mac = ismac(mtkwifi.__trim(v))
            if mac then
                table.insert(macs, mac)
            end
    end

    return macs
    -- body
end


function mtkwifi.scan_ap(vifname)
    local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..vifname.."/flags 2>/dev/null")) or 0
    if flags%2 ~= 1 then
        os.execute("ifconfig "..vifname.." up")
    end
    
    os.execute("iwpriv "..vifname.." set SiteSurvey=0")
    os.execute("sleep 5") -- depends on your env
    local op =  c_scanResult(vifname, 0)
    local scan_result = op["scanresult"]
    local next_line_index = 0
    local cur_index
    local total_index = 0
    local ap_list = {}
    local xx = {}
    local tmp

    while (1) do
        for i, line in ipairs(mtkwifi.__lines(scan_result)) do
            local is_mac_addr_present = string.match(line, "%s+%x%x:%x%x:%x%x:%x%x:%x%x:%x%x%s+")
            -- If the line does not contain any MAC address and length is greater than 40 bytes,
            -- then, the line is the header of the get_site_survey page.
            local total_str = string.find(line, "Total=")
            if total_str == 1 then
                total_index = tonumber(line:match("%d+"))
            end

            if #line>40 and not is_mac_addr_present then
                xx.Ch = {string.find(line, "Ch "),3}
                xx.SSID = {string.find(line, "SSID "),32}
                local fidx = string.find(line, "SSID_Len")
                if fidx then
                    xx.SSID_len = {fidx,2}
                end
                xx.BSSID = {string.find(line, "BSSID "),17}
                xx.Security = {string.find(line, "Security "),22}
                xx.Signal = {string.find(line, "Sig%a%al"),4}
                xx.Mode = {string.find(line, "W-Mode"),5}
                xx.ExtCh = {string.find(line, "ExtCH"),6}
                xx.WPS = {string.find(line, "WPS"),3}
                xx.NT = {string.find(line, "NT"),2}
                fidx = string.find(line, "OWETranIe")
                if fidx then
                    xx.OWETranIe = {fidx,9}
                end
            end

            if #line>40 and is_mac_addr_present then
                tmp = {}
                tmp.channel = mtkwifi.__trim(string.sub(line, xx.Ch[1], xx.Ch[1]+xx.Ch[2]))
                if xx.SSID_len then
                    -- Maximum xx.SSID[2] characters are supported in SSID
                    tmp.ssid_len = tonumber(mtkwifi.__trim(string.sub(line, xx.SSID_len[1], xx.SSID_len[1]+xx.SSID_len[2]))) or xx.SSID[2]
                    if tmp.ssid_len > xx.SSID[2] or tmp.ssid_len < 0 then
                        tmp.ssid_len = xx.SSID[2]
                        tmp.ssid = string.sub(line, xx.SSID[1], xx.SSID[1]+tmp.ssid_len-1)
                    else
                        tmp.ssid = string.sub(line, xx.SSID[1], xx.BSSID[1]-1)
                        if string.find(tmp.ssid, "0x") == nil then
                            tmp.ssid = string.sub(line, xx.SSID[1], xx.SSID[1]+tmp.ssid_len-1)
                        end
                    end
                else
                    tmp.ssid = mtkwifi.__trim(string.sub(line, xx.SSID[1], xx.SSID[1]+xx.SSID[2]))
                    tmp.ssid_len = tmp.ssid:len()
                end
                tmp.bssid = string.upper(mtkwifi.__trim(string.sub(line, xx.BSSID[1], xx.BSSID[1]+xx.BSSID[2])))
                tmp.security = mtkwifi.__trim(string.sub(line, xx.Security[1], xx.Security[1]+xx.Security[2]))
                tmp.authmode = mtkwifi.__trim(string.split(tmp.security, "/")[1])
                tmp.encrypttype = mtkwifi.__trim(string.split(tmp.security, "/")[2] or "NONE")
                tmp.rssi = mtkwifi.__trim(string.sub(line, xx.Signal[1], xx.Signal[1]+xx.Signal[2]))
                tmp.extch = mtkwifi.__trim(string.sub(line, xx.ExtCh[1], xx.ExtCh[1]+xx.ExtCh[2]))
                tmp.mode = mtkwifi.__trim(string.sub(line, xx.Mode[1], xx.Mode[1]+xx.Mode[2]))
                tmp.wps = mtkwifi.__trim(string.sub(line, xx.WPS[1], xx.WPS[1]+xx.WPS[2]))
                tmp.nt = mtkwifi.__trim(string.sub(line, xx.NT[1], xx.NT[1]+xx.NT[2]))
                if xx.OWETranIe then
                    tmp.OWETranIe = mtkwifi.__trim(string.sub(line, xx.OWETranIe[1], xx.OWETranIe[1]+xx.OWETranIe[2]))
                end
                table.insert(ap_list, tmp)
                cur_index = tonumber(line:match("^%d+"))
                if cur_index == total_index - 1 then
                    break;
                end
                next_line_index = cur_index and cur_index + 1 or next_line_index
            end
        end
        if cur_index and cur_index == next_line_index - 1 then
            --scan_result = mtkwifi.read_pipe("iwpriv "..vifname.." get_site_survey "..next_line_index)
            if next_line_index == total_index - 1 then
                scan_result = nil
            else
                op =  c_scanResult(vifname, next_line_index)
                scan_result = op["scanresult"]
            end
        else
            scan_result = nil
        end

        if not scan_result or not string.match(scan_result, "%s+%x%x:%x%x:%x%x:%x%x:%x%x:%x%x%s+") then
            break
        end
    end

    if flags%2 ~= 1 then
        os.execute("ifconfig "..vifname.." down")
    end

    return ap_list
end

function mtkwifi.__any_wsc_enabled(wsc_conf_mode)
    if (wsc_conf_mode == "") then
        return 0;
    end
    if (wsc_conf_mode == "7") then
        return 1;
    end
    if (wsc_conf_mode == "4") then
        return 1;
    end
    if (wsc_conf_mode == "2") then
        return 1;
    end
    if (wsc_conf_mode == "1") then
        return 1;
    end
    return 0;
end

function mtkwifi.__restart_if_wps(devname, ifname, cfgs)
    local devs = mtkwifi.get_all_devs()
    local ssid_index = devs[devname]["vifs"][ifname].vifidx
    local wsc_conf_mode = ""

    wsc_conf_mode=mtkwifi.token_get(cfgs["WscConfMode"], ssid_index, "")

    os.execute("iwpriv "..ifname.." set WscConfMode=0")
    debug_write("iwpriv "..ifname.." set WscConfMode=0")
    os.execute("route delete 239.255.255.250")
    debug_write("route delete 239.255.255.250")
    if(mtkwifi.__any_wsc_enabled(wsc_conf_mode)) then
        os.execute("iwpriv "..ifname.." set WscConfMode=7")
        debug_write("iwpriv "..ifname.." set WscConfMode=7")
        os.execute("route add -host 239.255.255.250 dev br0")
        debug_write("route add -host 239.255.255.250 dev br0")
    end

    return cfgs
end

function mtkwifi.restart_8021x(devname, devices)
    local l1dat, l1 = mtkwifi.__get_l1dat()
    local dridx = l1dat and l1.DEV_RINDEX

    local devs = devices or mtkwifi.get_all_devs()
    local dev = devs[devname]
    local main_ifname = l1dat and l1dat[dridx][devname].main_ifname or dbdc_prefix[mainidx][subidx].."0"
    local prefix = l1dat and l1dat[dridx][devname].ext_ifname or dbdc_prefix[mainidx][subidx]

    local ps_cmd = "ps | grep -v grep | grep rt2860apd | grep "..main_ifname.." | awk '{print $1}'"
    local pid_cmd = "cat /var/run/rt2860apd_"..devs[devname].vifs[1].vifname..".pid"
    local apd_pid = mtkwifi.read_pipe(pid_cmd) or mtkwifi.read_pipe(ps_cmd)
    if tonumber(apd_pid) then
        os.execute("kill "..apd_pid)
    end

    local cfgs = mtkwifi.load_profile(devs[devname].profile)
    local auth_mode = cfgs['AuthMode']
    local ieee8021x = cfgs['IEEE8021X']
    local pat_auth_mode = {"WPA$", "WPA;", "WPA2$", "WPA2;", "WPA1WPA2$", "WPA1WPA2;"}
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

    if not apd_en then
        return
    end
    if prefix == "ra" then
        mtkwifi.__fork_exec("rt2860apd -i "..main_ifname.." -p "..prefix)
    elseif prefix == "rae" then
        mtkwifi.__fork_exec("rtwifi3apd -i "..main_ifname.." -p "..prefix)
    elseif prefix == "rai" then
        mtkwifi.__fork_exec("rtinicapd -i "..main_ifname.." -p "..prefix)
    elseif prefix == "rax" or prefix == "ray" or prefix == "raz" then
        mtkwifi.__fork_exec("rt2860apd_x -i "..main_ifname.." -p "..prefix)
    end
end

function mtkwifi.dat2uci(datfile, ucifile)
    local shuci = require("shuci")
    local cfgs = mtkwifi.load_profile(datfile)

    local uci = {}

    uci["wifi-device"]={}
    uci["wifi-device"][".name"] = device
    uci["wifi-device"]["type"] = device
    uci["wifi-device"]["vendor"] = "ralink"
    uci["wifi-device"]["iface"] = {}

    local i = 1 -- index of wifi-iface

    uci["iface"] = {}
    while i <= tonumber(cfgs.BssidNum) do
        uci["iface"][i] = {}
        local iface = uci["iface"][i]
        iface["ssid"] = cfgs["SSID"..(i)]
        iface["mode"] = "ap"
        iface["network"] = "lan"
        iface["ifname"] = "ra0"
        iface[".name"] = device.."."..iface["ifname"]

        i=i+1
    end

    shuci.encode(uci, ucifile)
end

function mtkwifi.uci2dat(ucifile, devname, datfile)
    local shuci = require("shuci")
    local uci = shuci.decode(ucifile)
    local cfgs = mtkwifi.load_profile(datfile) or {}

    if not ucifile or not devname then return end

    for _,dev in ipairs(uci["wifi-device"][devname]) do
        for k,v in pairs(dev) do
            if string.byte(k) ~= string.byte(".")
            and string.byte(k) ~= string.byte("_") then
                cfgs.k = v
            end
        end
    end
    if datfile then
        save_profile(cfgs, datfile)
    end
end

function mtkwifi.get_referer_url()
    local to_url
    local script_name = luci.http.getenv('SCRIPT_NAME')
    local http_referer = luci.http.getenv('HTTP_REFERER')
    if script_name and http_referer then
        local fIdx = http_referer:find(script_name,1,true)
        if fIdx then
            to_url = http_referer:sub(fIdx)
        end
    end
    if not to_url or to_url == "" then
        to_url = luci.dispatcher.build_url("admin", "network", "wifi")
    end
    return to_url
end

return mtkwifi
