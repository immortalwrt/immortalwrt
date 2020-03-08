local uci = require"luci.model.uci".cursor()
local sys = require "luci.sys"
local jsonc = require "luci.jsonc"

local json = {}

function genconfig(i, section, d, u)
    local server = uci:get_all("ssr_python_pro_server", section)
    local enable = server.enable
    local remarks = server.remarks
    local port = server.port
    local password = server.password
    local method = server.method
    local protocol = server.protocol
    local obfs = server.obfs
    local device_limit = server.device_limit
    local speed_limit_per_con = server.speed_limit_per_con
    local speed_limit_per_user = server.speed_limit_per_user
    local forbidden_port = server.forbidden_port
    local transfer_enable = server.transfer_enable

    transfer_enable = transfer_enable and tonumber(transfer_enable) == 0 and
                          838868 or transfer_enable

    json[i] = {
        id = section,
        enable = tonumber(enable),
        user = remarks,
        port = tonumber(port),
        passwd = password,
        method = method,
        protocol = protocol,
        obfs = obfs,
        protocol_param = device_limit,
        speed_limit_per_con = tonumber(speed_limit_per_con),
        speed_limit_per_user = tonumber(speed_limit_per_user),
        forbidden_port = forbidden_port and forbidden_port or "",
        transfer_enable = transfer_enable and transfer_enable * 1024 * 1024 *
            1024 or 1073741824,
        d = d and tonumber(d) or 0,
        u = u and tonumber(u) or 0
    }
end

local mudbjson = luci.sys.exec("cat /usr/share/ssr_python_pro_server/mudb.json")
local mudbjson_object = jsonc.parse(mudbjson)

local i = 0
uci:foreach("ssr_python_pro_server", "user", function(s)
    i = i + 1
    local section = s[".name"]
    if mudbjson_object then
        local flag = true
        for index = 1, table.maxn(mudbjson_object) do
            local object = mudbjson_object[index]
            if mudbjson_object[index] ~= nil then
                if object.id == section then
                    flag = false
                    genconfig(i, section, object.d, object.u)
                    mudbjson_object[index] = nil
                    break
                end
            end
        end
        if flag == true then genconfig(i, section, 0, 0) end
    else
        genconfig(i, section, 0, 0)
    end
end)

print(jsonc.stringify(json, 1))
