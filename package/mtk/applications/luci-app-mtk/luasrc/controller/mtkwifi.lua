-- This module is a demo to configure MTK' proprietary WiFi driver.
-- Basic idea is to bypass uci and edit wireless profile (mt76xx.dat) directly.
-- LuCI's WiFi configuration is more logical and elegent, but it's quite tricky to 
-- translate uci into MTK's WiFi profile (like we did in "uci2dat").
-- And you will get your hands dirty.
-- 
-- Hua Shao <nossiac@163.com>

package.path = '/lib/wifi/?.lua;'..package.path
module("luci.controller.mtkwifi", package.seeall)

local ioctl_help = require "ioctl_helper"
local http = require("luci.http")
local i18n = require("luci.i18n")
local mtkwifi = require("mtkwifi")
local sys = require "luci.sys"
local jsc = require "luci.jsonc"
local nfs = require "nixio.fs"

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

function index()
    entry({"admin", "network", "wifi"}, template("admin_mtk/mtk_wifi_overview"), _("Wireless"), 10)
    entry({"admin", "network", "wifi", "test"}, call("test")).leaf = true
    entry({"admin", "network", "wifi", "chip_cfg_view"}, template("admin_mtk/mtk_wifi_chip_cfg")).leaf = true
    entry({"admin", "network", "wifi", "chip_cfg"}, call("chip_cfg")).leaf = true
    entry({"admin", "network", "wifi", "dev_cfg_view"}, template("admin_mtk/mtk_wifi_dev_cfg")).leaf = true
    entry({"admin", "network", "wifi", "dev_cfg"}, call("dev_cfg")).leaf = true
    entry({"admin", "network", "wifi", "dev_cfg_raw"}, call("dev_cfg_raw")).leaf = true
    entry({"admin", "network", "wifi", "vif_cfg_view"}, template("admin_mtk/mtk_wifi_vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_cfg"}, call("vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_add_view"}, template("admin_mtk/mtk_wifi_vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_add"}, call("vif_cfg")).leaf = true
    entry({"admin", "network", "wifi", "vif_del"}, call("vif_del")).leaf = true
    entry({"admin", "network", "wifi", "vif_disable"}, call("vif_disable")).leaf = true
    entry({"admin", "network", "wifi", "vif_enable"}, call("vif_enable")).leaf = true
    entry({"admin", "network", "wifi", "get_station_list"}, call("get_station_list"))
    entry({"admin", "network", "wifi", "get_country_region_list"}, call("get_country_region_list")).leaf = true
    entry({"admin", "network", "wifi", "get_channel_list"}, call("get_channel_list"))
    entry({"admin", "network", "wifi", "get_HT_ext_channel_list"}, call("get_HT_ext_channel_list"))
    entry({"admin", "network", "wifi", "get_5G_2nd_80Mhz_channel_list"}, call("get_5G_2nd_80Mhz_channel_list"))
    entry({"admin", "network", "wifi", "reset"}, call("reset_wifi")).leaf = true
    entry({"admin", "network", "wifi", "reload"}, call("reload_wifi")).leaf = true
    entry({"admin", "network", "wifi", "get_raw_profile"}, call("get_raw_profile"))
    entry({"admin", "network", "wifi", "apcli_cfg_view"}, template("admin_mtk/mtk_wifi_apcli")).leaf = true
    entry({"admin", "network", "wifi", "apcli_cfg"}, call("apcli_cfg")).leaf = true
    entry({"admin", "network", "wifi", "apcli_disconnect"}, call("apcli_disconnect")).leaf = true
    entry({"admin", "network", "wifi", "apcli_connect"}, call("apcli_connect")).leaf = true    
    entry({"admin", "network", "wifi", "apcli_scan"}, call("apcli_scan")).leaf = true;
    entry({"admin", "network", "wifi", "sta_info"}, call("sta_info")).leaf = true;
    entry({"admin", "network", "wifi", "get_apcli_conn_info"}, call("get_apcli_conn_info")).leaf = true;
    entry({"admin", "network", "wifi", "apply_power_boost_settings"}, call("apply_power_boost_settings")).leaf = true;
    entry({"admin", "network", "wifi", "apply_reboot"}, template("admin_mtk/mtk_wifi_apply_reboot")).leaf = true;
    entry({"admin", "network", "wifi", "reboot"}, call("exec_reboot")).leaf = true;
    entry({"admin", "network", "wifi", "get_bssid_num"}, call("get_bssid_num")).leaf = true;
    entry({"admin", "network", "wifi", "loading"}, template("admin_mtk/mtk_wifi_loading")).leaf = true;
    entry({"admin", "network", "wifi", "get_apply_status"}, call("get_apply_status")).leaf = true;
    entry({"admin", "network", "wifi", "reset_to_defaults"}, call("reset_to_defaults")).leaf = true;
end

function test()
    http.write_json(http.formvalue())
end

function exec_reboot()
    os.execute("rm -f /tmp/mtk/wifi/reboot_required >/dev/null 2>&1")
    os.execute("sync >/dev/null 2>&1")
    os.execute("reboot >/dev/null 2>&1")
end

function get_apply_status()
    local ret = {}

    if mtkwifi.is_child_active() then
        ret["status"] = "ON_PROGRESS"
    elseif mtkwifi.exists("/tmp/mtk/wifi/reboot_required") then
        -- If the "wifi restart" command can not re-install the driver; then, it will create
        -- "/tmp/mtk/wifi/reboot_required" file to indicate LuCI that the settings will be applied
        -- only after reboot of the device.
        -- Redirect "Reboot Device" web-page to get consent from the user to reboot the device.
        ret["status"] = "REBOOT"
    else
        ret["status"] = "DONE"
    end
    http.write_json(ret)
end

function __mtkwifi_save_profile(cfgs, path, isProfileSettingsAppliedToDriver)
    -- Create the applied settings backup file before saving the new profile settings only if it does not exist.
    if not mtkwifi.exists(mtkwifi.__profile_applied_settings_path(path)) then
        os.execute("cp -f "..path.." "..mtkwifi.__profile_applied_settings_path(path))
    end
    if isProfileSettingsAppliedToDriver then
        -- It means the some context based profile settings to be saved in DAT file is already applied to the driver.
        -- Find the profile settings which are not applied to the driver before saving the new profile settings
        local diff = mtkwifi.diff_profile(path)
        mtkwifi.save_profile(cfgs, path)
        -- If there are any settings which are not applied to the driver, then do NOT copy and WebUI will display the "need reload to apply changes" message
        -- Otherwise, copy the new profile settings and WebUI will NOT display the "need reload to apply changes" message
        if next(diff) == nil then
            os.execute("cp -f "..path.." "..mtkwifi.__profile_applied_settings_path(path))
        end
    else
        mtkwifi.save_profile(cfgs, path)
    end
end

local __get_default_wan_ifname = function()
    local boardinfo = jsc.parse(nfs.readfile("/etc/board.json") or "")
    local wan_ifname = "eth1"

    if type(boardinfo) == "table" and type(boardinfo.network.wan.device) == "string" then
        wan_ifname = boardinfo.network.wan.device
    end

    return wan_ifname
end

local __setup_wan = function(devname)
    local devs = mtkwifi.get_all_devs()
    local profiles = mtkwifi.search_dev_and_profile()
    local cfgs = mtkwifi.load_profile(profiles[devname])
    local dev = devs and devs[devname]
    local vif = __get_default_wan_ifname()

    if cfgs.ApCliEnable ~= "0" and cfgs.ApCliEnable ~= "" and dev.apcli.vifname then
        vif = dev.apcli.vifname
    end

    nixio.syslog("debug", "Set wan/wan6 to "..vif)

    os.execute("uci set network.wan.device="..vif)
    os.execute("uci set network.wan6.device="..vif)
    os.execute("uci commit")
    os.execute("ifup wan")
    os.execute("ifup wan6")

end

local __mtkwifi_reload = function (devname)
    local wifi_restart = false
    local wifi_reload = false
    local change_wan = false
    local profiles = mtkwifi.search_dev_and_profile()

    for dev,profile in pairs(profiles) do
        if not devname or devname == dev then
            local diff = mtkwifi.diff_profile(profile)
            __process_settings_before_apply(dev, profile, diff)

            if diff.BssidNum or diff.WHNAT or diff.E2pAccessMode or diff.HT_RxStream or diff.HT_TxStream or diff.WdsEnable then
                -- Addition or deletion of a vif requires re-installation of the driver.
                -- Change in WHNAT setting also requires re-installation of the driver.
                -- Driver will be re-installed by "wifi restart" command.
                wifi_restart = true
            else
                wifi_reload = true
            end

            if diff.ApCliEnable then
                change_wan = true
            end
        end
    end

    if change_wan and devname then
        __setup_wan(devname)
    end

    if wifi_restart then
        os.execute("wifi restart "..(devname or ""))
        debug_write("wifi restart "..(devname or ""))
    elseif wifi_reload then
        os.execute("wifi reload "..(devname or ""))
        debug_write("wifi reload "..(devname or ""))
    end

    for dev,profile in pairs(profiles) do
        if not devname or devname == dev then
            -- keep a backup for this commit
            -- it will be used in mtkwifi.diff_profile()
            os.execute("cp -f "..profile.." "..mtkwifi.__profile_applied_settings_path(profile))
            debug_write("cp -f "..profile.." "..mtkwifi.__profile_applied_settings_path(profile))
        end
    end

end

function __process_settings_before_apply(devname, profile, diff)
    local devs = mtkwifi.get_all_devs()
    local cfgs = mtkwifi.load_profile(profile)
    __apply_wifi_wpsconf(devs, devname, cfgs, diff)
end

function chip_cfg(devname)
    local profiles = mtkwifi.search_dev_and_profile()
    assert(profiles[devname])
    local cfgs = mtkwifi.load_profile(profiles[devname])
    local devs = mtkwifi.get_all_devs()
    local dbdc_cfgs = {}
    local dev = {}
    dev = devs and devs[devname]

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "chip_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) == string.byte("_") then
            nixio.syslog("err", "chip_cfg, special: "..k.."="..v)
        else
            if dev.dbdc == true then
                dbdc_cfgs[k] = v or ""
            else
                cfgs[k] = v or ""
            end
        end
    end

    -- VOW
    -- ATC should actually be scattered into each SSID, but I'm just lazy.
    if cfgs.VOW_Airtime_Fairness_En then
    for i = 1,tonumber(cfgs.BssidNum) do
        __atc_tp     = http.formvalue("__atc_vif"..i.."_tp")     or "0"
        __atc_min_tp = http.formvalue("__atc_vif"..i.."_min_tp") or "0"
        __atc_max_tp = http.formvalue("__atc_vif"..i.."_max_tp") or "0"
        __atc_at     = http.formvalue("__atc_vif"..i.."_at")     or "0"
        __atc_min_at = http.formvalue("__atc_vif"..i.."_min_at") or "0"
        __atc_max_at = http.formvalue("__atc_vif"..i.."_max_at") or "0"

        nixio.syslog("info", "ATC.__atc_tp     ="..i..__atc_tp     );
        nixio.syslog("info", "ATC.__atc_min_tp ="..i..__atc_min_tp );
        nixio.syslog("info", "ATC.__atc_max_tp ="..i..__atc_max_tp );
        nixio.syslog("info", "ATC.__atc_at     ="..i..__atc_at     );
        nixio.syslog("info", "ATC.__atc_min_at ="..i..__atc_min_at );
        nixio.syslog("info", "ATC.__atc_max_at ="..i..__atc_max_at );

        dbdc_cfgs.VOW_Rate_Ctrl_En    = mtkwifi.token_set(cfgs.VOW_Rate_Ctrl_En,    i, __atc_tp)
        dbdc_cfgs.VOW_Group_Min_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Min_Rate,  i, __atc_min_tp)
        dbdc_cfgs.VOW_Group_Max_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Max_Rate,  i, __atc_max_tp)

        dbdc_cfgs.VOW_Airtime_Ctrl_En = mtkwifi.token_set(cfgs.VOW_Airtime_Ctrl_En, i, __atc_at)
        dbdc_cfgs.VOW_Group_Min_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Min_Ratio, i, __atc_min_at)
        dbdc_cfgs.VOW_Group_Max_Ratiio = mtkwifi.token_set(cfgs.VOW_Group_Max_Ratio, i, __atc_max_at)

        cfgs.VOW_Rate_Ctrl_En    = mtkwifi.token_set(cfgs.VOW_Rate_Ctrl_En,    i, __atc_tp)
        cfgs.VOW_Group_Min_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Min_Rate,  i, __atc_min_tp)
        cfgs.VOW_Group_Max_Rate  = mtkwifi.token_set(cfgs.VOW_Group_Max_Rate,  i, __atc_max_tp)

        cfgs.VOW_Airtime_Ctrl_En = mtkwifi.token_set(cfgs.VOW_Airtime_Ctrl_En, i, __atc_at)
        cfgs.VOW_Group_Min_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Min_Ratio, i, __atc_min_at)
        cfgs.VOW_Group_Max_Ratio = mtkwifi.token_set(cfgs.VOW_Group_Max_Ratio, i, __atc_max_at)

    end

    dbdc_cfgs.VOW_RX_En = http.formvalue("VOW_RX_En") or "0"
    cfgs.VOW_RX_En = http.formvalue("VOW_RX_En") or "0"
    end

    if dev.dbdc == true then
        for devname, profile in pairs(profiles) do
            __mtkwifi_save_profile(dbdc_cfgs, profile, false)
        end
    else
        __mtkwifi_save_profile(cfgs, profiles[devname], false)
    end

    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "network", "wifi", "chip_cfg_view",devname)
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "chip_cfg_view",devname))
    end

end

function dev_cfg(devname)
    local profiles = mtkwifi.search_dev_and_profile()
    assert(profiles[devname])
    local cfgs = mtkwifi.load_profile(profiles[devname])

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "dev_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) == string.byte("_") then
            nixio.syslog("err", "dev_cfg, special: "..k.."="..v)
        else
            cfgs[k] = v or ""
        end
    end

    if cfgs.Channel == "0" then -- Auto Channel Select
        cfgs.AutoChannelSelect = "3"
    else
        cfgs.AutoChannelSelect = "0"
    end

    if http.formvalue("__bw") == "20" then
        cfgs.HT_BW = 0
        cfgs.VHT_BW = 0
    elseif http.formvalue("__bw") == "40" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 0
        cfgs.HT_BSSCoexistence = 0
    elseif http.formvalue("__bw") == "60" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 0
        cfgs.HT_BSSCoexistence = 1
    elseif http.formvalue("__bw") == "80" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 1
    elseif http.formvalue("__bw") == "160" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 2
    elseif http.formvalue("__bw") == "161" then
        cfgs.HT_BW = 1
        cfgs.VHT_BW = 3
        cfgs.VHT_Sec80_Channel = http.formvalue("VHT_Sec80_Channel") or ""
    end

    if mtkwifi.band(string.split(cfgs.WirelessMode,";")[1]) == "5G" or mtkwifi.band(cfgs.WirelessMode) == "6G" then
        cfgs.CountryRegionABand = http.formvalue("__cr");
    else
        cfgs.CountryRegion = http.formvalue("__cr");
    end

    if http.formvalue("TxPower") then
        local txpower = tonumber(http.formvalue("TxPower"))
        if txpower < 100 then
            cfgs.PERCENTAGEenable=1
        else
            cfgs.PERCENTAGEenable=0
        end
    end

    local mimo = http.formvalue("__mimo")
    if mimo == "0" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=0
    elseif mimo == "1" then
        cfgs.ETxBfEnCond=0
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "2" then
        cfgs.ETxBfEnCond=1
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=1
    elseif mimo == "3" then
        cfgs.ETxBfEnCond=1
        if tonumber(cfgs.ApCliEnable) == 1 then
            cfgs.MUTxRxEnable=3
        else
            cfgs.MUTxRxEnable=1
        end
        cfgs.ITxBfEn=0
    elseif mimo == "4" then
        cfgs.ETxBfEnCond=1
        if tonumber(cfgs.ApCliEnable) == 1 then
            cfgs.MUTxRxEnable=3
        else
            cfgs.MUTxRxEnable=1
        end
        cfgs.ITxBfEn=1
    else
        cfgs.ETxBfEnCond=0
        cfgs.MUTxRxEnable=0
        cfgs.ITxBfEn=0
    end

--    if cfgs.ApCliEnable == "1" then
--        cfgs.Channel = http.formvalue("__apcli_channel")
--    end

    -- WDS
    -- http.write_json(http.formvalue())
    __mtkwifi_save_profile(cfgs, profiles[devname], false)

    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "network", "wifi", "dev_cfg_view",devname)
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "dev_cfg_view",devname))
    end
end

function dev_cfg_raw(devname)
    -- http.write_json(http.formvalue())
    local profiles = mtkwifi.search_dev_and_profile()
    assert(profiles[devname])

    local raw = http.formvalue("raw")
    raw = string.gsub(raw, "\r\n", "\n")
    local cfgs = mtkwifi.load_profile(nil, raw)
    __mtkwifi_save_profile(cfgs, profiles[devname], false)

    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "dev_cfg_view", devname))
end

function __delete_mbss_para(cfgs, vif_idx)
    debug_write("delete_mbss_para vif: ", vif_idx)
    cfgs["WPAPSK"..vif_idx]=""
    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vif_idx,"")
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vif_idx,"")
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vif_idx,"")
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vif_idx,"")
    cfgs["RADIUS_Server"]=mtkwifi.token_set(cfgs["RADIUS_Server"],vif_idx,"")
    cfgs["RADIUS_Port"]=mtkwifi.token_set(cfgs["RADIUS_Port"],vif_idx,"")
    cfgs["RADIUS_Key"..vif_idx]=""
    cfgs["DefaultKeyID"]=mtkwifi.token_set(cfgs["DefaultKeyID"],vif_idx,"")
    cfgs["IEEE8021X"]=mtkwifi.token_set(cfgs["IEEE8021X"],vif_idx,"")
    cfgs["WscConfMode"]=mtkwifi.token_set(cfgs["WscConfMode"],vif_idx,"")
    cfgs["PreAuth"]=mtkwifi.token_set(cfgs["PreAuth"],vif_idx,"")
    cfgs["HT_STBC"] = mtkwifi.token_set(cfgs["HT_STBC"],vif_idx,"")
    cfgs["HT_LDPC"] = mtkwifi.token_set(cfgs["HT_LDPC"],vif_idx,"")
    cfgs["VHT_STBC"] = mtkwifi.token_set(cfgs["VHT_STBC"],vif_idx,"")
    cfgs["VHT_LDPC"] = mtkwifi.token_set(cfgs["VHT_LDPC"],vif_idx,"")
    cfgs["HideSSID"]=mtkwifi.token_set(cfgs["HideSSID"],vif_idx,"")
    cfgs["NoForwarding"]=mtkwifi.token_set(cfgs["NoForwarding"],vif_idx,"")
    cfgs["WmmCapable"]=mtkwifi.token_set(cfgs["WmmCapable"],vif_idx,"")
    cfgs["TxRate"]=mtkwifi.token_set(cfgs["TxRate"],vif_idx,"")
    cfgs["RekeyInterval"]=mtkwifi.token_set(cfgs["RekeyInterval"],vif_idx,"")
    cfgs["AuthMode"]=mtkwifi.token_set(cfgs["AuthMode"],vif_idx,"")
    cfgs["EncrypType"]=mtkwifi.token_set(cfgs["EncrypType"],vif_idx,"")
    cfgs["session_timeout_interval"]=mtkwifi.token_set(cfgs["session_timeout_interval"],vif_idx,"")
    --cfgs["WscModeOption"]=mtkwifi.token_set(cfgs["WscModeOption"],vif_idx,"")
    cfgs["RekeyMethod"]=mtkwifi.token_set(cfgs["RekeyMethod"],vif_idx,"")
    cfgs["PMFMFPC"] = mtkwifi.token_set(cfgs["PMFMFPC"],vif_idx,"")
    cfgs["PMFMFPR"] = mtkwifi.token_set(cfgs["PMFMFPR"],vif_idx,"")
    cfgs["PMFSHA256"] = mtkwifi.token_set(cfgs["PMFSHA256"],vif_idx,"")
    cfgs["PMKCachePeriod"] = mtkwifi.token_set(cfgs["PMKCachePeriod"],vif_idx,"")
    cfgs["Wapiifname"] = mtkwifi.token_set(cfgs["Wapiifname"],vif_idx,"")
    cfgs["RRMEnable"] = mtkwifi.token_set(cfgs["RRMEnable"],vif_idx,"")
    cfgs["DLSCapable"] = mtkwifi.token_set(cfgs["DLSCapable"],vif_idx,"")
    cfgs["APSDCapable"] = mtkwifi.token_set(cfgs["APSDCapable"],vif_idx,"")
    cfgs["FragThreshold"] = mtkwifi.token_set(cfgs["FragThreshold"],vif_idx,"")
    cfgs["RTSThreshold"] = mtkwifi.token_set(cfgs["RTSThreshold"],vif_idx,"")
    cfgs["VHT_SGI"] = mtkwifi.token_set(cfgs["VHT_SGI"],vif_idx,"")
    cfgs["VHT_BW_SIGNAL"] = mtkwifi.token_set(cfgs["VHT_BW_SIGNAL"],vif_idx,"")
    cfgs["HT_PROTECT"] = mtkwifi.token_set(cfgs["HT_PROTECT"],vif_idx,"")
    cfgs["HT_GI"] = mtkwifi.token_set(cfgs["HT_GI"],vif_idx,"")
    cfgs["HT_OpMode"] = mtkwifi.token_set(cfgs["HT_OpMode"],vif_idx,"")
    --cfgs["HT_TxStream"] = mtkwifi.token_set(cfgs["HT_TxStream"],vif_idx,"")
    --cfgs["HT_RxStream"] = mtkwifi.token_set(cfgs["HT_RxStream"],vif_idx,"")
    cfgs["HT_AMSDU"] = mtkwifi.token_set(cfgs["HT_AMSDU"],vif_idx,"")
    cfgs["HT_AutoBA"] = mtkwifi.token_set(cfgs["HT_AutoBA"],vif_idx,"")
    cfgs["IgmpSnEnable"] = mtkwifi.token_set(cfgs["IgmpSnEnable"],vif_idx,"")
    cfgs["WirelessMode"] = mtkwifi.token_set(cfgs["WirelessMode"],vif_idx,"")
    cfgs["WdsEnable"] = mtkwifi.token_set(cfgs["WdsEnable"],vif_idx,"")
    cfgs["MuOfdmaDlEnable"] = mtkwifi.token_set(cfgs["MuOfdmaDlEnable"],vif_idx,"")
    cfgs["MuOfdmaUlEnable"] = mtkwifi.token_set(cfgs["MuOfdmaUlEnable"],vif_idx,"")
    cfgs["MuMimoDlEnable"] = mtkwifi.token_set(cfgs["MuMimoDlEnable"],vif_idx,"")
    cfgs["MuMimoUlEnable"] = mtkwifi.token_set(cfgs["MuMimoUlEnable"],vif_idx,"")

end

function vif_del(dev, vif)
    debug_write("vif_del("..dev..vif..")")
    local devname,vifname = dev, vif
    debug_write("devname="..devname)
    debug_write("vifname="..vifname)
    local devs = mtkwifi.get_all_devs()
    local idx = devs[devname]["vifs"][vifname].vifidx -- or tonumber(string.match(vifname, "%d+")) + 1
    debug_write("idx="..idx, devname, vifname)
    local profile = devs[devname].profile 
    assert(profile)
    if idx and tonumber(idx) >= 0 then
        local cfgs = mtkwifi.load_profile(profile)
        __delete_mbss_para(cfgs, idx)
        if cfgs then
            debug_write("ssid"..idx.."="..cfgs["SSID"..idx].."<br>")
            cfgs["SSID"..idx] = ""
            debug_write("ssid"..idx.."="..cfgs["SSID"..idx].."<br>")
            debug_write("wpapsk"..idx.."="..cfgs["WPAPSK"..idx].."<br>")
            cfgs["WPAPSK"..idx] = ""
            local ssidlist = {}
            local j = 1
            for i = 1,16 do
                if cfgs["SSID"..i] ~= "" then
                    ssidlist[j] =  cfgs["SSID"..i]
                    j = j + 1
                end
            end
            for i,v in ipairs(ssidlist) do
                debug_write("ssidlist"..i.."="..v)
            end
            debug_write("cfgs.BssidNum="..cfgs.BssidNum.." #ssidlist="..#ssidlist)
            assert(tonumber(cfgs.BssidNum) == #ssidlist + 1, "BssidNum="..cfgs.BssidNum.." SSIDlist="..#ssidlist..", BssidNum count does not match with SSIDlist count.")
            cfgs.BssidNum = #ssidlist
            for i = 1,16 do
                if i <= cfgs.BssidNum then
                    cfgs["SSID"..i] = ssidlist[i]
                elseif cfgs["SSID"..i] then
                    cfgs["SSID"..i] = ""
                end
            end

            __mtkwifi_save_profile(cfgs, profile, false)
        else
            debug_write(profile.." cannot be found!")
        end
    end
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function vif_disable(iface)
    os.execute("ifconfig "..iface.." down")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function vif_enable(iface)
    os.execute("ifconfig "..iface.." up")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end


--[[
-- security config in mtk wifi is quite complicated!
-- cfgs listed below are attached with vif and combined like "0;0;0;0". They need specicial treatment.
        TxRate, WmmCapable, NoForwarding,
        HideSSID, IEEE8021X, PreAuth,
        AuthMode, EncrypType, RekeyMethod,
        RekeyInterval, PMKCachePeriod,
        DefaultKeyId, Key{n}Type, HT_EXTCHA,
        RADIUS_Server, RADIUS_Port,
]]

local function conf_wep_keys(cfgs,vifidx)
    cfgs.DefaultKeyID = mtkwifi.token_set(cfgs.DefaultKeyID, vifidx, http.formvalue("__DefaultKeyID") or 1)
    cfgs["Key1Str"..vifidx]  = http.formvalue("Key1Str"..vifidx)
    cfgs["Key2Str"..vifidx]  = http.formvalue("Key2Str"..vifidx)
    cfgs["Key3Str"..vifidx]  = http.formvalue("Key3Str"..vifidx)
    cfgs["Key4Str"..vifidx]  = http.formvalue("Key4Str"..vifidx)

    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vifidx, http.formvalue("WEP1Type"..vifidx))
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vifidx, http.formvalue("WEP2Type"..vifidx))
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vifidx, http.formvalue("WEP3Type"..vifidx))
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vifidx, http.formvalue("WEP4Type"..vifidx))

    return cfgs
end

local function __security_cfg(cfgs, vif_idx)
    -- Reset/Clear all necessary settings here. Later, these settings will be set as per AuthMode.
    cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "DISABLE")
    cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "0")
    cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "0")
    cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "0")
    cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    -- Update the settings which are not dependent on AuthMode
    cfgs.HideSSID = mtkwifi.token_set(cfgs.HideSSID, vif_idx, http.formvalue("__hidessid") or "0")
    cfgs.NoForwarding = mtkwifi.token_set(cfgs.NoForwarding, vif_idx, http.formvalue("__noforwarding") or "0")
    cfgs.WmmCapable = mtkwifi.token_set(cfgs.WmmCapable, vif_idx, http.formvalue("__wmmcapable") or "0")
    cfgs.TxRate = mtkwifi.token_set(cfgs.TxRate, vif_idx, http.formvalue("__txrate") or "0");
    cfgs.RekeyInterval = mtkwifi.token_set(cfgs.RekeyInterval, vif_idx, http.formvalue("__rekeyinterval") or "0");

    local __authmode = http.formvalue("__authmode") or "Disable"
    cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, __authmode)
    cfgs.PMFMFPC = "0"
    cfgs.PMFMFPR = "0"
    cfgs.PMFSHA256 = "0"

    if __authmode == "Disable" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OPEN")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "NONE")

    elseif __authmode == "OPEN" or __authmode == "SHARED" or __authmode == "WEPAUTO" then
        cfgs.WscModeOption = "0"
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "WEP")
        cfgs = conf_wep_keys(cfgs,vif_idx)

    elseif __authmode == "Enhanced Open" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OWE")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "AES")
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPAPSK"  then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")

    elseif __authmode == "WPAPSKWPA2PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")

    elseif __authmode == "WPA2PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, http.formvalue("__pmfsha256") or "0")

    elseif __authmode == "WPA3PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA2PSKWPA3PSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA2" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, http.formvalue("__pmfmfpc") or "0")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, http.formvalue("__pmfmfpr") or "0")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, http.formvalue("__pmfsha256") or "0")

    elseif __authmode == "WPA3" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA3-192-bit" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "WPA3-192")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "GCMP256")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")
        -- for DOT11W_PMF_SUPPORT
        cfgs.PMFMFPC = mtkwifi.token_set(cfgs.PMFMFPC, vif_idx, "1")
        cfgs.PMFMFPR = mtkwifi.token_set(cfgs.PMFMFPR, vif_idx, "1")
        cfgs.PMFSHA256 = mtkwifi.token_set(cfgs.PMFSHA256, vif_idx, "0")

    elseif __authmode == "WPA1WPA2" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__encrypttype") or "AES")
        cfgs.RekeyMethod = mtkwifi.token_set(cfgs.RekeyMethod, vif_idx, "TIME")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "1812")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")
        cfgs.PMKCachePeriod = mtkwifi.token_set(cfgs.PMKCachePeriod, vif_idx, http.formvalue("__pmkcacheperiod") or "0")
        cfgs.PreAuth = mtkwifi.token_set(cfgs.PreAuth, vif_idx, http.formvalue("__preauth") or "0")

    elseif __authmode == "IEEE8021X" then
        cfgs.AuthMode = mtkwifi.token_set(cfgs.AuthMode, vif_idx, "OPEN")
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, http.formvalue("__8021x_wep") and "WEP" or "NONE")
        cfgs.IEEE8021X = mtkwifi.token_set(cfgs.IEEE8021X, vif_idx, "1")
        cfgs.RADIUS_Server = mtkwifi.token_set(cfgs.RADIUS_Server, vif_idx, http.formvalue("__radius_server") or "0")
        cfgs.RADIUS_Port = mtkwifi.token_set(cfgs.RADIUS_Port, vif_idx, http.formvalue("__radius_port") or "0")
        cfgs.session_timeout_interval = mtkwifi.token_set(cfgs.session_timeout_interval, vif_idx, http.formvalue("__session_timeout_interval") or "0")

    elseif __authmode == "WAICERT" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "SMS4")
        cfgs.Wapiifname = mtkwifi.token_set(cfgs.Wapiifname, vif_idx, "br-lan")
        -- cfgs.wapicert_asipaddr
        -- cfgs.WapiAsPort
        -- cfgs.wapicert_ascert
        -- cfgs.wapicert_usercert

    elseif __authmode == "WAIPSK" then
        cfgs.EncrypType = mtkwifi.token_set(cfgs.EncrypType, vif_idx, "SMS4")
        -- cfgs.wapipsk_keytype
        -- cfgs.wapipsk_prekey
    end
end

function initialize_multiBssParameters(cfgs,vif_idx)
    cfgs["WPAPSK"..vif_idx]="12345678"
    cfgs["Key1Type"]=mtkwifi.token_set(cfgs["Key1Type"],vif_idx,"0")
    cfgs["Key2Type"]=mtkwifi.token_set(cfgs["Key2Type"],vif_idx,"0")
    cfgs["Key3Type"]=mtkwifi.token_set(cfgs["Key3Type"],vif_idx,"0")
    cfgs["Key4Type"]=mtkwifi.token_set(cfgs["Key4Type"],vif_idx,"0")
    cfgs["RADIUS_Server"]=mtkwifi.token_set(cfgs["RADIUS_Server"],vif_idx,"0")
    cfgs["RADIUS_Port"]=mtkwifi.token_set(cfgs["RADIUS_Port"],vif_idx,"1812")
    cfgs["RADIUS_Key"..vif_idx]="ralink"
    cfgs["DefaultKeyID"]=mtkwifi.token_set(cfgs["DefaultKeyID"],vif_idx,"1")
    cfgs["IEEE8021X"]=mtkwifi.token_set(cfgs["IEEE8021X"],vif_idx,"0")
    cfgs["WscConfMode"]=mtkwifi.token_set(cfgs["WscConfMode"],vif_idx,"0")
    cfgs["PreAuth"]=mtkwifi.token_set(cfgs["PreAuth"],vif_idx,"0")
    return cfgs
end

function __wps_ap_pbc_start_all(ifname)
    os.execute("iwpriv "..ifname.." set WscMode=2");
    os.execute("iwpriv "..ifname.." set WscGetConf=1");
end

function __wps_ap_pin_start_all(ifname, pincode)
    os.execute("iwpriv "..ifname.." set WscMode=1")
    os.execute("iwpriv "..ifname.." set WscPinCode="..pincode)
    os.execute("iwpriv "..ifname.." set WscGetConf=1")
end

function __apply_wifi_wpsconf(devs, devname, cfgs, diff)
    local saved = cfgs.WscConfMode and cfgs.WscConfMode:gsub(";-(%d);-","%1") or ""
    local applied = diff.WscConfMode and diff["WscConfMode"][2]:gsub(";-(%d);-","%1") or ""
    local num_ifs = tonumber(cfgs.BssidNum) or 0

    for idx=1, num_ifs do
        local ifname = devs[devname]["vifs"][idx]["vifname"]
        if mtkwifi.__any_wsc_enabled(saved:sub(idx,idx)) == 1 then
            cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, idx, "2")
        else
            cfgs.WscConfStatus = mtkwifi.token_set(cfgs.WscConfStatus, idx, "1")
        end
        if (diff.WscConfMode) and saved:sub(idx,idx) ~= applied:sub(idx,idx) then
            cfgs = mtkwifi.__restart_if_wps(devname, ifname, cfgs)
        end
    end

    -- __mtkwifi_save_profile() is called outside the loop because it is a high time consuming function.
    __mtkwifi_save_profile(cfgs, devs[devname]["profile"], false)

end

function __set_wifi_wpsconf(cfgs, wsc_enable, vif_idx)
    debug_write("__set_wifi_wpsconf : wsc_enable = ",wsc_enable)
    if(wsc_enable == "1") then
        cfgs["WscConfMode"] = mtkwifi.token_set(cfgs["WscConfMode"], vif_idx, "7")
    else
        cfgs["WscConfMode"] = mtkwifi.token_set(cfgs["WscConfMode"], vif_idx, "0")
    end
    if(((http.formvalue("__authmode")=="OPEN") and
        (http.formvalue("__encrypttype") == "WEP")) or
       (http.formvalue("__hidessid") == "1")) then
        cfgs.WscConfMode = mtkwifi.token_set(cfgs.WscConfMode, vif_idx, "0")
    end
    debug_write("__set_wifi_wpsconf : WscConfMode = ",cfgs["WscConfMode"])
end

function __update_mbss_para(cfgs, vif_idx)
    debug_write("update_mbss_para vif: ", vif_idx)
    cfgs.HT_STBC = mtkwifi.token_set(cfgs.HT_STBC, vif_idx, http.formvalue("__ht_stbc") or "0")
    cfgs.HT_LDPC = mtkwifi.token_set(cfgs.HT_LDPC, vif_idx, http.formvalue("__ht_ldpc") or "0")
    cfgs.VHT_STBC = mtkwifi.token_set(cfgs.VHT_STBC, vif_idx, http.formvalue("__vht_stbc") or "0")
    cfgs.VHT_LDPC = mtkwifi.token_set(cfgs.VHT_LDPC, vif_idx, http.formvalue("__vht_ldpc") or "0")
    cfgs.DLSCapable = mtkwifi.token_set(cfgs.DLSCapable, vif_idx, http.formvalue("__dls_capable") or "0")
    cfgs.RRMEnable = mtkwifi.token_set(cfgs.RRMEnable, vif_idx, http.formvalue("__rrmenable") or "0")
    cfgs.APSDCapable = mtkwifi.token_set(cfgs.APSDCapable, vif_idx, http.formvalue("__apsd_capable") or "0")
    cfgs.FragThreshold = mtkwifi.token_set(cfgs.FragThreshold, vif_idx, http.formvalue("__frag_threshold") or "0")
    cfgs.KickStaRssiLow = mtkwifi.token_set(cfgs.KickStaRssiLow, vif_idx, http.formvalue("__kickrssi") or "0")
    cfgs.AssocReqRssiThres = mtkwifi.token_set(cfgs.AssocReqRssiThres, vif_idx, http.formvalue("__assocthres") or "0")
    cfgs.RTSThreshold = mtkwifi.token_set(cfgs.RTSThreshold, vif_idx, http.formvalue("__rts_threshold") or "0")
    cfgs.VHT_SGI = mtkwifi.token_set(cfgs.VHT_SGI, vif_idx, http.formvalue("__vht_sgi") or "0")
    cfgs.VHT_BW_SIGNAL = mtkwifi.token_set(cfgs.VHT_BW_SIGNAL, vif_idx, http.formvalue("__vht_bw_signal") or "0")
    cfgs.HT_PROTECT = mtkwifi.token_set(cfgs.HT_PROTECT, vif_idx, http.formvalue("__ht_protect") or "0")
    cfgs.HT_GI = mtkwifi.token_set(cfgs.HT_GI, vif_idx, http.formvalue("__ht_gi") or "0")
    cfgs.HT_OpMode = mtkwifi.token_set(cfgs.HT_OpMode, vif_idx, http.formvalue("__ht_opmode") or "0")
    cfgs.HT_AMSDU = mtkwifi.token_set(cfgs.HT_AMSDU, vif_idx, http.formvalue("__ht_amsdu") or "0")
    cfgs.HT_AutoBA = mtkwifi.token_set(cfgs.HT_AutoBA, vif_idx, http.formvalue("__ht_autoba") or "0")
    cfgs.IgmpSnEnable = mtkwifi.token_set(cfgs.IgmpSnEnable, vif_idx, http.formvalue("__igmp_snenable") or "0")
    cfgs.WirelessMode = mtkwifi.token_set(cfgs.WirelessMode, vif_idx, http.formvalue("__wirelessmode") or "0")
    cfgs.WdsEnable = mtkwifi.token_set(cfgs.WdsEnable, vif_idx, http.formvalue("__wdsenable") or "0")
    cfgs.MuOfdmaDlEnable = mtkwifi.token_set(cfgs.MuOfdmaDlEnable, vif_idx, http.formvalue("__muofdma_dlenable") or "0")
    cfgs.MuOfdmaUlEnable = mtkwifi.token_set(cfgs.MuOfdmaUlEnable, vif_idx, http.formvalue("__muofdma_ulenable") or "0")
    cfgs.MuMimoDlEnable = mtkwifi.token_set(cfgs.MuMimoDlEnable, vif_idx, http.formvalue("__mumimo_dlenable") or "0")
    cfgs.MuMimoUlEnable = mtkwifi.token_set(cfgs.MuMimoUlEnable, vif_idx, http.formvalue("__mumimo_ulenable") or "0")

end

function vif_cfg(dev, vif)
    local devname, vifname = dev, vif
    if not devname then devname = vif end
    debug_write("devname="..devname)
    debug_write("vifname="..(vifname or ""))
    local devs = mtkwifi.get_all_devs()
    local profile = devs[devname].profile
    assert(profile)

    --local ssid_index;
    --ssid_index = devs[devname]["vifs"][vifname].vifidx

    local cfgs = mtkwifi.load_profile(profile)

    --for k,v in pairs(http.formvalue()) do
    --    if type(v) == type("") or type(v) == type(0) then
    --        nixio.syslog("debug", "post."..k.."="..tostring(v))
    --    else
    --        nixio.syslog("debug", "post."..k.." invalid, type="..type(v))
    --    end
    --end

    -- sometimes vif_idx start from 0, like AccessPolicy0
    -- sometimes it starts from 1, like WPAPSK1. nice!
    local vif_idx
    local to_url
    if http.formvalue("__action") == "vif_cfg_view" then
        vif_idx = devs[devname]["vifs"][vifname].vifidx
        debug_write("vif_idx=", vif_idx, devname, vifname)
        to_url = luci.dispatcher.build_url("admin", "network", "wifi", "vif_cfg_view", devname, vifname)
    elseif http.formvalue("__action") == "vif_add_view" then
        cfgs.BssidNum = tonumber(cfgs.BssidNum) + 1
        vif_idx = tonumber(cfgs.BssidNum)
        to_url = luci.dispatcher.build_url("admin", "network", "wifi")
        -- initializing ; separated parameters for the new interface
        cfgs = initialize_multiBssParameters(cfgs, vif_idx)
    end
    assert(vif_idx)
    assert(to_url)
    -- "__" should not be the prefix of a name if user wants to copy form value data directly to the dat file variable
    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "vif_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
            --debug_write("vif_cfg: Copying",k,v)
            cfgs[k] = v or ""
        end
    end

    -- WDS
    -- Update WdsXKey if respective WdsEncrypType is NONE
    for i=0,3 do
        if (cfgs["Wds"..i.."Key"] and cfgs["Wds"..i.."Key"] ~= "") and
           ((not mtkwifi.token_get(cfgs["WdsEncrypType"],i+1,nil)) or
            ("NONE" == mtkwifi.token_get(cfgs["WdsEncrypType"],i+1,nil))) then
            cfgs["Wds"..i.."Key"] = ""
        end
    end

    cfgs["AccessPolicy"..vif_idx-1] = http.formvalue("__accesspolicy")
    local t = mtkwifi.parse_mac(http.formvalue("__maclist"))
    cfgs["AccessControlList"..vif_idx-1] = table.concat(t, ";")

    __security_cfg(cfgs, vif_idx)
    __update_mbss_para(cfgs, vif_idx)
    __set_wifi_wpsconf(cfgs, http.formvalue("WPSRadio"), vif_idx)

    __mtkwifi_save_profile(cfgs, profile, false)
    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "loading",to_url))
    else
        luci.http.redirect(to_url)
    end
end

function string.tohex(str)
    return (str:gsub('.', function (c)
        return string.format('%02X', string.byte(c))
    end))
end

function unencode_ssid(raw_ssid)
    local c
    local output = ""
    local convertNext = 0
    for c in raw_ssid:gmatch"." do
        if(convertNext == 0) then
            if(c == '+') then
                output = output..' '
            elseif(c == '%') then
                convertNext = 1
            else
                output = output..c
            end
        else
            output = output..string.tohex(c)
            convertNext = 0
        end
    end
    return output
end

function decode_ssid(raw_ssid)
    local output = raw_ssid
    output = output:gsub("&amp;", "&")
    output = output:gsub("&lt;", "<")
    output = output:gsub("&gt;", ">")
    output = output:gsub("&#34;", "\"")
    output = output:gsub("&#39;", "'")
    output = output:gsub("&nbsp;", " ")
    for codenum in raw_ssid:gmatch("&#(%d+);") do
        output = output:gsub("&#"..codenum..";", string.char(tonumber(codenum)))
    end
    return output
end

function get_apcli_conn_info(ifname)
    local rsp = {}
    if not ifname then
        rsp["conn_state"]="Disconnected"
    else
        local flags = tonumber(mtkwifi.read_pipe("cat /sys/class/net/"..ifname.."/flags 2>/dev/null")) or 0
        rsp["infc_state"] = flags%2 == 1 and "up" or "down"
        local iwapcli = mtkwifi.read_pipe("iwconfig "..ifname.." | grep ESSID 2>/dev/null")
        local ssid = string.match(iwapcli, "ESSID:\"(.*)\"")
        iwapcli = mtkwifi.read_pipe("iwconfig "..ifname.." | grep 'Access Point' 2>/dev/null")
        local bssid = string.match(iwapcli, "%x%x:%x%x:%x%x:%x%x:%x%x:%x%x")
        if not ssid or ssid == "" then
            rsp["conn_state"]= "Disconnected"
        else
            rsp["conn_state"] = "Connected"
            rsp["ssid"] = ssid
            rsp["bssid"] = bssid or "N/A"
        end
    end
    http.write_json(rsp)
end

function sta_info(ifname)
    local output = {}
    local stalist = c_StaInfo(ifname)

    local ifname_t = {["ifname"] = ifname}
    table.insert(output, ifname_t)

    local count = 0
    for _ in pairs(stalist) do count = count + 1 end

    local hosts = sys.net.host_hints()

    for i=0, count - 1 do
        if stalist[i].MacAddr then
            if hosts[stalist[i].MacAddr] then
                stalist[i].ipv4 = hosts[stalist[i].MacAddr].ipv4 or ""
                stalist[i].ipv6 = hosts[stalist[i].MacAddr].ipv6 or ""
                stalist[i].hostname = hosts[stalist[i].MacAddr].name or "-"
            else
                stalist[i].ipv4 = ""
                stalist[i].ipv6 = ""
                stalist[i].hostname = "-"
            end
            table.insert(output, stalist[i])
        end
        stalist[i].security = stalist[i].AuthMode.."-"..stalist[i].EncryptMode
    end
    http.write_json(output)
end

function apcli_scan(ifname)
    local aplist = mtkwifi.scan_ap(ifname)
    local convert="";
    for i=1, #aplist do
        convert = c_convert_string_display(aplist[i]["ssid"])
        aplist[i]["original_ssid"] = aplist[i]["ssid"]
        aplist[i]["ssid"] = convert["output"]
    end
    http.write_json(aplist)
end

function get_station_list()
    http.write("get_station_list")
end

function reset_wifi(devname)
    if devname then
        os.execute("cp -f /rom/etc/wireless/"..devname.."/ /etc/wireless/")
    else
        os.execute("cp -rf /rom/etc/wireless /etc/")
    end
    return luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function reload_wifi(devname)
    profiles = mtkwifi.search_dev_and_profile()
    path = profiles[devname]
    if mtkwifi.exists("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".last") then
        os.execute("rm -rf /tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".last")
    end
    mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
    local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "network", "wifi")
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "loading",url_to_visit_after_reload))
end

function get_raw_profile()
    local sid = http.formvalue("sid")
    http.write_json("get_raw_profile")
end

function get_country_region_list()
    local mode = http.formvalue("mode")
    local cr_list;

    if mtkwifi.band(mode) == "5G" then
        cr_list = mtkwifi.CountryRegionList_5G_All
    elseif mtkwifi.band(mode) == "6G" then
        cr_list = mtkwifi.CountryRegionList_6G_All
    else
        cr_list = mtkwifi.CountryRegionList_2G_All
    end

    http.write_json(cr_list)
end

function remove_ch_by_region(ch_list, region)
    for i = #ch_list,2,-1 do
        if not ch_list[i].region[region] then
            table.remove(ch_list, i)
        end
    end
end

function get_channel_list()
    local mode = http.formvalue("mode")
    local region = tonumber(http.formvalue("country_region")) or 1
    local ch_list

    if mtkwifi.band(mode) == "5G" then
        ch_list = mtkwifi.ChannelList_5G_All
    elseif mtkwifi.band(mode) == "6G" then
        ch_list = mtkwifi.ChannelList_6G_All
    else
        ch_list = mtkwifi.ChannelList_2G_All
    end

    remove_ch_by_region(ch_list, region)

    for k,v in ipairs(ch_list) do
        v.text = i18n.translate(v.text)
    end

    http.write_json(ch_list)
end

function get_HT_ext_channel_list()
    local mode = http.formvalue("mode")
    local ch_cur = tonumber(http.formvalue("ch_cur"))
    local region = tonumber(http.formvalue("country_region")) or 1
    local ext_ch_list = {}

    if mtkwifi.band(mode) == "6G" then -- 6G Channel
        local ch_list = mtkwifi.ChannelList_6G_All
        local ext_ch_idx = -1
        local len = 0

        for k, v in ipairs(ch_list) do
            len = len + 1
            if v.channel == ch_cur then
                ext_ch_idx = (k % 2 == 0) and k + 1 or k - 1
            end
        end

        if ext_ch_idx > 0 and ext_ch_idx < len and ch_list[ext_ch_idx].region[region] then
            ext_ch_list[1] = {}
            ext_ch_list[1].val = ext_ch_idx % 2
            ext_ch_list[1].text = ch_list[ext_ch_idx].text
        end
        
    elseif mtkwifi.band(mode) == "2.4G" then -- 2.4G Channel
        local ch_list = mtkwifi.ChannelList_2G_All
        local below_ch = ch_cur - 4
        local above_ch = ch_cur + 4
        local i = 1

        if below_ch > 0 and ch_list[below_ch + 1].region[region] then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 0
            ext_ch_list[i].text = ch_list[below_ch + 1].text
            i = i + 1
        end

        if above_ch <= 14 and ch_list[above_ch + 1].region[region] then
            ext_ch_list[i] = {}
            ext_ch_list[i].val = 1
            ext_ch_list[i].text = ch_list[above_ch + 1].text
        end
    else  -- 5G Channel
        local ch_list = mtkwifi.ChannelList_5G_All
        local ext_ch_idx = -1
        local len = 0

        for k, v in ipairs(ch_list) do
            len = len + 1
            if v.channel == ch_cur then
                ext_ch_idx = (k % 2 == 0) and k + 1 or k - 1
            end
        end

        if ext_ch_idx > 0 and ext_ch_idx < len and ch_list[ext_ch_idx].region[region] then
            ext_ch_list[1] = {}
            ext_ch_list[1].val = ext_ch_idx % 2
            ext_ch_list[1].text = ch_list[ext_ch_idx].text
        end
    end

    for k,v in ipairs(ext_ch_list) do
        v.text = i18n.translate(v.text)
    end

    http.write_json(ext_ch_list)
end

function get_5G_2nd_80Mhz_channel_list()
    local ch_cur = tonumber(http.formvalue("ch_cur"))
    local region = tonumber(http.formvalue("country_region"))
    local ch_list = mtkwifi.ChannelList_5G_2nd_80MHZ_ALL
    local ch_list_5g = mtkwifi.ChannelList_5G_All
    local i, j, test_ch, test_idx
    local bw80_1st_idx = -1

    -- remove adjacent freqencies starting from list tail.
    for i = #ch_list,1,-1 do
        for j = 0,3 do
            if ch_list[i].channel == -1 then
                break
            end

            test_ch = ch_list[i].channel + j * 4
            test_idx = ch_list[i].chidx + j

            if test_ch == ch_cur then
            if i + 1 <= #ch_list and ch_list[i + 1] then
                table.remove(ch_list, i + 1)
            end
            table.remove(ch_list, i)
                bw80_1st_idx = i
                break
            end

            if i == (bw80_1st_idx - 1) or (not ch_list_5g[test_idx].region[region]) then
                table.remove(ch_list, i)
            break
        end
    end
    end

    -- remove unused channel.
    for i = #ch_list,1,-1 do
        if ch_list[i].channel == -1 then
            table.remove(ch_list, i)
        end
    end

    for k,v in ipairs(ch_list) do
        v.text = i18n.translate(v.text)
    end

    http.write_json(ch_list)
end

function apcli_cfg(dev, vif)
    local devname = dev
    debug_write(devname)
    local profiles = mtkwifi.search_dev_and_profile()
    debug_write(profiles[devname])
    assert(profiles[devname])

    local cfgs = mtkwifi.load_profile(profiles[devname])

    for k,v in pairs(http.formvalue()) do
        if type(v) ~= type("") and type(v) ~= type(0) then
            nixio.syslog("err", "apcli_cfg, invalid value type for "..k..","..type(v))
        elseif string.byte(k) ~= string.byte("_") then
            cfgs[k] = v or ""
        end
    end

    -- http.write_json(http.formvalue())

    -- Mediatek Adaptive Network
    --[=[ moved to a separated page
    if cfgs.ApCliEzEnable then
        cfgs.EzEnable = cfgs.ApCliEzEnable
        cfgs.ApMWDS = cfgs.ApCliMWDS
        cfgs.EzConfStatus = cfgs.ApCliEzConfStatus
        cfgs.EzOpenGroupID = cfgs.ApCliEzOpenGroupID
        if http.formvalue("__group_id_mode") == "0" then
            cfgs.EzGroupID = cfgs.ApCliEzGroupID
            cfgs.EzGenGroupID = ""
            cfgs.ApCliEzGenGroupID = ""
        else
            cfgs.EzGroupID = ""
            cfgs.ApCliEzGroupID = ""
            cfgs.EzGenGroupID = cfgs.ApCliEzGenGroupID
        end
        -- if dbdc
        -- os.execute("app_ez &")
        -- os.execute("ManDaemon ")
    end
    ]=]
    __mtkwifi_save_profile(cfgs, profiles[devname], false)

    -- M.A.N Push parameters
    -- They are not part of wifi profile, we save it into /etc/man.conf.

    --[=[ moved to a separated page
    local man_ssid = http.formvalue("__man_ssid_"..vifname)
    local man_pass = http.formvalue("__man_pass_"..vifname)
    local man_auth = http.formvalue("__man_auth_"..vifname) or ""

    if man_ssid and man_pass then
        local fp = io.open("/etc/man."..vifname..".conf", "w+")
        fp:write("__man_ssid_"..vifname.."="..man_ssid.."\n")
        fp:write("__man_pass_"..vifname.."="..man_pass.."\n")
        fp:write("__man_auth_"..vifname.."="..man_auth.."\n")
        fp:close()
    end
    ]=]

    -- commented, do not connect by default
    --[=[
    os.execute("iwpriv apcli0 set ApCliEnable=0")
    os.execute("iwpriv apcli0 set Channel="..cfgs.Channel)
    os.execute("iwpriv apcli0 set ApCliAuthMode="..cfgs.ApCliAuthMode)
    os.execute("iwpriv apcli0 set ApCliEncrypType="..cfgs.ApCliEncrypType)
    if cfgs.ApCliAuthMode == "WEP" then
        os.execute("#iwpriv apcli0 set ApCliDefaultKeyID="..cfgs.ApCliDefaultKeyID)
        os.execute("#iwpriv apcli0 set ApCliKey1="..cfgs.ApCliKey1Str)
    elseif cfgs.ApCliAuthMode == "WPAPSK"
        or cfgs.ApCliAuthMode == "WPA2PSK"
        or cfgs.ApCliAuthMode == "WPAPSKWPA2PSK" then
        os.execute("iwpriv apcli0 set ApCliWPAPSK="..cfgs.ApCliWPAPSK)
    end
    -- os.execute("iwpriv apcli0 set ApCliWirelessMode=")
    os.execute("iwpriv apcli0 set ApCliSsid="..cfgs.ApCliSsid)
    os.execute("iwpriv apcli0 set ApCliEnable=1")
    ]=]
    if http.formvalue("__apply") then
        mtkwifi.__run_in_child_env(__mtkwifi_reload, devname)
        local url_to_visit_after_reload = luci.dispatcher.build_url("admin", "network", "wifi", "apcli_cfg_view", dev, vif)
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "loading",url_to_visit_after_reload))
    else
        luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "apcli_cfg_view", dev, vif))
    end
end

function apcli_connect(dev, vif)
    -- dev_vif can be
    --  1. mt7620.apcli0         # simple case
    --  2. mt7615e.1.apclix0     # multi-card
    --  3. mt7615e.1.2G.apclix0  # multi-card & multi-profile
    local devname,vifname = dev, vif
    debug_write("devname=", dev, "vifname=", vif)
    local profiles = mtkwifi.search_dev_and_profile()
    debug_write(profiles[devname])
    assert(profiles[devname])
    local cfgs = mtkwifi.load_profile(profiles[devname])
    cfgs.ApCliEnable = "1"
    __mtkwifi_save_profile(cfgs, profiles[devname], true)
    os.execute("ifconfig "..vifname.." up")
    --os.execute("brctl addif br-lan "..vifname)
    os.execute("iwpriv "..vifname.." set MACRepeaterEn="..cfgs.MACRepeaterEn)
    os.execute("iwpriv "..vifname.." set ApCliEnable=0")
    os.execute("iwpriv "..vifname.." set Channel="..cfgs.Channel)
    os.execute("iwpriv "..vifname.." set ApCliAuthMode="..cfgs.ApCliAuthMode)
    os.execute("iwpriv "..vifname.." set ApCliEncrypType="..cfgs.ApCliEncrypType)
    if cfgs.ApCliEncrypType == "WEP" then
        os.execute("iwpriv "..vifname.." set ApCliDefaultKeyID="..cfgs.ApCliDefaultKeyID)
        if (cfgs.ApCliDefaultKeyID == "1") then
            os.execute("iwpriv "..vifname.." set ApCliKey1=\""..mtkwifi.__handleSpecialChars(cfgs.ApCliKey1Str).."\"")
        elseif (cfgs.ApCliDefaultKeyID == "2") then
            os.execute("iwpriv "..vifname.." set ApCliKey2=\""..mtkwifi.__handleSpecialChars(cfgs.ApCliKey2Str).."\"")
        elseif (cfgs.ApCliDefaultKeyID == "3") then
            os.execute("iwpriv "..vifname.." set ApCliKey3=\""..mtkwifi.__handleSpecialChars(cfgs.ApCliKey3Str).."\"")
        elseif (cfgs.ApCliDefaultKeyID == "4") then
            os.execute("iwpriv "..vifname.." set ApCliKey4=\""..mtkwifi.__handleSpecialChars(cfgs.ApCliKey4Str).."\"")
        end
    elseif cfgs.ApCliAuthMode == "WPAPSK"
        or cfgs.ApCliAuthMode == "WPA2PSK"
        or cfgs.ApCliAuthMode == "WPAPSKWPA2PSK" then
        os.execute("iwpriv "..vifname.." set ApCliWPAPSK=\""..mtkwifi.__handleSpecialChars(cfgs.ApCliWPAPSK).."\"")
    end
    os.execute("iwpriv "..vifname.." set ApCliSsid=\""..mtkwifi.__handleSpecialChars(cfgs.ApCliSsid).."\"")
    os.execute("iwpriv "..vifname.." set ApCliEnable=1")
    os.execute("iwpriv "..vifname.." set ApCliAutoConnect=3")

    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function apcli_disconnect(dev, vif)
    -- dev_vif can be
    --  1. mt7620.apcli0         # simple case
    --  2. mt7615e.1.apclix0     # multi-card
    --  3. mt7615e.1.2G.apclix0  # multi-card & multi-profile
    local devname,vifname = dev, vif
    debug_write("devname=", dev, "vifname", vif)
    debug_write(devname)
    debug_write(vifname)
    local profiles = mtkwifi.search_dev_and_profile()
    debug_write(profiles[devname])
    assert(profiles[devname])
    local cfgs = mtkwifi.load_profile(profiles[devname])
    cfgs.ApCliEnable = "1"
    __mtkwifi_save_profile(cfgs, profiles[devname], true)
    os.execute("iwpriv "..vifname.." set ApCliEnable=0")
    os.execute("ifconfig "..vifname.." down")
    --os.execute("brctl delif br-lan "..vifname)
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi"))
end

function apply_power_boost_settings()
    local devname = http.formvalue("__devname")
    local ret_status = {}
    local devs = mtkwifi.get_all_devs()
    local dev = {}
    for _,v in ipairs(devs) do
        if v.devname == devname then
            dev = v
            break
        end
    end
    if next(dev) == nil then
        ret_status["status"]= "Device "..(devname or "").." not found!"
    elseif not dev.isPowerBoostSupported then
        ret_status["status"]= "Power Boost feature is not supported by "..(devname or "").." Device!"
    else
        local cfgs = mtkwifi.load_profile(dev.profile)
        if type(cfgs) ~= "table" or next(cfgs) == nil then
            ret_status["status"]= "Profile settings file not found!"
        else
            for k,v in pairs(http.formvalue()) do
                if type(v) ~= type("") and type(v) ~= type(0) then
                    debug_write("ERROR: [apply_power_boost_settings] String expected; Got"..type(v).."for"..k.."key")
                    ret_status["status"]= "Power Boost settings are of incorrect type!"
                    break
                elseif string.byte(k) ~= string.byte("_") then
                    cfgs[k] = v or ""
                end
            end
            if next(ret_status) == nil then
                if type(dev.vifs) ~= "table" or next(dev.vifs) == nil or not cfgs.BssidNum or cfgs.BssidNum == "0" then
                    ret_status["status"]= "No Wireless Interfaces has been added yet!"
                elseif cfgs.PowerUpenable ~= "1" then
                    ret_status["status"]= "Power Boost feature is not enabled!"
                else
                    local up_vif_name_list = {}
                    for idx,vif in ipairs(dev.vifs) do
                        if vif.state == "up" and vif.vifname ~= nil and vif.vifname ~= "" and type(vif.vifname) == "string" then
                            up_vif_name_list[idx] = vif.vifname
                        end
                    end
                    if next(up_vif_name_list) == nil then
                        ret_status["status"]= "No Wireless Interfaces is up!"
                    else
                        for _,vifname in ipairs(up_vif_name_list) do
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=0:"..cfgs.PowerUpCckOfdm)
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=1:"..cfgs.PowerUpHT20)
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=2:"..cfgs.PowerUpHT40)
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=3:"..cfgs.PowerUpVHT20)
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=4:"..cfgs.PowerUpVHT40)
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=5:"..cfgs.PowerUpVHT80)
                            os.execute("iwpriv "..vifname.." set TxPowerBoostCtrl=6:"..cfgs.PowerUpVHT160)
                            os.execute("sleep 1") -- Wait for 1 second to let driver process the above data
                        end
                        __mtkwifi_save_profile(cfgs, dev.profile, true)
                        ret_status["status"]= "SUCCESS"
                    end
                end
            end
        end
    end
    http.write_json(ret_status)
end

function get_bssid_num(devName)
    local ret_status = {}
    local profiles = mtkwifi.search_dev_and_profile()
    for dev,profile in pairs(profiles) do
        if devName == dev then
            local cfgs = mtkwifi.load_profile(profile)
            if type(cfgs) ~= "table" or next(cfgs) == nil then
                ret_status["status"]= "Profile settings file not found!"
            else
                ret_status["status"] = "SUCCESS"
                ret_status["bssidNum"] = cfgs.BssidNum
            end
            break
        end
    end
    if next(ret_status) == nil then
        ret_status["status"]= "Device "..(devName or "").." not found!"
    end
    http.write_json(ret_status)
end

local exec_reset_to_defaults_cmd = function (devname)
    if devname then
        os.execute("wifi reset "..devname)
    else
        os.execute("wifi reset")
    end
end

function reset_to_defaults(devname)
    mtkwifi.__run_in_child_env(exec_reset_to_defaults_cmd, devname)
    luci.http.redirect(luci.dispatcher.build_url("admin", "network", "wifi", "loading",mtkwifi.get_referer_url()))
end

