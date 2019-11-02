local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2]
local redir_port = arg[3]
local socks5_proxy_port = arg[4]
local server = ucursor:get_all("passwall", server_section)

local trojan = {
    run_type = "client",
    local_addr = "0.0.0.0",
    local_port = socks5_proxy_port,
    remote_addr = server.server,
    remote_port = tonumber(server.server_port),
    password = {server.password},
    log_level = 1,
    ssl = {
        verify = true,
        verify_hostname = true,
        cert = "",
        cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:RSA-AES128-GCM-SHA256:RSA-AES256-GCM-SHA384:RSA-AES128-SHA:RSA-AES256-SHA:RSA-3DES-EDE-SHA",
        sni = "",
        alpn = {"h2", "http/1.1"},
        reuse_session = true,
        session_ticket = false,
        curves = ""
    },
    tcp = {
        no_delay = true,
        keep_alive = true,
        fast_open = false,
        fast_open_qlen = 20
    }
}
print(json.stringify(trojan, 1))
