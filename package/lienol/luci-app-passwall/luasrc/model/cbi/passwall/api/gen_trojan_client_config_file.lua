local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local node_section = arg[1]
local run_type = arg[2]
local proxy_port = arg[3]
local node = ucursor:get_all("passwall", node_section)

local trojan = {
    run_type = run_type,
    local_addr = "0.0.0.0",
    local_port = proxy_port,
    remote_addr = node.address,
    remote_port = tonumber(node.port),
    password = {node.password},
    log_level = 1,
    ssl = {
        verify = (node.trojan_verify_cert == "1") and true or false,
        verify_hostname = true,
        cert = node.trojan_cert_path,
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
        fast_open = (node.tcp_fast_open == "true") and true or false,
        fast_open_qlen = 20
    }
}
print(json.stringify(trojan, 1))
