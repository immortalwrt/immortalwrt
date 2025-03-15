
function debug_info_write(devname,content)
    local filename = "/tmp/mtk/wifi/"..devname.."_quick_setting_cmd.sh"
    local ff = io.open(filename, "a")
    ff:write(content)
    ff:write("\n")
    ff:close()
end

function token(str, n, default)
    local i = 1
    local list = {}
    for k in string.gmatch(str, "([^;]+)") do
        list[i] = k
        i = i + 1
    end
    return list[tonumber(n)] or default
end

function GetFileSize( filename )
    local fp = io.open( filename )
    if fp == nil then
    return nil
    end
    local filesize = fp:seek( "end" )
    fp:close()
    return filesize
end

function vifs_cfg_parm(parm)
    local vifs_cfg_parms = {"AuthMode", "EncrypType", "Key", "WPAPSK", "Access", "^WPS", "^wps", "^Wsc", "PIN", "^WEP", ";", "_"}
    for _, pat in ipairs(vifs_cfg_parms) do
        if string.find(parm, pat) then
            return false
        end
    end
    return true
end

function __set_wifi_apcli_security(cfgs, diff, device, devname)
    -- to keep it simple, we always reconf the security if anything related is changed.
    -- do optimization only if there's significant performance defect.
    --if not diff[ApCliEnable][2] == "1" and cfgs[ApCliEnable] ~= "1" then return end
    local vifs = {} -- changed vifs

    -- figure out which vif is changed
    -- since multi-bssid is possible, both AuthMode and EncrypType can be a group
    local auth_old = cfgs.ApCliAuthMode:split() or {}
    local encr_old = cfgs.ApCliEncrypType and cfgs.ApCliEncrypType:split() or {}
    local keyid_old = cfgs.ApCliDefaultKeyID:split() or {}
    local auth_old_i = (auth_old[1] or ''):split(";")
    local encr_old_i = (encr_old[1] or ''):split(";")
    local keyid_old_i = (keyid_old[1] or ''):split(";")
    local auth_new = diff.ApCliAuthMode and diff.ApCliAuthMode[2]:split() or auth_old
    local auth_new_i = (auth_new[1] or ''):split(";") 
    local encr_new = diff.ApCliEncrypType and diff.ApCliEncrypType[2]:split() or encr_old
    local encr_new_i = (encr_new[1] or ''):split(";") 
    local keyid_new = diff.ApCliDefaultKeyID and diff.ApCliDefaultKeyID[2]:split() or keyid_old
    local keyid_new_i = (keyid_new[1] or ''):split(";") 


    --print("encry ="..encr_new[1],auth_new[1], keyid_new[1],keyid_new_i[1])
    --print("encry_old ="..encr_old[1],auth_old[1], keyid_old[1])
    local num = math.max(#encr_old_i, #encr_new_i)
    for i = 1, num do
        local changed = false
        if next(auth_new) and auth_old_i[i] ~= auth_new_i[i] then
            changed = true
        elseif next(encr_new) and encr_old_i[i] ~= encr_new_i[i] then
            changed = true
        elseif next(keyid_new) and keyid_old_i[i] ~= keyid_new_i[i] then
            changed = true
        elseif diff["ApCliWPAPSK"] then
            changed = true
        else
            -- just support apcli0/apclii0/apclix0
            for j = 1, 4 do
                if diff["ApCliKey"..tostring(j).."Str"] then
                    changed = true
                    break
                end
            end
        end

        if changed then
            local vif = {}
            vif.idx = i
            vif.vifname = device.apcli_ifname..tostring(i-1)
            vif.AuthMode = auth_new_i and auth_new_i[i] or auth_old_i[i]
            vif.EncrypType = encr_new_i and encr_new_i[i] or encr_old_i[i]
            vif.KeyID = keyid_new_i and keyid_new_i[i] or keyid_old_i[i]
            vif.DefaultKeyID_idx = "ApCliKey"..tostring(vif.KeyID)
            vif.DefaultKey = diff["ApCliKey"..tostring(vif.KeyID).."Str"] 
                        and diff["ApCliKey"..tostring(vif.KeyID).."Str"][2] or cfgs["ApCliKey"..tostring(vif.KeyID).."Str"]
            --vif.WEPType = "WEP"..tostring(vif.KeyID).."Type"
            --vif.WEPTypeVal = diff["WEP"..tostring(vif.KeyID).."Type"..tostring(i)] and
                        --diff["WEP"..tostring(vif.KeyID).."Type"..tostring(i)][2] or cfgs["WEP"..tostring(vif.KeyID).."Type"..tostring(i)]
            vif.WPAPSK = diff["ApCliWPAPSK"] and diff["ApCliWPAPSK"][2] or cfgs["ApCliWPAPSK"]
            vif.SSID = diff["ApCliSsid"] and diff["ApCliSsid"][2] or cfgs["ApCliSsid"]
            table.insert(vifs, vif)
        end
    end

    -- iwpriv here
    for i, vif in ipairs(vifs) do
        if vif.AuthMode == "OPEN" then
            if vif.EncrypType == "WEP" then
                commands = string.format([[
                iwpriv %s set ApCliAuthMode=OPEN;
                iwpriv %s set ApCliEncrypType=WEP;
                iwpriv %s set %s="%s";
                iwpriv %s set ApCliDefaultKeyID=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname,  vif.DefaultKeyID_idx,
            vif.DefaultKey, vif.vifname, vif.KeyID, vif.vifname)
            else
                commands = string.format([[
                iwpriv %s set ApCliAuthMode=OPEN;
                iwpriv %s set ApCliEncrypType=NONE;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname)
            end
        elseif vif.AuthMode == "WEPAUTO" and  vif.EncrypType == "WEP" then
                commands = string.format([[
                iwpriv %s set ApCliAuthMode=OPEN;
                iwpriv %s set ApCliEncrypType=WEP;
                iwpriv %s set %s="%s";
                iwpriv %s set ApCliDefaultKeyID=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname,  vif.DefaultKeyID_idx,
            vif.DefaultKey, vif.vifname, vif.KeyID, vif.vifname)
        elseif vif.AuthMode == "OWE" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=OWE;
                iwpriv %s set ApCliEncrypType=AES;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname)
        elseif vif.AuthMode == "SHARED" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=SHARED;
                iwpriv %s set ApCliEncrypType=WEP;
                iwpriv %s set %s="%s";
                iwpriv %s set ApCliDefaultKeyID=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname,  vif.DefaultKeyID_idx,
            vif.DefaultKey, vif.vifname, vif.KeyID, vif.vifname)
        elseif vif.AuthMode == "WPA2PSK" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=WPA2PSK;
                iwpriv %s set ApCliEncrypType=%s;
                iwpriv %s set ApCliWPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
         elseif vif.AuthMode == "WPA3PSK" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=WPA3PSK;
                iwpriv %s set ApCliEncrypType=%s;
                iwpriv %s set ApCliWPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPAPSKWPA2PSK" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=WPAPSKWPA2PSK;
                iwpriv %s set ApCliEncrypType=%s;
                iwpriv %s set ApCliWPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPA2PSKWPA3PSK" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=WPA2PSKWPA3PSK;
                iwpriv %s set ApCliEncrypType=%s;
                iwpriv %s set ApCliRekeyMethod=TIME;
                iwpriv %s set ApCliWPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPAPSK" then
            commands = string.format([[
                iwpriv %s set ApCliAuthMode=WPAPSK;
                iwpriv %s set ApCliEncrypType=%s;
                iwpriv %s set ApCliWPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPA1WPA2" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA1WPA2;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.RADIUS_Server,
            vif.vifname,vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
        else
            error(string.format("invalid AuthMode \"%s\"", vif.AuthMode))
        end

        -- must append extra SSID command to make changes take effect
        commands = commands .."\n".. string.format([[
                iwpriv %s set ApCliSSID="%s";]], vif.vifname, vif.SSID)
        debug_info_write(devname, commands)
    end
end

function __set_wifi_security(cfgs, diff, device, devname)
    -- to keep it simple, we always reconf the security if anything related is changed.
    -- do optimization only if there's significant performance defect.

    local vifs = {} -- changed vifs

    -- figure out which vif is changed
    -- since multi-bssid is possible, both AuthMode and EncrypType can be a group
    local auth_old = cfgs.AuthMode:split()
    local encr_old = cfgs.EncrypType:split()
    local IEEE8021X_old = cfgs.IEEE8021X:split()
    local keyid_old = cfgs.DefaultKeyID:split()
    local auth_new = {}
    local auth_new1 = {}
    local encr_new = {}
    local encr_new1 = {}
    local IEEE8021X_new ={}
    local IEEE8021X_new1 ={}
    local keyid_new = {}
    local keyid_new1 = {}

    if diff.EncrypType then
        encr_new = diff.EncrypType[2]:split()
        encr_new1 = encr_new[1]:split(";")
    end
    if diff.AuthMode then
        auth_new = diff.AuthMode[2]:split()
        auth_new1 = auth_new[1]:split(";")
    end
    if diff.IEEE8021X then
        IEEE8021X_new = diff.IEEE8021X[2]:split()
        IEEE8021X_new1 = IEEE8021X_new[1]:split(";")
    end
    if diff.DefaultKeyID then
        keyid_new = diff.DefaultKeyID[2]:split()
        keyid_new1 = keyid_new[1]:split(";")
    end

    -- For WPA/WPA2
    local RadiusS_old = cfgs.RADIUS_Server:split() or {}
    local RadiusP_old = cfgs.RADIUS_Port:split() or {}
    local RadiusS_old_i = (RadiusS_old[1] or ''):split(";")
    local RadiusP_old_i = (RadiusP_old[1] or ''):split(";")
    local RadiusS_new = diff.RADIUS_Server and diff.RADIUS_Server[2]:split() or RadiusS_old
    local RadiusP_new = diff.RADIUS_Port and diff.RADIUS_Port[2]:split() or RadiusP_old
    local RadiusS_new_i = (RadiusS_new[1] or ''):split(";") --split by ";"
    local RadiusP_new_i = (RadiusP_new[1] or ''):split(";")

    local auth_old1 = auth_old[1]:split(";") --auth_old1[1]=OPEN,auth_old1[2]=WPA2PSK
    local encr_old1 = encr_old[1]:split(";")
    local IEEE8021X_old1 =IEEE8021X_old[1]:split(";")
    local keyid_old1 =keyid_old[1]:split(";")
   
    for i = 1, #encr_old1 do
        local changed = false
        if next(auth_new) and auth_old1[i] ~= auth_new1[i] then
            changed = true
        elseif next(encr_new) and encr_old1[i] ~= encr_new1[i] then
            changed = true
        elseif next(IEEE8021X_new) and IEEE8021X_old1[i] ~= IEEE8021X_new1[i] then
            changed = true
        elseif next(keyid_new) and keyid_old1[i] ~= keyid_new1[i] then
            changed = true
        elseif diff["WPAPSK"..tostring(i)] then
            changed = true
        elseif next(RadiusS_new) and RadiusS_old_i[i] ~= RadiusS_new_i[i] then
            changed = true
        elseif next(RadiusP_new) and RadiusP_old_i[i] ~= RadiusP_new_i[i] then
            changed = true
        elseif diff["RADIUS_Key"..tostring(i)] then
            changed = true
        else
            for j = 1, 4 do
                if diff["Key"..tostring(j).."Str"..tostring(i)] then
                    changed = true
                    break
                end
            end
        end

        if changed then
            local vif = {}
            vif.idx = i
            vif.vifname = device.ext_ifname..tostring(i-1)
            vif.AuthMode = auth_new1 and auth_new1[i] or auth_old1[i]
            vif.EncrypType = encr_new1 and encr_new1[i] or encr_old1[i]
            vif.KeyID = keyid_new1 and keyid_new1[i] or keyid_old1[i]
            vif.DefaultKeyID_idx = "Key"..tostring(vif.KeyID)
            vif.DefaultKey = diff["Key"..tostring(vif.KeyID).."Str"..tostring(i)] and
                        diff["Key"..tostring(vif.KeyID).."Str"..tostring(i)][2] or cfgs["Key"..tostring(vif.KeyID).."Str"..tostring(i)]
            vif.WEPType = "WEP"..tostring(vif.KeyID).."Type"
            vif.WEPTypeVal = diff["WEP"..tostring(vif.KeyID).."Type"..tostring(i)] and
                        diff["WEP"..tostring(vif.KeyID).."Type"..tostring(i)][2] or cfgs["WEP"..tostring(vif.KeyID).."Type"..tostring(i)]
            vif.WPAPSK = diff["WPAPSK"..tostring(i)] and diff["WPAPSK"..tostring(i)][2] or cfgs["WPAPSK"..tostring(i)]
            vif.SSID = diff["SSID"..tostring(i)] and diff["SSID"..tostring(i)][2] or cfgs["SSID"..tostring(i)]
            vif.IEEE8021X = IEEE8021X_new1 and IEEE8021X_new1[i] or IEEE8021X_old1[i]
            vif.RADIUS_Server = RadiusS_new_i and RadiusS_new_i[i] or RadiusS_old_i[i]
            vif.RADIUS_Port = RadiusP_new_i and RadiusP_new_i[i] or RadiusP_old_i[i]
            vif.RADIUS_Key = diff["RADIUS_Key"..tostring(i)] and diff["RADIUS_Key"..tostring(i)][2] or cfgs["RADIUS_Key"..tostring(i)]
            table.insert(vifs, vif)
        end
    end

    -- iwpriv here
    for i, vif in ipairs(vifs) do
        if vif.AuthMode == "OPEN" then
            if vif.EncrypType == "WEP" then
                commands = string.format([[
                iwpriv %s set AuthMode=OPEN;
                iwpriv %s set EncrypType=WEP;
                iwpriv %s set %s="%s";
                iwpriv %s set DefaultKeyID=%s;
                iwpriv %s set %s=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname, vif.DefaultKeyID_idx, vif.DefaultKey,
            vif.vifname, vif.KeyID, vif.vifname, vif.WEPType,vif.WEPTypeVal, vif.vifname)
            elseif vif.EncrypType == "NONE" and vif.IEEE8021X == "1" then
            commands = string.format([[
                iwpriv %s set AuthMode=OPEN;
                iwpriv %s set EncrypType=NONE;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=1;]],
            vif.vifname, vif.vifname, vif.vifname, vif.RADIUS_Server,
            vif.vifname, vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
            else
                commands = string.format([[
                iwpriv %s set AuthMode=OPEN;
                iwpriv %s set EncrypType=NONE;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname)
            end
        elseif vif.AuthMode == "WEPAUTO" and vif.EncrypType == "WEP" then
                commands = string.format([[
                iwpriv %s set AuthMode=WEPAUTO;
                iwpriv %s set EncrypType=WEP;
                iwpriv %s set %s="%s";
                iwpriv %s set DefaultKeyID=%s;
                iwpriv %s set %s=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname, vif.DefaultKeyID_idx, vif.DefaultKey,
            vif.vifname, vif.KeyID, vif.vifname, vif.WEPType, vif.WEPTypeVal, vif.vifname)
        elseif vif.AuthMode == "OWE" then
            commands = string.format([[
                iwpriv %s set AuthMode=OWE;
                iwpriv %s set EncrypType=AES;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname)
        elseif vif.AuthMode == "SHARED" then
            commands = string.format([[
                iwpriv %s set AuthMode=SHARED;
                iwpriv %s set EncrypType=WEP;
                iwpriv %s set %s="%s";
                iwpriv %s set DefaultKeyID=%s;
                iwpriv %s set %s=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname, vif.DefaultKeyID_idx, vif.DefaultKey,
            vif.vifname, vif.KeyID, vif.vifname, vif.WEPType,vif.WEPTypeVal, vif.vifname)
        elseif vif.AuthMode == "WPA2PSK" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA2PSK;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set WPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
         elseif vif.AuthMode == "WPA3PSK" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA3PSK;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set WPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPAPSKWPA2PSK" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPAPSKWPA2PSK;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set WPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPA2PSKWPA3PSK" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA2PSKWPA3PSK;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set RekeyMethod=TIME;
                iwpriv %s set WPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPAPSK" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPAPSK;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set WPAPSK="%s";]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.WPAPSK)
        elseif vif.AuthMode == "WPA" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.RADIUS_Server,
            vif.vifname, vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
        elseif vif.AuthMode == "WPA2" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA2;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.RADIUS_Server,
            vif.vifname,vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
        elseif vif.AuthMode == "WPA3" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA3;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.RADIUS_Server,
            vif.vifname,vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
        elseif vif.AuthMode == "WPA1WPA2" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA1WPA2;
                iwpriv %s set EncrypType=%s;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.EncrypType, vif.vifname, vif.RADIUS_Server,
            vif.vifname,vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
        elseif vif.AuthMode == "WPA3-192" then
            commands = string.format([[
                iwpriv %s set AuthMode=WPA3-192;
                iwpriv %s set EncrypType=GCMP256;
                iwpriv %s set RADIUS_Server=%s;
                iwpriv %s set RADIUS_Port=%s;
                iwpriv %s set RADIUS_Key=%s;
                iwpriv %s set IEEE8021X=0;]],
            vif.vifname, vif.vifname, vif.vifname, vif.RADIUS_Server,
            vif.vifname,vif.RADIUS_Port, vif.vifname, vif.RADIUS_Key, vif.vifname)
        else
            error(string.format("invalid AuthMode \"%s\"", vif.AuthMode))
        end

        -- must append extra SSID command to make changes take effect
        commands = commands .."\n".. string.format([[
                iwpriv %s set SSID="%s";]], vif.vifname, vif.SSID)
        debug_info_write(devname, commands)
    end
end

function __set_he_mu(cfgs, diff, device, devname)
    local vifs = {} -- changed vifs

    local mu_of_maDl_old = cfgs.MuOfdmaDlEnable:split()
    local mu_of_maUl_old = cfgs.MuOfdmaUlEnable:split()
    local mu_mi_moDl_old = cfgs.MuMimoDlEnable:split()
    local mu_mi_moUl_old = cfgs.MuMimoUlEnable:split()
    local mu_of_maDl_new = {}
    local mu_of_maDl_new1 = {}
    local mu_of_maUl_new = {}
    local mu_of_maUl_new1 = {}
    local mu_mi_moDl_new = {}
    local mu_mi_moDl_new1 = {}
    local mu_mi_moUl_new = {}
    local mu_mi_moUl_new1 = {}

    if diff.MuOfdmaDlEnable then
        mu_of_maDl_new = diff.MuOfdmaDlEnable[2]:split()
        mu_of_maDl_new1 = mu_of_maDl_new[1]:split(";")
    end
    if diff.MuOfdmaUlEnable then
        mu_of_maUl_new = diff.MuOfdmaUlEnable[2]:split()
        mu_of_maUl_new1 = mu_of_maUl_new[1]:split(";")
    end
    if diff.MuMimoDlEnable then
        mu_mi_moDl_new = diff.MuMimoDlEnable[2]:split()
        mu_mi_moDl_new1 = mu_mi_moDl_new[1]:split(";")
    end
    if diff.MuMimoUlEnable then
        mu_mi_moUl_new = diff.MuMimoUlEnable[2]:split()
        mu_mi_moUl_new1 = mu_mi_moUl_new[1]:split(";")
    end

    local mu_of_maDl_old1 = mu_of_maDl_old[1]:split(";")
    local mu_of_maUl_old1 = mu_of_maUl_old[1]:split(";")
    local mu_mi_moDl_old1 = mu_mi_moDl_old[1]:split(";")
    local mu_mi_moUl_old1 = mu_mi_moUl_old[1]:split(";")

    for i = 1, #mu_of_maDl_old1 do
        local changed = false
        if next(mu_of_maDl_new) and mu_of_maDl_old1[i] ~= mu_of_maDl_new1[i] then
            changed = true
        elseif next(mu_of_maUl_new) and mu_of_maUl_old1[i] ~= mu_of_maUl_new1[i] then
            changed = true
        elseif next(mu_mi_moDl_new) and mu_mi_moDl_old1[i] ~= mu_mi_moDl_new1[i] then
            changed = true
        elseif next(mu_mi_moUl_new) and mu_mi_moUl_old1[i] ~= mu_mi_moUl_new1[i] then
            changed = true
        end

        if changed then
            local vif = {}
            vif.idx = i
            vif.vifname = device.ext_ifname..tostring(i-1)
            vif.MuOfdmaDlEnable = mu_of_maDl_new1 and mu_of_maDl_new1[i] or mu_of_maDl_old1[i]
            vif.MuOfdmaUlEnable = mu_of_maUl_new1 and mu_of_maUl_new1[i] or mu_of_maUl_old1[i]
            vif.MuMimoDlEnable = mu_mi_moDl_new1 and mu_mi_moDl_new1[i] or mu_mi_moDl_old1[i]
            vif.MuMimoUlEnable = mu_mi_moUl_new1 and mu_mi_moUl_new1[i] or mu_mi_moUl_old1[i]
            vif.SSID = diff["SSID"..tostring(i)] and diff["SSID"..tostring(i)][2] or cfgs["SSID"..tostring(i)]
            table.insert(vifs, vif)
        end

    end

    -- iwpriv here
    for i, vif in ipairs(vifs) do
        if vif.MuOfdmaDlEnable == "0" then
           commands1 = string.format([[
                iwpriv %s set muru_dl_enable=0;]],
            vif.vifname)
        elseif  vif.MuOfdmaDlEnable == "1" then
            commands1 = string.format([[
                iwpriv %s set muru_dl_enable=1;]],
            vif.vifname)
        else
            error(string.format("invalid MuOfdmaDlEnable \"%s\"", vif.MuOfdmaDlEnable))
        end

        if vif.MuOfdmaUlEnable == "0" then
           commands2 = string.format([[
                iwpriv %s set muru_ul_enable=0;]],
            vif.vifname)
        elseif vif.MuOfdmaUlEnable == "1" then
            commands2 = string.format([[
                iwpriv %s set muru_ul_enable=1;]],
            vif.vifname)
        else
            error(string.format("invalid MuOfdmaUlEnable \"%s\"", vif.MuOfdmaUlEnable))
        end

        if vif.MuMimoDlEnable == "0" then
           commands3 = string.format([[
                iwpriv %s set mu_dl_enable=0;]],
            vif.vifname)
        elseif vif.MuMimoDlEnable == "1" then
            commands3 = string.format([[
                iwpriv %s set mu_dl_enable=1;]],
            vif.vifname)
        else
            error(string.format("invalid MuMimoDlEnable \"%s\"", vif.MuMimoDlEnable))
        end

        if vif.MuMimoUlEnable == "0" then
            commands4 = string.format([[
                iwpriv %s set mu_ul_enable=0;]],
            vif.vifname)
        elseif vif.MuMimoUlEnable == "1" then
            commands4 = string.format([[
                iwpriv %s set mu_ul_enable=1;]],
            vif.vifname)
        else
            error(string.format("invalid MuMimoUlEnable \"%s\"", vif.MuMimoUlEnable))
        end

        commands = commands1.."\n"..commands2.."\n" ..commands3.."\n"..commands4.."\n".. string.format([[
                iwpriv %s set SSID="%s";]], vif.vifname, vif.SSID)
        debug_info_write(devname, commands)
    end
end

--dev cfg, key is dat parm, value is for iwpriv cmd.
function match_dev_parm(key)
    local dat_iw_table = {
                    CountryCode = "CountryCode",
                    CountryRegion = "CountryRegion",
                    CountryRegionABand = "CountryRegionABand",
                    BGProtection = "BGProtection",
                    ShortSlot = "ShortSlot",
                    PktAggregate = "PktAggregate",
                    HT_BADecline = "BADecline",
                    HT_DisallowTKIP = "HtDisallowTKIP",
                    TxPreamble = "TxPreamble",
                    TxBurst =  "TxBurst",
                    HT_MCS = "HtMcs",
                    HT_EXTCHA = "HtExtCha",
                    HT_MpduDensity = "HtMpduDensity",
                    HT_RDG = "HtRdg",
                    VOW_Airtime_Fairness_En = "vow_airtime_fairness_en",
                    HT_TxStream = "HtTxStream",
                    HT_RxStream = "HtRxStream",
                    DtimPeriod = "DtimPeriod",
                    IEEE80211H = "IEEE80211H",
                    HT_BAWinSize = "HtBaWinSize"
    }

    return dat_iw_table[key]
end

function match_dev_parm_no_ssid(key)
    local dat_iw_table = {
        BeaconPeriod = "BeaconPeriod",
        TxPower = "TxPower",
    }

    return dat_iw_table[key]
end

function match_vif_parm(key)
    local dat_iw_table = {
                    APSDCapable = "UAPSDCapable",
                    FragThreshold = "FragThreshold",
                    HT_AMSDU = "HtAmsdu",
                    HT_AutoBA = "HtAutoBa",
                    HT_GI = "HtGi",
                    HT_OpMode = "HtOpMode",
                    HT_PROTECT = "HtProcect",
                    HT_STBC = "HtStbc",
                    IgmpSnEnable = "IgmpSnEnable",
                    NoForwarding = "NoForwarding",
                    HideSSID = "HideSSID",
                    WmmCapable = "WmmCapable",
                    PMKCachePeriod = "PMKCachePeriod",
                    PreAuth = "PreAuth",
                    PMFMFPC = "PMFMFPC",
                    PMFMFPR = "PMFMFPR",
                    PMFSHA256 = "PMFSHA256",
                    VHT_STBC = "VhtStbc",
                    WirelessMode = "WirelessMode",
                    WscConfMode = "WscConfMode",
                    WscConfStatus = "WscConfStatus",
                    VHT_BW_SIGNAL = "VhtBwSignal",
     }

    return dat_iw_table[key]
end

function match_vif_parm_no_ssid(key)
    local dat_iw_table = {
                RTSThreshold = "RTSThreshold",
    }

    return dat_iw_table[key]
end

function __set_wifi_misc(cfgs, diff, device,devname)
    local vifname = device.main_ifname
    local vifext = device.ext_ifname
    local vifapcli = device.apcli_ifname
    local last_command = string.format([[
                iwpriv %s set SSID="%s";]], vifname, cfgs.SSID1)

    local vifidx = cfgs.AuthMode:split(";")
    local commands_vifs_ssid = false
    local commands_dev = false -- flag for setting ssid
    local commands_ssid = {}
    local commands_access_1 = {}
    local commands_access_2 = {} -- for black list
    local commands_bw = false -- for BW, to prevent exexute cmd twice

    for k,v in pairs(diff) do
        local commands, commands_1, commands_2, commandns
        local commands_vifs, val
        if k:find("^SSID") then
            local _,_,i = string.find(k, "^SSID([%d]+)")
            if i == "1" then
                last_command = string.format([[
                iwpriv %s set SSID="%s";]], vifname, tostring(v[2]))
                commands_dev = true
            else
                commands_ssid[tonumber(i)] = string.format([[
                iwpriv %s set SSID="%s";]], vifext..tostring(tonumber(i)-1), tostring(v[2]))
            end
        ----------------------------------------------------------------------------------------------------
                    -----------------------------device config ----------------------------
        elseif k == "Channel" or k == "channel" then
            if v[2] == "0" then
                cmdchann = string.format([[
                iwpriv %s set AutoChannelSel=3;]], vifname)
            else
                cmdchann = string.format([[
                iwpriv %s set Channel=%s;]], vifname, tostring(v[2]))
            end
            debug_info_write(devname, cmdchann)
        elseif k == "AutoChannelSelect" then
            -- do nothing
        elseif k == "PowerUpCckOfdm" or k == "powerupcckOfdm" then
            val = "0:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))
        elseif k == "PowerUpHT20" or k == "powerupht20" then
            val = "1:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))
        elseif k == "PowerUpHT40" or k == "powerupht40" then
            val = "2:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))
        elseif k == "PowerUpVHT20" or k == "powerupvht20" then
            val = "3:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))
        elseif k == "PowerUpVHT40" or k == "powerupvht40" then
            local val = "4:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))
        elseif k == "PowerUpVHT80" or k == "powerupvht80" then
            val = "5:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))
        elseif k == "PowerUpVHT160" or k == "powerupvht160" then
            val = "6:"..v[2]
            commands = string.format([[
                iwpriv %s set TxPowerBoostCtrl=%s;]], vifname, tostring(val))

        -- Find k in dat_iw_table and return the iwkey for iwpriv.
        elseif  match_dev_parm(k) then
            commands = string.format([[
                iwpriv %s set %s=%s;]], vifname, tostring(match_dev_parm(k)), tostring(v[2]))

        -- Don't need to set SSID
        elseif match_dev_parm_no_ssid(k) then
            commandns = string.format([[
                iwpriv %s set %s=%s;]], vifname, tostring(match_dev_parm_no_ssid(k)), tostring(v[2]))

        --Wps need double check
        --elseif k == "PINCode" or k == "pincode" then
            --commands = string.format([[
                --iwpriv %s set WscVendorPinCode=%s;]], vifname, tostring(v[2]))
        --elseif k == "PINPBCRadio" or k == "pinpbcradio" then
            --commands = string.format([[
                --iwpriv %s set WscMode=%s;]], vifname, tostring(v[2]))
        --elseif k == "PIN" or k == "pin" then
            --commands = string.format([[
                --iwpriv %s set WscPinCode=%s;]], vifname, tostring(v[2]))

        -- For apcli
        elseif k == "MACRepeaterEn" or k == "maprepeateren" then
            commands = string.format([[
                iwpriv %s set MACRepeaterEn=%s;]], vifapcli..tostring(0), tostring(v[2])) 

        ----------------------------------------------------------------------------------------------------
                    -----------------------------interface config ----------------------------
        -- Common case, set vif parameter and it's ssid
        elseif match_vif_parm(k) then
            for i=1, #vifidx  do
              if token(cfgs[k], i) ~= token(diff[k][2], i) then
                commands_vifs = string.format([[
                iwpriv %s set %s=%s;]], vifext..tostring(tonumber(i)-1), tostring(match_vif_parm(k)), token(diff[k][2], i))
                commands_ssid[i] = string.format([[
                iwpriv %s set SSID=%s;]], vifext..tostring(tonumber(i)-1), diff["SSID"..tostring(i)] and 
                        tostring(diff["SSID"..tostring(i)][2]) or cfgs["SSID"..tostring(i)])
                debug_info_write(devname, commands_vifs)
              end
            end

        -- Don't need to set SSID, it will take effect immediately after iwpriv
        elseif match_vif_parm_no_ssid(k) then
            for i=1, #vifidx  do
              if token(cfgs[k], i) ~= token(diff[k][2], i) then
                commands_vifs = string.format([[
                iwpriv %s set %s=%s;]], vifext..tostring(tonumber(i)-1), tostring(match_vif_parm_no_ssid(k)), token(diff[k][2], i))
                debug_info_write(devname, commands_vifs)
              end
            end

        -- Special case : need to set multiple parameters at the same time when one parameter changed
        elseif k == "RekeyInterval" or k == "rekeyinterval" then
            for i=1, #vifidx  do
              if token(cfgs.RekeyInterval, i) ~= token(diff.RekeyInterval[2], i) then
                commands_vifs = string.format([[
                iwpriv %s set RekeyInterval=%s;]], vifext..tostring(tonumber(i)-1), token(diff.RekeyInterval[2], i))
                local commands_time = string.format([[
                iwpriv %s set RekeyMethod=TIME;]], vifext..tostring(tonumber(i)-1))
                commands_ssid[i] = string.format([[
                iwpriv %s set SSID=%s;]], vifext..tostring(tonumber(i)-1), diff["SSID"..tostring(i)] and 
                        tostring(diff["SSID"..tostring(i)][2]) or cfgs["SSID"..tostring(i)])
                debug_info_write(devname, commands_vifs)
                debug_info_write(devname, commands_time)
              end
            end

        -- Special case : need to set multiple parameters at the same time when one parameter changed
        elseif not commands_bw and k == "HT_BSSCoexistence" then
            for i=1, #vifidx  do
                commands_vifs = string.format([[
                iwpriv %s set HtBssCoex=%s;]], vifext..tostring(tonumber(i)-1), tostring(v[2]))
                commands_1 = string.format([[
                iwpriv %s set HtBw=%s;]], vifext..tostring(tonumber(i)-1), diff["HT_BW"] and tostring(diff["HT_BW"][2])
                    or tostring(cfgs["HT_BW"]))
                commands_2 = string.format([[
                iwpriv %s set VhtBw=%s;]], vifext..tostring(tonumber(i)-1), diff["VHT_BW"] and tostring(diff["VHT_BW"][2])
                    or tostring(cfgs["VHT_BW"]))
                if commands_vifs then
                    debug_info_write(devname, commands_1)
                    debug_info_write(devname, commands_2)
                    debug_info_write(devname, commands_vifs)
                end
            end
            commands_vifs_ssid = true
            commands_bw = true
        elseif not commands_bw and k == "HT_BW" then
            local htbw = v[2]
            local vhtbw = diff["VHT_BW"] and tostring(diff["VHT_BW"][2]) or tostring(cfgs["VHT_BW"])
            for i=1, #vifidx  do
                commands_vifs = string.format([[
                iwpriv %s set HtBw=%s;]], vifext..tostring(tonumber(i)-1), tostring(v[2]))
                commands_1 = string.format([[
                iwpriv %s set VhtBw=%s;]], vifext..tostring(tonumber(i)-1), tostring(vhtbw))
                -- workaround
                if htbw == "1" and vhtbw == "0" then
                    commands_2 = string.format([[
                iwpriv %s set HtBssCoex=%s;]], vifext..tostring(tonumber(i)-1), diff["HT_BSSCoexistence"]
                                and tostring(diff["HT_BSSCoexistence"][2]) or tostring(cfgs["HT_BSSCoexistence"]))
                else
                    commands_2 = string.format([[
                iwpriv %s set HtBssCoex=0;]], vifext..tostring(tonumber(i)-1))
                end
                if commands_vifs then
                    debug_info_write(devname, commands_vifs)
                    debug_info_write(devname, commands_1)
                    debug_info_write(devname, commands_2)
                end
            end
            commands_vifs_ssid = true
            commands_bw = true
        elseif not commands_bw and k == "VHT_BW" then
            local vhtbw = v[2]
            local htbw = diff["HT_BW"] and tostring(diff["HT_BW"][2]) or tostring(cfgs["HT_BW"])
            for i=1, #vifidx  do
                commands_vifs = string.format([[
                iwpriv %s set VhtBw=%s;]], vifext..tostring(tonumber(i)-1), tostring(v[2]))
                commands_1 = string.format([[
                iwpriv %s set HtBw=%s;]], vifext..tostring(tonumber(i)-1), tostring(htbw)
                    or tostring(cfgs["HT_BW"]))
                 -- workaround
                if htbw == "1" and vhtbw == "0" then
                    commands_2 = string.format([[
                iwpriv %s set HtBssCoex=%s;]], vifext..tostring(tonumber(i)-1), diff["HT_BSSCoexistence"]
                                and tostring(diff["HT_BSSCoexistence"][2]) or tostring(cfgs["HT_BSSCoexistence"]))
                else
                    commands_2 = string.format([[
                iwpriv %s set HtBssCoex=0;]], vifext..tostring(tonumber(i)-1))
                end
                if commands_vifs then
                    debug_info_write(devname, commands_vifs)
                    debug_info_write(devname, commands_1)
                    debug_info_write(devname, commands_2)
                end
            end
            commands_vifs_ssid = true
            commands_bw = true
        elseif  k:find("AccessPolicy") then
            local index = string.match(k, '%d')
            if commands_access_2[index] then return end

            commands_vifs = string.format([[
                    iwpriv %s set AccessPolicy=%s;]], vifext..tostring(index), tostring(v[2]))
            debug_info_write(devname, commands_vifs)
            if v[2] == '0' then break end

            -- Delete all entry first
            local commands_del_list = string.format([[
                    iwpriv %s set ACLClearAll=1;]], vifext..tostring(index))
            debug_info_write(devname, commands_del_list)
            local list_old = cfgs["AccessControlList"..tostring(index)]:split() or {}
            local list_old_i = (list_old[1] or ''):split(";")
            local list_new = diff["AccessControlList"..tostring(index)] and diff["AccessControlList"..tostring(index)][2]:split() or {}
            local list_old_i = (list_old[1] or ''):split(";")
            local list_new_i = (list_new[1] or ''):split(";")

            if diff["AccessControlList"..tostring(index)] then
                for i=1, #list_new_i do
                    local commands_aclist = string.format([[
                    iwpriv %s set ACLAddEntry=%s;]], vifext..tostring(index), list_new_i[i])
                    debug_info_write(devname, commands_aclist)
                end
            elseif cfgs["AccessControlList"..tostring(index)] and cfgs["AccessControlList"..tostring(index)] ~= "" then
                for i=1, #list_old_i do
                    local commands_aclist = string.format([[
                    iwpriv %s set ACLAddEntry=%s;]], vifext..tostring(index), list_old_i[i])
                    debug_info_write(devname, commands_aclist)
                end
            end
            commands_access_1[index] = true
        elseif k:find("AccessControlList") then
            local index = string.match(k, '%d')
            if commands_access_1[index] then return end
            -- Clear all entry first
            local commands_del_list = string.format([[
                    iwpriv %s set ACLClearAll=1;]], vifext..tostring(index))
            debug_info_write(devname, commands_del_list)
            -- Then add entries
            local commands_ac = string.format([[
                    iwpriv %s set AccessPolicy=%s;]], vifext..tostring(index),  diff["AccessPolicy"..tostring(index)] 
                and tostring(diff["AccessPolicy"..tostring(index)][2]) or cfgs["AccessPolicy"..tostring(index)])
            debug_info_write(devname, commands_ac)
            local list_new = diff["AccessControlList"..tostring(index)] and diff["AccessControlList"..tostring(index)][2]:split() or {}
            local list_new_i = (list_new[1] or ''):split(";")

            if diff["AccessControlList"..tostring(index)] and #list_new_i > 0 then
                for i=1, #list_new_i do
                    local commands_aclist = string.format([[
                    iwpriv %s set ACLAddEntry=%s;]], vifext..tostring(index), list_new_i[i])
                    debug_info_write(devname, commands_aclist)
                end
            end
            commands_access_2[index] = true

        -- Now I assume that the reset keywords are we did not consider,
        -- and they all match the dat's keywords
        -- So, I simply set the "iwpriv vif set k=v"
        elseif vifs_cfg_parm(k) and vifs_cfg_parm(v[2]) then
          if string.find(k, "ApCli") then
            commands = string.format([[
                iwpriv %s set %s=%s;]], vifapcli..tostring(0), k, tostring(v[2]))
          else
            commands = string.format([[
                iwpriv %s set %s=%s;]], vifname, k, tostring(v[2]))
          end
        end

        if commands then
            commands_dev = true -- it will set iwpriv vif set SSID=xxx;
            debug_info_write(devname, commands)
        elseif commandns then
            debug_info_write(devname, commandns)
        end
    end

    if commands_vifs_ssid then
        for i=1, #vifidx  do
            commands_vifs_ssid = string.format([[
                iwpriv %s set SSID="%s";]], vifext..tostring(tonumber(i)-1), diff["SSID"..tostring(i)] and 
                        tostring(diff["SSID"..tostring(i)][2]) or cfgs["SSID"..tostring(i)])
            debug_info_write(devname, commands_vifs_ssid)
        end
    else
        local ssid1 = true
        for i=1, #vifidx  do
            if commands_ssid[i] then
                debug_info_write(devname, commands_ssid[i])
                if i == 1 then ssid1 = false end
            end
        end
        if ssid1 and commands_dev then debug_info_write(devname, last_command) end
    end
end

function quick_settings(devname,path)
    local mtkwifi = require("mtkwifi")
    local devs, l1parser = mtkwifi.__get_l1dat()
    local path_last, cfgs, diff, device
    assert(l1parser, "failed to parse l1profile!")

    -- If there is't /tmp/mtk/wifi/devname.last, wifi down/wifi up is necessary.
    -- Case 1: The first time wifi setup;
    -- Case 2: When reload wifi by pressing UI button.
    if not mtkwifi.exists("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".last") then
        need_downup = true 
    end
    -- Copy /tmp/mtk/wifi/devname.applied to /tmp/mtk/wifi/devname.last for diff
    if not mtkwifi.exists("/tmp/mtk/wifi/"..string.match(path, "([^/]+)\.dat")..".applied") then
        os.execute("cp -f "..path.." "..mtkwifi.__profile_previous_settings_path(path))
    else
        os.execute("cp -f "..mtkwifi.__profile_applied_settings_path(path)..
            " "..mtkwifi.__profile_previous_settings_path(path))
    end
    -- there are no /tmp/mtk/wifi/devname.last
    if need_downup then return true end

    path_last = mtkwifi.__profile_previous_settings_path(path)
    diff =  mtkwifi.diff_profile(path_last, path)
    cfgs = mtkwifi.load_profile(path_last)

    if not next(diff) then return true end -- diff == nil

    -- It maybe better to save this parms in a new file.
    need_downup_parms = {"HT_LDPC", "VHT_SGI", "VHT_LDPC", "idle_timeout_interval",
                          "E2pAccessMode", "MUTxRxEnable","DLSCapable","VHT_Sec80_Channel",
                          "Wds", "PowerUpenable", "session_timeout_interval", "MapMode",
                          "ChannelGrp","TxOP"}

    for k, v in pairs(diff) do
        nixio.syslog("debug", "quick_settings diff : "..k.."="..v[2])
        for _, pat in ipairs(need_downup_parms) do
            if string.find(k, pat) then
                need_downup = true;
                nixio.syslog("debug", "quick_settings: need_downup "..k.."="..v[2])
                break
            end
        end
        if need_downup then break end
    end
    if need_downup then return true end


    -- Quick Setting
    os.execute("rm -rf /tmp/mtk/wifi/"..devname.."_quick_setting_cmd.sh")
    device = devs.devname_ridx[devname]

    -- need to set Authmode and Encry before MFPC&MFPR
    if cfgs["ApCliEnable"] and cfgs["ApCliEnable"] == "1" or
        diff["ApCliEnable"] and diff["ApCliEnable"][2] =="1" then
        __set_wifi_apcli_security(cfgs, diff, device, devname)
    end

    __set_wifi_misc(cfgs, diff, device, devname)

    __set_he_mu(cfgs, diff, device, devname)

    -- security is complicated enough to get a special API
    __set_wifi_security(cfgs, diff, device, devname)

    --execute all iwpriv cmd
    os.execute("sh /tmp/mtk/wifi/"..devname.."_quick_setting_cmd.sh")

    -- save the quick seting log, we assume it can hold up to 10000 at most.
    if mtkwifi.exists("/tmp/mtk/wifi/"..devname.."_quick_setting_cmd.sh") then
        if mtkwifi.exists("/tmp/mtk/wifi/quick_setting_cmds.log") then
            filesize = GetFileSize("/tmp/mtk/wifi/quick_setting_cmds.log")
            if filesize > 10000 then
                os.execute("mv -f /tmp/mtk/wifi/quick_setting_cmds.log /tmp/mtk/wifi/quick_setting_cmds_bak.log")
            end
        end
        os.execute("echo ............................................... >> /tmp/mtk/wifi/quick_setting_cmds.log")
        os.execute("cat /tmp/mtk/wifi/"..devname.."_quick_setting_cmd.sh >> /tmp/mtk/wifi/quick_setting_cmds.log")
    end
    return false
end
