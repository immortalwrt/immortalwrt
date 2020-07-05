local d = require "luci.dispatcher"
local ipkg = require("luci.model.ipkg")
local uci = require"luci.model.uci".cursor()
local api = require "luci.model.cbi.passwall.api.api"

local function is_finded(e)
    local function get_customed_path(e)
        return api.uci_get_type("global_app", e .. "_file")
    end
    return luci.sys.exec("find /usr/*bin %s -iname %s -type f" % {get_customed_path(e), e}) ~= "" and true or false
end

local function is_installed(e) return ipkg.installed(e) end

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

local v2ray_ss_encrypt_method_list = {
    "aes-128-cfb", "aes-256-cfb", "aes-128-gcm", "aes-256-gcm", "chacha20", "chacha20-ietf", "chacha20-poly1305", "chacha20-ietf-poly1305"
}

local v2ray_header_type_list = {
    "none", "srtp", "utp", "wechat-video", "dtls", "wireguard"
}

map = Map("passwall_server", translate("Server Config"))
map.redirect = d.build_url("admin", "vpn", "passwall", "server")

s = map:section(NamedSection, arg[1], "user", "")
s.addremove = false
s.dynamic = false

enable = s:option(Flag, "enable", translate("Enable"))
enable.default = "1"
enable.rmempty = false

remarks = s:option(Value, "remarks", translate("Remarks"))
remarks.default = translate("Remarks")
remarks.rmempty = false

type = s:option(ListValue, "type", translate("Type"))
if is_finded("ssr-server") then
    type:value("SSR", translate("ShadowsocksR"))
end
if is_installed("v2ray") or is_finded("v2ray") then
    type:value("V2ray", translate("V2ray"))
end
if is_installed("brook") or is_finded("brook") then
    type:value("Brook", translate("Brook"))
end
if is_installed("trojan") or is_finded("trojan") then
    type:value("Trojan", translate("Trojan"))
end

v2ray_protocol = s:option(ListValue, "v2ray_protocol", translate("Protocol"))
v2ray_protocol:value("vmess", translate("Vmess"))
v2ray_protocol:value("http", translate("HTTP"))
v2ray_protocol:value("socks", translate("Socks"))
v2ray_protocol:value("shadowsocks", translate("Shadowsocks"))
v2ray_protocol:depends("type", "V2ray")

-- Brook协议
brook_protocol = s:option(ListValue, "brook_protocol",
                          translate("Brook Protocol"))
brook_protocol:value("server", translate("Brook"))
brook_protocol:value("wsserver", translate("WebSocket"))
brook_protocol:depends("type", "Brook")

brook_tls = s:option(Flag, "brook_tls", translate("Use TLS"))
brook_tls:depends("brook_protocol", "wsserver")

port = s:option(Value, "port", translate("Port"))
port.datatype = "port"
port.rmempty = false
port:depends("type", "SSR")
port:depends({ type = "V2ray", v2ray_protocol = "vmess" })
port:depends({ type = "V2ray", v2ray_protocol = "http" })
port:depends({ type = "V2ray", v2ray_protocol = "socks" })
port:depends({ type = "V2ray", v2ray_protocol = "shadowsocks" })
port:depends("type", "Brook")
port:depends("type", "Trojan")

username = s:option(Value, "username", translate("Username"))
username:depends("v2ray_protocol", "http")
username:depends("v2ray_protocol", "socks")

password = s:option(Value, "password", translate("Password"))
password.password = true
password:depends("type", "SSR")
password:depends("type", "Brook")
password:depends("type", "Trojan")
password:depends({ type = "V2ray", v2ray_protocol = "http" })
password:depends({ type = "V2ray", v2ray_protocol = "socks" })
password:depends({ type = "V2ray", v2ray_protocol = "shadowsocks" })

ssr_encrypt_method = s:option(ListValue, "ssr_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(ssr_encrypt_method_list) do ssr_encrypt_method:value(t) end
ssr_encrypt_method:depends("type", "SSR")

v2ray_ss_encrypt_method = s:option(ListValue, "v2ray_ss_encrypt_method", translate("Encrypt Method"))
for a, t in ipairs(v2ray_ss_encrypt_method_list) do v2ray_ss_encrypt_method:value(t) end
v2ray_ss_encrypt_method:depends("v2ray_protocol", "shadowsocks")

v2ray_ss_network = s:option(ListValue, "v2ray_ss_network", translate("Transport"))
v2ray_ss_network.default = "tcp,udp"
v2ray_ss_network:value("tcp", "TCP")
v2ray_ss_network:value("udp", "UDP")
v2ray_ss_network:value("tcp,udp", "TCP,UDP")
v2ray_ss_network:depends("v2ray_protocol", "shadowsocks")

v2ray_ss_ota = s:option(Flag, "v2ray_ss_ota", translate("OTA"), translate("When OTA is enabled, V2Ray will reject connections that are not OTA enabled. This option is invalid when using AEAD encryption."))
v2ray_ss_ota.default = "0"
v2ray_ss_ota:depends("v2ray_protocol", "shadowsocks")

protocol = s:option(ListValue, "protocol", translate("Protocol"))
for a, t in ipairs(ssr_protocol_list) do protocol:value(t) end
protocol:depends("type", "SSR")

protocol_param = s:option(Value, "protocol_param", translate("Protocol_param"))
protocol_param:depends("type", "SSR")

obfs = s:option(ListValue, "obfs", translate("Obfs"))
for a, t in ipairs(ssr_obfs_list) do obfs:value(t) end
obfs:depends("type", "SSR")

obfs_param = s:option(Value, "obfs_param", translate("Obfs_param"))
obfs_param:depends("type", "SSR")

timeout = s:option(Value, "timeout", translate("Connection Timeout"))
timeout.datatype = "uinteger"
timeout.default = 300
timeout:depends("type", "SSR")

tcp_fast_open = s:option(ListValue, "tcp_fast_open", translate("TCP Fast Open"), translate("Need node support required"))
tcp_fast_open:value("false")
tcp_fast_open:value("true")
tcp_fast_open:depends("type", "SSR")
tcp_fast_open:depends("type", "Trojan")

udp_forward = s:option(Flag, "udp_forward", translate("UDP Forward"))
udp_forward.default = "1"
udp_forward.rmempty = false
udp_forward:depends("type", "SSR")

vmess_id = s:option(DynamicList, "vmess_id", translate("ID"))
for i = 1, 3 do
    local uuid = luci.sys.exec("echo -n $(cat /proc/sys/kernel/random/uuid)")
    vmess_id:value(uuid)
end
vmess_id:depends({ type = "V2ray", v2ray_protocol = "vmess" })

vmess_alterId = s:option(Value, "vmess_alterId", translate("Alter ID"))
vmess_alterId.default = 16
vmess_alterId:depends({ type = "V2ray", v2ray_protocol = "vmess" })

vmess_level = s:option(Value, "vmess_level", translate("User Level"))
vmess_level.default = 1
vmess_level:depends({ type = "V2ray", v2ray_protocol = "vmess" })
vmess_level:depends({ type = "V2ray", v2ray_protocol = "shadowsocks" })

v2ray_transport = s:option(ListValue, "v2ray_transport", translate("Transport"))
v2ray_transport:value("tcp", "TCP")
v2ray_transport:value("mkcp", "mKCP")
v2ray_transport:value("ws", "WebSocket")
v2ray_transport:value("h2", "HTTP/2")
v2ray_transport:value("ds", "DomainSocket")
v2ray_transport:value("quic", "QUIC")
v2ray_transport:depends("v2ray_protocol", "vmess")

-- [[ TCP部分 ]]--

-- TCP伪装
v2ray_tcp_guise = s:option(ListValue, "v2ray_tcp_guise", translate("Camouflage Type"))
v2ray_tcp_guise:value("none", "none")
v2ray_tcp_guise:value("http", "http")
v2ray_tcp_guise:depends("v2ray_transport", "tcp")

-- HTTP域名
v2ray_tcp_guise_http_host = s:option(DynamicList, "v2ray_tcp_guise_http_host", translate("HTTP Host"))
v2ray_tcp_guise_http_host:depends("v2ray_tcp_guise", "http")

-- HTTP路径
v2ray_tcp_guise_http_path = s:option(DynamicList, "v2ray_tcp_guise_http_path", translate("HTTP Path"))
v2ray_tcp_guise_http_path:depends("v2ray_tcp_guise", "http")

-- [[ mKCP部分 ]]--

v2ray_mkcp_guise = s:option(ListValue, "v2ray_mkcp_guise", translate("Camouflage Type"), translate('<br />none: default, no masquerade, data sent is packets with no characteristics.<br />srtp: disguised as an SRTP packet, it will be recognized as video call data (such as FaceTime).<br />utp: packets disguised as uTP will be recognized as bittorrent downloaded data.<br />wechat-video: packets disguised as WeChat video calls.<br />dtls: disguised as DTLS 1.2 packet.<br />wireguard: disguised as a WireGuard packet. (not really WireGuard protocol)'))
for a, t in ipairs(v2ray_header_type_list) do v2ray_mkcp_guise:value(t) end
v2ray_mkcp_guise:depends("v2ray_transport", "mkcp")

v2ray_mkcp_mtu = s:option(Value, "v2ray_mkcp_mtu", translate("KCP MTU"))
v2ray_mkcp_mtu:depends("v2ray_transport", "mkcp")

v2ray_mkcp_tti = s:option(Value, "v2ray_mkcp_tti", translate("KCP TTI"))
v2ray_mkcp_tti:depends("v2ray_transport", "mkcp")

v2ray_mkcp_uplinkCapacity = s:option(Value, "v2ray_mkcp_uplinkCapacity", translate("KCP uplinkCapacity"))
v2ray_mkcp_uplinkCapacity:depends("v2ray_transport", "mkcp")

v2ray_mkcp_downlinkCapacity = s:option(Value, "v2ray_mkcp_downlinkCapacity", translate("KCP downlinkCapacity"))
v2ray_mkcp_downlinkCapacity:depends("v2ray_transport", "mkcp")

v2ray_mkcp_congestion = s:option(Flag, "v2ray_mkcp_congestion", translate("KCP Congestion"))
v2ray_mkcp_congestion:depends("v2ray_transport", "mkcp")

v2ray_mkcp_readBufferSize = s:option(Value, "v2ray_mkcp_readBufferSize", translate("KCP readBufferSize"))
v2ray_mkcp_readBufferSize:depends("v2ray_transport", "mkcp")

v2ray_mkcp_writeBufferSize = s:option(Value, "v2ray_mkcp_writeBufferSize", translate("KCP writeBufferSize"))
v2ray_mkcp_writeBufferSize:depends("v2ray_transport", "mkcp")

-- [[ WebSocket部分 ]]--

v2ray_ws_host = s:option(Value, "v2ray_ws_host", translate("WebSocket Host"))
v2ray_ws_host:depends("v2ray_transport", "ws")
v2ray_ws_host:depends("v2ray_ss_transport", "ws")

v2ray_ws_path = s:option(Value, "v2ray_ws_path", translate("WebSocket Path"))
v2ray_ws_path:depends("v2ray_transport", "ws")
v2ray_ws_path:depends("v2ray_ss_transport", "ws")

-- [[ HTTP/2部分 ]]--

v2ray_h2_host = s:option(DynamicList, "v2ray_h2_host", translate("HTTP/2 Host"))
v2ray_h2_host:depends("v2ray_transport", "h2")
v2ray_h2_host:depends("v2ray_ss_transport", "h2")

v2ray_h2_path = s:option(Value, "v2ray_h2_path", translate("HTTP/2 Path"))
v2ray_h2_path:depends("v2ray_transport", "h2")
v2ray_h2_path:depends("v2ray_ss_transport", "h2")

-- [[ DomainSocket部分 ]]--

v2ray_ds_path = s:option(Value, "v2ray_ds_path", "Path", translate("A legal file path. This file must not exist before running V2Ray."))
v2ray_ds_path:depends("v2ray_transport", "ds")

-- [[ QUIC部分 ]]--
v2ray_quic_security = s:option(ListValue, "v2ray_quic_security", translate("Encrypt Method"))
v2ray_quic_security:value("none")
v2ray_quic_security:value("aes-128-gcm")
v2ray_quic_security:value("chacha20-poly1305")
v2ray_quic_security:depends("v2ray_transport", "quic")

v2ray_quic_key = s:option(Value, "v2ray_quic_key", translate("Encrypt Method") .. translate("Key"))
v2ray_quic_key:depends("v2ray_transport", "quic")

v2ray_quic_guise = s:option(ListValue, "v2ray_quic_guise", translate("Camouflage Type"))
for a, t in ipairs(v2ray_header_type_list) do v2ray_quic_guise:value(t) end
v2ray_quic_guise:depends("v2ray_transport", "quic")

remote_enable = s:option(Flag, "remote_enable", translate("Enable Remote"),translate("You can forward to Nginx/Caddy/V2ray WebSocket and more."))
remote_enable.default = "1"
remote_enable.rmempty = false
remote_enable:depends("type", "Trojan")

remote_address = s:option(Value, "remote_address", translate("Remote Address"))
remote_address.default = "127.0.0.1"
remote_address:depends("remote_enable", 1)

remote_port = s:option(Value, "remote_port", translate("Remote Port"))
remote_port.datatype = "port"
remote_port.default = "80"
remote_port:depends("remote_enable", 1)

-- [[ TLS部分 ]] --
tls_enable = s:option(Flag, "tls_enable", "TLS/SSL")
tls_enable:depends({ type = "V2ray", v2ray_protocol = "vmess", v2ray_transport = "ws" })
tls_enable:depends({ type = "V2ray", v2ray_protocol = "vmess", v2ray_transport = "h2" })
tls_enable.default = "0"
tls_enable.rmempty = false

tls_certificateFile = s:option(Value, "tls_certificateFile", translate("Public key absolute path"), translate("as:") .. "/etc/ssl/fullchain.pem")
tls_certificateFile:depends("tls_enable", 1)
tls_certificateFile:depends("type", "Trojan")

tls_keyFile = s:option(Value, "tls_keyFile", translate("Private key absolute path"), translate("as:") .. "/etc/ssl/private.key")
tls_keyFile:depends("tls_enable", 1)
tls_keyFile:depends("type", "Trojan")

local nodes_table = {}
uci:foreach("passwall", "nodes", function(e)
    if e.type and e.type == "V2ray" and e.remarks and e.address and e.port then
        nodes_table[#nodes_table + 1] = {
            id = e[".name"],
            remarks = "%s：[%s] %s:%s" % {e.type, e.remarks, e.address, e.port}
        }
    end
end)

transit_node = s:option(ListValue, "transit_node", translate("transit node"))
transit_node:value("nil", translate("Close"))
for k, v in pairs(nodes_table) do transit_node:value(v.id, v.remarks) end
transit_node.default = "nil"
transit_node:depends("type", "V2ray")

bind_local = s:option(Flag, "bind_local", translate("Bind Local"), translate("When selected, it can only be accessed locally,It is recommended to turn on when using reverse proxies."))
bind_local.default = "0"
bind_local:depends("type", "V2ray")

accept_lan = s:option(Flag, "accept_lan", translate("Accept LAN Access"), translate("When selected, it can accessed lan , this will not be safe!"))
accept_lan.default = "0"
accept_lan.rmempty = false
accept_lan:depends("type", "V2ray")

return map
