local d = require "luci.dispatcher"
local ipkg = require("luci.model.ipkg")

local appname = "passwall"

local function is_finded(e)
    return luci.sys.exec("find /usr/*bin -iname " .. e .. " -type f") ~= "" and
               true or false
end

local function is_installed(e) return ipkg.installed(e) end

local ss_encrypt_method_list = {
    "rc4-md5", "aes-128-cfb", "aes-192-cfb", "aes-256-cfb", "aes-128-ctr",
    "aes-192-ctr", "aes-256-ctr", "bf-cfb", "camellia-128-cfb",
    "camellia-192-cfb", "camellia-256-cfb", "salsa20", "chacha20",
    "chacha20-ietf", -- aead
    "aes-128-gcm", "aes-192-gcm", "aes-256-gcm", "chacha20-ietf-poly1305",
    "xchacha20-ietf-poly1305"
}

local ssr_encrypt_method_list = {
    "none", "table", "rc2-cfb", "rc4", "rc4-md5", "rc4-md5-6", "aes-128-cfb",
    "aes-192-cfb", "aes-256-cfb", "aes-128-ctr", "aes-192-ctr", "aes-256-ctr",
    "bf-cfb", "camellia-128-cfb", "camellia-192-cfb", "camellia-256-cfb",
    "cast5-cfb", "des-cfb", "idea-cfb", "seed-cfb", "salsa20", "chacha20",
    "chacha20-ietf"
}

local ssr_protocol_list = {
    "origin", "verify_simple", "verify_deflate", "verify_sha1", "auth_simple",
    "auth_sha1", "auth_sha1_v2", "auth_sha1_v4", "auth_aes128_md5",
    "auth_aes128_sha1", "auth_chain_a", "auth_chain_b", "auth_chain_c",
    "auth_chain_d", "auth_chain_e", "auth_chain_f"
}
local ssr_obfs_list = {
    "plain", "http_simple", "http_post", "random_head", "tls_simple",
    "tls1.0_session_auth", "tls1.2_ticket_auth"
}

local v2ray_security_list = {"none", "auto", "aes-128-gcm", "chacha20-poly1305"}

local v2ray_header_type_list = {
    "none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}

m = Map(appname, translate("ShadowSocks Server Config"),
        translate("TCP quick open server does not support do not open.") ..
            translate("HAProxy cannot be used with KCP."))
m.redirect = d.build_url("admin", "vpn", "passwall")

s = m:section(NamedSection, arg[1], "servers", "")
s.addremove = false
s.dynamic = false

remarks = s:option(Value, "remarks", translate("Node Remarks"))
remarks.default = translate("Node Remarks")
remarks.rmempty = false

server_type = s:option(ListValue, "server_type", translate("Server Type"))
if ((is_installed("redsocks2") or is_finded("redsocks2")) or
    (is_installed("ipt2socks") or is_finded("ipt2socks"))) then
    server_type:value("Socks5", translate("Socks5 Server"))
end
if is_finded("ss-redir") then
    server_type:value("SS", translate("Shadowsocks Server"))
end
if is_finded("ssr-redir") then
    server_type:value("SSR", translate("ShadowsocksR Server"))
end
if is_installed("v2ray") then
    server_type:value("V2ray", translate("V2ray Server"))
end
if is_installed("brook") or is_finded("brook") then
    server_type:value("Brook", translate("Brook Server"))
end
if is_installed("trojan") or is_finded("trojan") then
    server_type:value("Trojan", translate("Trojan Server"))
end

v2ray_protocol = s:option(ListValue, "v2ray_protocol",
                          translate("V2ray Protocol"))
v2ray_protocol:value("vmess", translate("Vmess"))
v2ray_protocol:depends("server_type", "V2ray")

server = s:option(Value, "server",
                  translate("Server Address (Support Domain Name)"))
server.rmempty = false

use_ipv6 = s:option(Flag, "use_ipv6", translate("Use IPv6"))
use_ipv6.default = 0

server_port = s:option(Value, "server_port", translate("Server Port"))
server_port.datatype = "port"
server_port.rmempty = false

username = s:option(Value, "username", translate("Username"))
username:depends("server_type", "Socks5")

password = s:option(Value, "password", translate("Password"))
password.password = true
password:depends("server_type", "Socks5")
password:depends("server_type", "SS")
password:depends("server_type", "SSR")
password:depends("server_type", "Brook")
password:depends("server_type", "Trojan")

ss_encrypt_method = s:option(ListValue, "ss_encrypt_method",
                             translate("Encrypt Method"))
for a, t in ipairs(ss_encrypt_method_list) do ss_encrypt_method:value(t) end
ss_encrypt_method:depends("server_type", "SS")

ssr_encrypt_method = s:option(ListValue, "ssr_encrypt_method",
                              translate("Encrypt Method"))
for a, t in ipairs(ssr_encrypt_method_list) do ssr_encrypt_method:value(t) end
ssr_encrypt_method:depends("server_type", "SSR")

v2ray_security = s:option(ListValue, "v2ray_security",
                          translate("Encrypt Method"))
for a, t in ipairs(v2ray_security_list) do v2ray_security:value(t) end
v2ray_security:depends("server_type", "V2ray")

protocol = s:option(ListValue, "protocol", translate("Protocol"))
for a, t in ipairs(ssr_protocol_list) do protocol:value(t) end
protocol:depends("server_type", "SSR")

protocol_param = s:option(Value, "protocol_param", translate("Protocol_param"))
protocol_param:depends("server_type", "SSR")

obfs = s:option(ListValue, "obfs", translate("Obfs"))
for a, t in ipairs(ssr_obfs_list) do obfs:value(t) end
obfs:depends("server_type", "SSR")

obfs_param = s:option(Value, "obfs_param", translate("Obfs_param"))
obfs_param:depends("server_type", "SSR")

timeout = s:option(Value, "timeout", translate("Connection Timeout"))
timeout.datatype = "uinteger"
timeout.default = 300
timeout:depends("server_type", "SS")
timeout:depends("server_type", "SSR")

fast_open = s:option(ListValue, "fast_open", translate("Fast_open"))
fast_open:value("false")
fast_open:value("true")
fast_open:depends("server_type", "SS")
fast_open:depends("server_type", "SSR")
fast_open:depends("server_type", "Trojan")

use_kcp = s:option(Flag, "use_kcp", translate("Use Kcptun"),
                   "<span style='color:red'>" .. translate(
                       "Please confirm whether the Kcptun is installed. If not, please go to Rule Update download installation.") ..
                       "</span>")
use_kcp.default = 0
use_kcp:depends("server_type", "SS")
use_kcp:depends("server_type", "SSR")
use_kcp:depends("server_type", "Brook")

kcp_server = s:option(Value, "kcp_server", translate("Kcptun Server"))
kcp_server.placeholder = translate("Default:Current Server")
kcp_server:depends("use_kcp", "1")

kcp_use_ipv6 = s:option(Flag, "kcp_use_ipv6", translate("Use IPv6"))
kcp_use_ipv6.default = 0
kcp_use_ipv6:depends("use_kcp", "1")

kcp_port = s:option(Value, "kcp_port", translate("Kcptun Port"))
kcp_port.datatype = "port"
kcp_port:depends("use_kcp", "1")

kcp_opts = s:option(TextValue, "kcp_opts", translate("Kcptun Config"),
                    translate(
                        "--crypt aes192 --key abc123 --mtu 1350 --sndwnd 128 --rcvwnd 1024 --mode fast"))
kcp_opts.placeholder =
    "--crypt aes192 --key abc123 --mtu 1350 --sndwnd 128 --rcvwnd 1024 --mode fast"
kcp_opts:depends("use_kcp", "1")

v2ray_VMess_id = s:option(Value, "v2ray_VMess_id", translate("ID"))
v2ray_VMess_id.password = true
v2ray_VMess_id:depends("v2ray_protocol", "vmess")

v2ray_VMess_alterId = s:option(Value, "v2ray_VMess_alterId",
                               translate("Alter ID"))
v2ray_VMess_alterId:depends("v2ray_protocol", "vmess")

v2ray_VMess_level =
    s:option(Value, "v2ray_VMess_level", translate("User Level"))
v2ray_VMess_level.default = 1
v2ray_VMess_level:depends("server_type", "V2ray")

v2ray_stream_security = s:option(ListValue, "v2ray_stream_security",
                                 translate("Transport Layer Encryption"),
                                 translate(
                                     'Whether or not transport layer encryption is enabled, the supported options are "none" for unencrypted (default) and "TLS" for using TLS.'))
v2ray_stream_security:value("none", "none")
v2ray_stream_security:value("tls", "tls")
v2ray_stream_security:depends("server_type", "V2ray")

-- [[ TLS部分 ]] --
tls_serverName = s:option(Value, "tls_serverName", translate("Domain"))
tls_serverName:depends("v2ray_stream_security", "tls")

tls_allowInsecure = s:option(Flag, "tls_allowInsecure",
                             translate("allowInsecure"), translate(
                                 "Whether unsafe connections are allowed. When checked, V2Ray does not check the validity of the TLS certificate provided by the remote host."))
tls_allowInsecure.default = "0"
tls_allowInsecure.rmempty = false
tls_allowInsecure:depends("v2ray_stream_security", "tls")

v2ray_transport = s:option(ListValue, "v2ray_transport", translate("Transport"))
v2ray_transport:value("tcp", "TCP")
v2ray_transport:value("mkcp", "mKCP")
v2ray_transport:value("ws", "WebSocket")
v2ray_transport:value("h2", "HTTP/2")
v2ray_transport:value("ds", "DomainSocket")
v2ray_transport:value("quic", "QUIC")
v2ray_transport:depends("server_type", "V2ray")

-- [[ TCP部分 ]]--

-- TCP伪装
v2ray_tcp_guise = s:option(ListValue, "v2ray_tcp_guise",
                           translate("Camouflage Type"))
v2ray_tcp_guise:depends("v2ray_transport", "tcp")
v2ray_tcp_guise:value("none", "none")
v2ray_tcp_guise:value("http", "http")

-- HTTP域名
v2ray_tcp_guise_http_host = s:option(DynamicList, "v2ray_tcp_guise_http_host",
                                     translate("HTTP Host"))
v2ray_tcp_guise_http_host:depends("v2ray_tcp_guise", "http")

-- HTTP路径
v2ray_tcp_guise_http_path = s:option(DynamicList, "v2ray_tcp_guise_http_path",
                                     translate("HTTP Path"))
v2ray_tcp_guise_http_path:depends("v2ray_tcp_guise", "http")

-- [[ mKCP部分 ]]--

v2ray_mkcp_guise = s:option(ListValue, "v2ray_mkcp_guise",
                            translate("Camouflage Type"), translate(
                                '<br>none: default, no masquerade, data sent is packets with no characteristics.<br>srtp: disguised as an SRTP packet, it will be recognized as video call data (such as FaceTime).<br>utp: packets disguised as uTP will be recognized as bittorrent downloaded data.<br>wechat-video: packets disguised as WeChat video calls.<br>dtls: disguised as DTLS 1.2 packet.<br>wireguard: disguised as a WireGuard packet. (not really WireGuard protocol)'))
for a, t in ipairs(v2ray_header_type_list) do v2ray_mkcp_guise:value(t) end
v2ray_mkcp_guise:depends("v2ray_transport", "mkcp")

v2ray_mkcp_mtu = s:option(Value, "v2ray_mkcp_mtu", translate("KCP MTU"))
v2ray_mkcp_mtu:depends("v2ray_transport", "mkcp")

v2ray_mkcp_tti = s:option(Value, "v2ray_mkcp_tti", translate("KCP TTI"))
v2ray_mkcp_tti:depends("v2ray_transport", "mkcp")

v2ray_mkcp_uplinkCapacity = s:option(Value, "v2ray_mkcp_uplinkCapacity",
                                     translate("KCP uplinkCapacity"))
v2ray_mkcp_uplinkCapacity:depends("v2ray_transport", "mkcp")

v2ray_mkcp_downlinkCapacity = s:option(Value, "v2ray_mkcp_downlinkCapacity",
                                       translate("KCP downlinkCapacity"))
v2ray_mkcp_downlinkCapacity:depends("v2ray_transport", "mkcp")

v2ray_mkcp_congestion = s:option(Flag, "v2ray_mkcp_congestion",
                                 translate("KCP Congestion"))
v2ray_mkcp_congestion:depends("v2ray_transport", "mkcp")

v2ray_mkcp_readBufferSize = s:option(Value, "v2ray_mkcp_readBufferSize",
                                     translate("KCP readBufferSize"))
v2ray_mkcp_readBufferSize:depends("v2ray_transport", "mkcp")

v2ray_mkcp_writeBufferSize = s:option(Value, "v2ray_mkcp_writeBufferSize",
                                      translate("KCP writeBufferSize"))
v2ray_mkcp_writeBufferSize:depends("v2ray_transport", "mkcp")

-- [[ WebSocket部分 ]]--

v2ray_ws_host = s:option(Value, "v2ray_ws_host", translate("WebSocket Host"))
v2ray_ws_host:depends("v2ray_transport", "ws")

v2ray_ws_path = s:option(Value, "v2ray_ws_path", translate("WebSocket Path"))
v2ray_ws_path:depends("v2ray_transport", "ws")

-- [[ HTTP/2部分 ]]--

v2ray_h2_host = s:option(DynamicList, "v2ray_h2_host", translate("HTTP/2 Host"))
v2ray_h2_host:depends("v2ray_transport", "h2")

v2ray_h2_path = s:option(Value, "v2ray_h2_path", translate("HTTP/2 Path"))
v2ray_h2_path:depends("v2ray_transport", "h2")

-- [[ DomainSocket部分 ]]--

v2ray_ds_path = s:option(Value, "v2ray_ds_path", "Path", translate(
                             "A legal file path. This file must not exist before running V2Ray."))
v2ray_ds_path:depends("v2ray_transport", "ds")

-- [[ QUIC部分 ]]--
v2ray_quic_security = s:option(ListValue, "v2ray_quic_security",
                               translate("Encrypt Method"))
v2ray_quic_security:value("none")
v2ray_quic_security:value("aes-128-gcm")
v2ray_quic_security:value("chacha20-poly1305")
v2ray_quic_security:depends("v2ray_transport", "quic")

v2ray_quic_key = s:option(Value, "v2ray_quic_key",
                          translate("Encrypt Method") .. translate("Key"))
v2ray_quic_key:depends("v2ray_transport", "quic")

v2ray_quic_guise = s:option(ListValue, "v2ray_quic_guise",
                            translate("Camouflage Type"))
for a, t in ipairs(v2ray_header_type_list) do v2ray_quic_guise:value(t) end
v2ray_quic_guise:depends("v2ray_transport", "quic")

-- [[ 其它 ]]--

v2ray_mux = s:option(Flag, "v2ray_mux", translate("Mux"))
v2ray_mux:depends("server_type", "V2ray")

v2ray_mux_concurrency = s:option(Value, "v2ray_mux_concurrency",
                                 translate("Mux Concurrency"))
v2ray_mux_concurrency.default = 8
v2ray_mux_concurrency:depends("v2ray_mux", "1")

-- v2ray_insecure = s:option(Flag, "v2ray_insecure", translate("allowInsecure"))
-- v2ray_insecure:depends("server_type", "V2ray")

function rmempty_restore()
    password.rmempty = true
    timeout.rmempty = true
    fast_open.rmempty = true
    v2ray_protocol.rmempty = true
    v2ray_VMess_id.rmempty = true
    v2ray_VMess_alterId.rmempty = true
end

server_type.validate = function(self, value)
    rmempty_restore()
    if value == "SS" then
        password.rmempty = false
        timeout.rmempty = false
        fast_open.rmempty = false
    elseif value == "SSR" then
        password.rmempty = false
        timeout.rmempty = false
        fast_open.rmempty = false
    elseif value == "V2ray" then
        v2ray_protocol.rmempty = false
        v2ray_VMess_id.rmempty = false
        v2ray_VMess_alterId.rmempty = false
    elseif value == "Brook" then
        password.rmempty = false
    elseif value == "Trojan" then
        password.rmempty = false
        fast_open.rmempty = false
    end
    return value
end

v2ray_transport.validate = function(self, value) return value end

return m
