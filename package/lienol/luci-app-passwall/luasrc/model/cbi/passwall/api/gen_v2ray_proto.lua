local json = require "luci.jsonc"
local inbounds = {}
local outbounds = {}
local routing = nil

local local_proto = arg[1]
local local_address = arg[2]
local local_port = arg[3]
local server_proto = arg[4]
local server_address = arg[5]
local server_port = arg[6]
local server_username = arg[7] or "nil"
local server_password = arg[8] or "nil"

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
                    users = (username ~= "nil" and password ~= "nil") and {
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

local v2ray = {
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
print(json.stringify(v2ray, 1))
