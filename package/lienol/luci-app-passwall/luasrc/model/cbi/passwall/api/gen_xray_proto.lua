local api = require "luci.model.cbi.passwall.api.api"
local json = require "luci.jsonc"
local inbounds = {}
local outbounds = {}
local routing = nil

local myarg = {
    "-local_proto", "-local_address", "-local_port", "-server_proto", "-server_address", "-server_port", "-server_username", "-server_password"
}

local var = api.get_args(arg, myarg)

local local_proto = var["-local_proto"]
local local_address = var["-local_address"]
local local_port = var["-local_port"]
local server_proto = var["-server_proto"]
local server_address = var["-server_address"]
local server_port = var["-server_port"]
local server_username = var["-server_username"]
local server_password = var["-server_password"]

function gen_outbound(proto, address, port, username, password)
    local result = {
        protocol = proto,
        streamSettings = {
            network = "tcp",
            security = "none"
        },
        settings = {
            servers = {
                {
                    address = address,
                    port = tonumber(port),
                    users = (username and password) and {
                        {
                            user = username,
                            pass = password
                        }
                    } or nil
                }
            }
        }
    }
    return result
end

if local_proto ~= "nil" and local_address ~= "nil" and local_port ~= "nil" then
    local inbound = {
        listen = local_address,
        port = tonumber(local_port),
        protocol = local_proto,
        settings = {
            accounts = nil
        }
    }
    if local_proto == "socks" then
        inbound.settings.auth = "noauth"
        inbound.settings.udp = true
    elseif local_proto == "http" then
        inbound.settings.allowTransparent = false
    end
    table.insert(inbounds, inbound)
end

if server_proto ~= "nil" and server_address ~= "nil" and server_port ~= "nil" then
    local outbound = gen_outbound(server_proto, server_address, server_port, server_username, server_password)
    if outbound then table.insert(outbounds, outbound) end
end

-- 额外传出连接
table.insert(outbounds, {
    protocol = "freedom", tag = "direct", settings = {keep = ""}
})

local xray = {
    log = {
        -- error = string.format("/var/etc/passwall/%s.log", node[".name"]),
        loglevel = "warning"
    },
    -- 传入连接
    inbounds = inbounds,
    -- 传出连接
    outbounds = outbounds,
    -- 路由
    routing = routing
}
print(json.stringify(xray, 1))
