module("luci.model.cbi.passwall.server.api.trojan", package.seeall)
function gen_config(user)
    local config = {
        run_type = "server",
        local_addr = "0.0.0.0",
        local_port = tonumber(user.port),
        remote_addr = (user.remote_enable == "1" and user.remote_address) and user.remote_address or nil,
        remote_port = (user.remote_enable == "1" and user.remote_port) and user.remote_port or nil,
        password = user.password,
        log_level = 1,
        ssl = {
            cert = user.tls_certificateFile,
            key = user.tls_keyFile,
            key_password = "",
            cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA",
            cipher_tls13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384",
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
            reuse_port = false,
            fast_open = (user.fast_open == "true") and true or false,
            fast_open_qlen = 20
        }
    }
    return config
end