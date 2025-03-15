#!/usr/bin/lua
--[[
 *
 * Copyright (C) 2023 hanwckf <hanwckf@vip.qq.com>
 *
 * 	This program is free software; you can redistribute it and/or modify
 * 	it under the terms of the GNU General Public License as published by
 * 	The Free Software Foundation; either version 2 of the License, or
 * 	(at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
]]

local mtwifi_defs = {}

mtwifi_defs.dbdc_init_ifname = "ra0"
mtwifi_defs.max_mbssid = 16
mtwifi_defs.max_acl_entry = 129

mtwifi_defs.vif_cfgs = {
    -- dat cfg = default val
    ["AuthMode"] = "OPEN",
    ["EncrypType"] = "NONE",
    ["PMFMFPC"] = "0",
    ["PMFMFPR"] = "0",
    ["PMFSHA256"] = "0",
    ["RekeyInterval"] = "3600",
    ["DefaultKeyID"] = "1",
    ["IEEE8021X"] = "0",
    ["Key1Type"] = "0",
    ["Key2Type"] = "0",
    ["Key3Type"] = "0",
    ["Key4Type"] = "0",
    ["PMKCachePeriod"] = "10",
    ["PreAuth"] = "0",
    ["RADIUS_Port"] = "1812",
    ["RADIUS_Server"] = "0",
    ["RekeyMethod"] = "DISABLE",
    ["session_timeout_interval"] = "0",
    ["Wapiifname"] = "",
    ["HideSSID"] = "0",
    ["WirelessMode"] = "",
    ["NoForwarding"] = "0",
    ["APSDCapable"] = "1",
    ["WmmCapable"] = "1",
    ["FragThreshold"] = "2346",
    ["RTSThreshold"] = "2347",
    ["HT_AMSDU"] = "1",
    ["HT_AutoBA"] = "1",
    ["HT_GI"] = "1",
    ["HT_LDPC"] = "1",
    ["HT_OpMode"] = "0",
    ["HT_PROTECT"] = "1",
    ["HT_STBC"] = "1",
    ["IgmpSnEnable"] = "0",
    ["RRMEnable"] = "1",
    ["FtSupport"] = "0",
    ["VHT_BW_SIGNAL"] = "0",
    ["VHT_LDPC"] = "1",
    ["VHT_SGI"] = "1",
    ["VHT_STBC"] = "1",
    ["MuMimoDlEnable"] = "1",
    ["MuMimoUlEnable"] = "1",
    ["MuOfdmaDlEnable"] = "1",
    ["MuOfdmaUlEnable"] = "1",
    ["DLSCapable"] = "0",
    ["WdsEnable"] = "0",
    ["WscConfMode"] = "0",
    ["TxRate"] = "0",
}

mtwifi_defs.vif_cfgs_idx = {
    ["WPAPSK"] = "12345678",
    ["SSID"] = "",
    ["RADIUS_Key"] = "",
}

mtwifi_defs.vif_acl = {
    ["AccessPolicy"] = "0",
    ["AccessControlList"] = "",
}

mtwifi_defs.chip_cfgs = {
    -- uci config = dat config, default value
    ["beacon_int"] = { "BeaconPeriod" , "100"},
    ["dtim_period"] = { "DtimPeriod", "1"},
    ["whnat"] = { "WHNAT", "1"},
    ["bandsteering"] = { "BandSteering", "0"},
}

mtwifi_defs.reinstall_cfgs = {
    "BssidNum", "WHNAT", "E2pAccessMode",
    "HT_RxStream", "HT_TxStream", "WdsEnable"
}

mtwifi_defs.iwpriv_ap_cfgs = {
    -- uci config = iwpriv set cmd , default value
    ["kicklow"] = {"KickStaRssiLow", "0"},
    ["assocthres"] = {"AssocReqRssiThres" , "0"},
    ["steeringthresold"] = {"Steeringthresold" , "0"},
}

mtwifi_defs.enc2dat = {
    -- enc = AuthMode, EncrypType
    ["none"] = {"OPEN", "NONE"},
    ["sae"] = {"WPA3PSK", "AES"},
    ["sae-mixed"] = {"WPA2PSKWPA3PSK", "AES"},
    ["psk2+tkip+ccmp"] = {"WPA2PSK", "TKIPAES"},
    ["psk2+tkip+aes"] = {"WPA2PSK", "TKIPAES"},
    ["psk2+tkip"] = {"WPA2PSK", "AES"},
    ["psk2+ccmp"] = {"WPA2PSK", "AES"},
    ["psk2+aes"] = {"WPA2PSK", "AES"},
    ["psk2"] = {"WPA2PSK", "AES"},
    ["psk+tkip+ccmp"] = {"WPAPSK", "AES"},
    ["psk+tkip+aes"] = {"WPAPSK", "AES"},
    ["psk+tkip"] = {"WPAPSK", "TKIP"},
    ["psk+ccmp"] = {"WPAPSK", "AES"},
    ["psk+aes"] = {"WPAPSK", "AES"},
    ["psk"] = {"WPAPSK", "AES"},
    ["psk-mixed+tkip+ccmp"] = {"WPAPSKWPA2PSK", "TKIPAES"},
    ["psk-mixed+tkip+aes"] = {"WPAPSKWPA2PSK", "TKIPAES"},
    ["psk-mixed+tkip"] = {"WPAPSKWPA2PSK", "TKIP"},
    ["psk-mixed+ccmp"] = {"WPAPSKWPA2PSK", "AES"},
    ["psk-mixed+aes"] = {"WPAPSKWPA2PSK", "AES"},
    ["psk-mixed"] = {"WPAPSKWPA2PSK", "AES"},
    ["owe"] = {"OWE", "AES"},
}

mtwifi_defs.countryRegions = {
    -- CountryCode = 2g region, 5g region
    ["DB"] = { 5, 13 },
    ["AE"] = { 1, 0 },
    ["AL"] = { 1, 0 },
    ["AR"] = { 1, 3 },
    ["AT"] = { 1, 1 },
    ["AM"] = { 1, 2 },
    ["AU"] = { 1, 0 },
    ["AZ"] = { 1, 2 },
    ["BE"] = { 1, 1 },
    ["BH"] = { 1, 0 },
    ["BY"] = { 1, 0 },
    ["BO"] = { 1, 4 },
    ["BR"] = { 1, 1 },
    ["BN"] = { 1, 4 },
    ["BG"] = { 1, 1 },
    ["BZ"] = { 1, 4 },
    ["CA"] = { 0, 0 },
    ["CH"] = { 1, 1 },
    ["CL"] = { 1, 0 },
    ["CN"] = { 1, 0 },
    ["CO"] = { 0, 0 },
    ["CR"] = { 1, 0 },
    ["CY"] = { 1, 1 },
    ["CZ"] = { 1, 2 },
    ["DE"] = { 1, 1 },
    ["DK"] = { 1, 1 },
    ["DO"] = { 0, 0 },
    ["DZ"] = { 1, 0 },
    ["EC"] = { 1, 0 },
    ["EG"] = { 1, 2 },
    ["EE"] = { 1, 1 },
    ["ES"] = { 1, 1 },
    ["FI"] = { 1, 1 },
    ["FR"] = { 1, 2 },
    ["GE"] = { 1, 2 },
    ["GB"] = { 1, 1 },
    ["GR"] = { 1, 1 },
    ["GT"] = { 0, 0 },
    ["HN"] = { 1, 0 },
    ["HK"] = { 1, 0 },
    ["HU"] = { 1, 1 },
    ["HR"] = { 1, 2 },
    ["IS"] = { 1, 1 },
    ["IN"] = { 1, 0 },
    ["ID"] = { 1, 4 },
    ["IR"] = { 1, 4 },
    ["IE"] = { 1, 1 },
    ["IL"] = { 1, 0 },
    ["IT"] = { 1, 1 },
    ["JP"] = { 1, 9 },
    ["JO"] = { 1, 0 },
    ["KP"] = { 1, 5 },
    ["KR"] = { 1, 5 },
    ["KW"] = { 1, 0 },
    ["KZ"] = { 1, 0 },
    ["LB"] = { 1, 0 },
    ["LI"] = { 1, 1 },
    ["LT"] = { 1, 1 },
    ["LU"] = { 1, 1 },
    ["LV"] = { 1, 1 },
    ["MA"] = { 1, 0 },
    ["MC"] = { 1, 2 },
    ["MO"] = { 1, 0 },
    ["MK"] = { 1, 0 },
    ["MX"] = { 0, 0 },
    ["MY"] = { 1, 0 },
    ["NL"] = { 1, 1 },
    ["NO"] = { 0, 0 },
    ["NZ"] = { 1, 0 },
    ["OM"] = { 1, 0 },
    ["PA"] = { 0, 0 },
    ["PE"] = { 1, 4 },
    ["PH"] = { 1, 4 },
    ["PL"] = { 1, 1 },
    ["PK"] = { 1, 0 },
    ["PT"] = { 1, 1 },
    ["PR"] = { 0, 0 },
    ["QA"] = { 1, 0 },
    ["RO"] = { 1, 0 },
    ["RU"] = { 1, 0 },
    ["SA"] = { 1, 0 },
    ["SG"] = { 1, 0 },
    ["SK"] = { 1, 1 },
    ["SI"] = { 1, 1 },
    ["SV"] = { 1, 0 },
    ["SE"] = { 1, 1 },
    ["SY"] = { 1, 0 },
    ["TH"] = { 1, 0 },
    ["TN"] = { 1, 2 },
    ["TR"] = { 1, 2 },
    ["TT"] = { 1, 2 },
    ["TW"] = { 0, 3 },
    ["UA"] = { 1, 0 },
    ["US"] = { 0, 0 },
    ["UY"] = { 1, 5 },
    ["UZ"] = { 0, 1 },
    ["VE"] = { 1, 5 },
    ["VN"] = { 1, 0 },
    ["YE"] = { 1, 0 },
    ["ZA"] = { 1, 1 },
    ["ZW"] = { 1, 0 },    
}

return mtwifi_defs
