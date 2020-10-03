local ucursor = require "luci.model.uci".cursor()
local jsonc = require "luci.jsonc"
local node_section = arg[1]
local run_type = arg[2]
local local_addr = arg[3]
local local_port = arg[4]
local relay_port = arg[5]
local node = ucursor:get_all("passwall", node_section)

if relay_port and relay_port ~= "nil" then
    node.address = "127.0.0.1"
    node.port = tonumber(relay_port)
end

local config = {
    listen = run_type .. "://" .. local_addr .. ":" .. local_port,
    proxy = node.protocol .. "://" .. node.username .. ":" .. node.password .. "@" .. node.address .. ":" .. node.port
}

print(jsonc.stringify(config, 1))
