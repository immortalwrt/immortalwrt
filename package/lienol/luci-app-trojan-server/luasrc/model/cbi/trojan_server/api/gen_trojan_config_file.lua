local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local server = ucursor:get_all("trojan_server", server_section)

local trojan = {
    run_type = "server",
    local_addr = "0.0.0.0",
    local_port = tonumber(server.port),
    --remote_addr = "127.0.0.1",
    --remote_port = 80,
    password = server.password,
    log_level = 1,
    ssl = {
        cert = server.ssl_certFile,
        key = server.ssl_keyFile,
        key_password = "",
        cipher = "ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES256-SHA384:ECDHE-RSA-AES256-SHA384:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256",
        prefer_server_cipher = true,
        alpn = {"http/1.1"},
        reuse_session = true,
        session_ticket = false,
        session_timeout = 600,
        plain_http_response = "",
        curves = "",
        dhparam = ""
    },
    tcp = {
        prefer_ipv4 = false,
        no_delay = true,
        keep_alive = true,
        fast_open = (server.fast_open == "true") and true or false,
        fast_open_qlen = 20
    }
}
print(json.stringify(trojan, 1))
