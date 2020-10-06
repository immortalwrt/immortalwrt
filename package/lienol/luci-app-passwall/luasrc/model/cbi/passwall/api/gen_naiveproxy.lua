local ucursor = require "luci.model.uci".cursor()
local jsonc = require "luci.jsonc"
local node_section = arg[1]
local run_type = arg[2]
local local_addr = arg[3]
local local_port = arg[4]
local server_host = arg[5]
local server_port = arg[6]
local node = ucursor:get_all("passwall", node_section)

local config = {
    listen = run_type .. "://" .. local_addr .. ":" .. local_port,
    proxy = node.protocol .. "://" .. node.username .. ":" .. node.password .. "@" .. (server_host or node.address) .. ":" .. (server_port or node.port)
}

print(jsonc.stringify(config, 1))
