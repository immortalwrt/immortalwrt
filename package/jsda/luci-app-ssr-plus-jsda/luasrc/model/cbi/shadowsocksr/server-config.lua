-- Copyright (C) 2017 yushi studio <ywb94@qq.com>
-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local shadowsocksr = "shadowsocksr"
local uci = luci.model.uci.cursor()
local fs = require "nixio.fs"
local sys = require "luci.sys"
local sid = arg[1]
local uuid = luci.sys.exec("cat /proc/sys/kernel/random/uuid")
local http = luci.http
local ucursor = require "luci.model.uci".cursor()

local encrypt_methods = {
	"rc4-md5",
	"rc4-md5-6",
	"rc4",
	"table",
	"aes-128-gcm",
	"aes-192-gcm",
	"aes-256-gcm",
	"aes-128-cfb",
	"aes-192-cfb",
	"aes-256-cfb",
	"aes-128-ctr",
	"aes-192-ctr",
	"aes-256-ctr",
	"aes-128-gcm",
	"aes-192-gcm",
	"aes-256-gcm",	
	"bf-cfb",
	"camellia-128-cfb",
	"camellia-192-cfb",
	"camellia-256-cfb",
	"cast5-cfb",
	"des-cfb",
	"idea-cfb",
	"rc2-cfb",
	"seed-cfb",
	"salsa20",
	"chacha20",
	"chacha20-ietf",
	"chacha20-ietf-poly1305",
	"xchacha20-ietf-poly1305",
}

local protocol = {
	"origin",
}

local obfs = {
	"plain",
	"http_simple",
	"http_post",
}

local securitys = {
    "auto",
    "none",
    "aes-128-gcm",
    "chacha20-poly1305"
}


m = Map(shadowsocksr, translate("Edit ShadowSocksR Server"))

m.redirect = luci.dispatcher.build_url("admin/services/shadowsocksr/server")
if m.uci:get(shadowsocksr, sid) ~= "server_config" then
	luci.http.redirect(m.redirect) 
	return
end




-- [[ Server Setting ]]--
s = m:section(NamedSection, sid, "server_config")
s.anonymous = true
s.addremove   = false

o = s:option(Flag, "enable", translate("Enable"))
o.default = 1
o.rmempty = false

o = s:option(ListValue, "type", translate("Server Node Type"))
if nixio.fs.access("/usr/bin/ssr-server") then
o:value("ssr", translate("ShadowsocksR"))
end
if nixio.fs.access("/usr/bin/ss-server") then
o:value("ss", translate("Shadowsocks New Version"))
end
if nixio.fs.access("/usr/bin/v2ray/v2ray") then
o:value("v2ray", translate("V2Ray"))
end
o.description = translate("Using incorrect encryption mothod may causes service fail to start")

use_conf_file = s:option(Flag, "use_conf_file", translate("Use Config File"), translate("Use Config File"))	
use_conf_file:depends("type", "v2ray")	
use_conf_file.rmempty = false	

conf_file_path = s:option(Value, "conf_file_path", translate("Config File Path"),	
	translate("Add the file name. JSON after the path."))	
conf_file_path.default = "/etc/shadowsocksr/"	
conf_file_path:depends("use_conf_file", 1)	

upload_conf = s:option(FileUpload, "")	
upload_conf.template = "cbi/ssr_other_upload2"	
upload_conf:depends("use_conf_file", 1)	

um = s:option(DummyValue, "", nil)	
um.template = "cbi/ssr_other_dvalue"	
um:depends("use_conf_file", 1)	

local conf_dir, fd	
conf_dir = "/etc/shadowsocksr/"	
nixio.fs.mkdir(conf_dir)	
http.setfilehandler(	
	function(meta, chunk, eof)	
		if not fd then	
			if not meta then return end	

 			if	meta and chunk then fd = nixio.open(conf_dir .. meta.file, "w") end	

 			if not fd then	
				um.value = translate("Create upload file error.")	
				return	
			end	
		end	
		if chunk and fd then	
			fd:write(chunk)	
		end	
		if eof and fd then	
			fd:close()	
			fd = nil	
			um.value = translate("File saved to") .. ' "/etc/shadowsocksr/' .. meta.file .. '"'	
			ucursor:set("v2ray","v2ray","conf_file_path","/etc/shadowsocksr/" .. meta.file)	
			ucursor:commit("v2ray")	
		end	
	end	
)	

if luci.http.formvalue("upload") then	
	local f = luci.http.formvalue("ulfile")	
	if #f <= 0 then	
		um.value = translate("No specify upload file.")	
	end	
end

o = s:option(Flag, "ipv4_ipv6", translate("Enabling IPv6 server"))
o.default = 0
o.rmempty = false

o = s:option(Value, "server_port", translate("Server Port"))
o.datatype = "port"
o.default = 8388
o.rmempty = false

o = s:option(Value, "timeout", translate("Connection Timeout"))
o.datatype = "uinteger"
o.default = 60
o.rmempty = false

o = s:option(Value, "password", translate("Password"))
o.password = true
o.rmempty = true
o:depends("type", "ssr")
o:depends("type", "ss")

o = s:option(ListValue, "encrypt_method", translate("Encrypt Method"))
for _, v in ipairs(encrypt_methods) do o:value(v) end
o.rmempty = true
o:depends("type", "ssr")
o:depends("type", "ss")

o = s:option(ListValue, "plugin", translate("plugin"))
o:value("none", "None")
if nixio.fs.access("/usr/bin/v2ray-plugin") then
o:value("/usr/bin/v2ray-plugin", "v2ray-plugin")
end
if nixio.fs.access("/usr/bin/obfs-server") then
o:value("/usr/bin/obfs-server", "obfs-server")
end
if nixio.fs.access("/usr/bin/gq-server") then
o:value("/usr/bin/gq-server", "GoQuiet")
end
o.rmempty = false
o.default = "none"
o:depends("type", "ss")

o = s:option(Value, "plugin_opts", translate("Plugin Opts"))
o.rmempty = true
o:depends("plugin", "/usr/bin/v2ray-plugin")
o:depends("plugin", "/usr/bin/obfs-server")
o:depends("plugin", "/usr/bin/gq-server")

o = s:option(ListValue, "protocol", translate("Protocol"))
for _, v in ipairs(protocol) do o:value(v) end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(ListValue, "obfs", translate("Obfs"))
for _, v in ipairs(obfs) do o:value(v) end
o.rmempty = true
o:depends("type", "ssr")

o = s:option(Value, "obfs_param", translate("Obfs param(optional)"))
o:depends("type", "ssr")

-- AlterId
o = s:option(Value, "alter_id", translate("AlterId"))
o.default = 100
o.rmempty = true
o:depends("type", "v2ray")

o=s:option(Value,"VMess_level",translate("User Level"))
o.default=1
o:depends("type", "v2ray")

-- VmessId
o = s:option(Value, "vmess_id", translate("VmessId (UUID)"))
o.rmempty = true
o.default = uuid
o:depends("type", "v2ray")

-- 加密方式
o = s:option(ListValue, "security", translate("Encrypt Method"))
for _, v in ipairs(securitys) do o:value(v, v:upper()) end
o.rmempty = true
o:depends("type", "v2ray")

-- 传输协议
o = s:option(ListValue, "transport", translate("Transport"))
o:value("tcp", "TCP")
o:value("kcp", "mKCP")
o:value("ws", "WebSocket")
o:value("h2", "HTTP/2")
o:value("quic", "QUIC")
o.rmempty = true
o:depends("type", "v2ray")

-- [[ TCP部分 ]]--

-- TCP伪装
o = s:option(ListValue, "tcp_guise", translate("Camouflage Type"))
o:depends("transport", "tcp")
o:value("none", translate("None"))
o:value("http", "HTTP")
o.rmempty = true

-- HTTP域名
o = s:option(DynamicList, "http_host", translate("HTTP Host"))
o:depends("tcp_guise", "http")
o.rmempty = true

-- HTTP路径
o = s:option(DynamicList, "http_path", translate("HTTP Path"))
o:depends("tcp_guise", "http")
o.rmempty = true

-- [[ WS部分 ]]--

-- WS域名
o = s:option(Value, "ws_host", translate("WebSocket Host"))
o:depends("transport", "ws")
o.rmempty = true

-- WS路径
o = s:option(Value, "ws_path", translate("WebSocket Path"))
o:depends("transport", "ws")
o.rmempty = true

-- [[ H2部分 ]]--

-- H2域名
o = s:option(DynamicList, "h2_host", translate("HTTP/2 Host"))
o:depends("transport", "h2")
o.rmempty = true

-- H2路径
o = s:option(Value, "h2_path", translate("HTTP/2 Path"))
o:depends("transport", "h2")
o.rmempty = true

-- [[ QUIC部分 ]]--

o = s:option(ListValue, "quic_security", translate("QUIC Security"))
o:depends("transport", "quic")
o.rmempty = true
o:value("none", translate("None"))
o:value("aes-128-gcm", translate("aes-128-gcm"))
o:value("chacha20-poly1305", translate("chacha20-poly1305"))

o = s:option(Value, "quic_key", translate("QUIC Key"))
o:depends("transport", "quic")
o.rmempty = true

o = s:option(ListValue, "quic_guise", translate("Header"))
o:depends("transport", "quic")
o.rmempty = true
o:value("none", translate("None"))
o:value("srtp", translate("VideoCall (SRTP)"))
o:value("utp", translate("BitTorrent (uTP)"))
o:value("wechat-video", translate("WechatVideo"))
o:value("dtls", "DTLS 1.2")
o:value("wireguard", "WireGuard")

-- [[ mKCP部分 ]]--

o = s:option(ListValue, "kcp_guise", translate("Camouflage Type"))
o:depends("transport", "kcp")
o:value("none", translate("None"))
o:value("srtp", translate("VideoCall (SRTP)"))
o:value("utp", translate("BitTorrent (uTP)"))
o:value("wechat-video", translate("WechatVideo"))
o:value("dtls", "DTLS 1.2")
o:value("wireguard", "WireGuard")
o.rmempty = true

o = s:option(Value, "mtu", translate("MTU"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 1350
o.rmempty = true

o = s:option(Value, "tti", translate("TTI"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 50
o.rmempty = true

o = s:option(Value, "uplink_capacity", translate("Uplink Capacity"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 50
o.rmempty = true

o = s:option(Value, "downlink_capacity", translate("Downlink Capacity"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 20
o.rmempty = true

o = s:option(Value, "read_buffer_size", translate("Read Buffer Size"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 2
o.rmempty = true

o = s:option(Value, "write_buffer_size", translate("Write Buffer Size"))
o.datatype = "uinteger"
o:depends("transport", "kcp")
o.default = 2
o.rmempty = true

o = s:option(Flag, "congestion", translate("Congestion"))
o:depends("transport", "kcp")
o.rmempty = true

-- [[ allowInsecure ]]--
o = s:option(Flag, "insecure", translate("allowInsecure"))
o.rmempty = true
o:depends("type", "v2ray")

-- [[ TLS ]]--
o = s:option(Flag, "tls", translate("TLS"))
o.rmempty = true
o.default = "0"
o:depends("type", "v2ray")

-- [[ Mux ]]--
o = s:option(Flag, "mux", translate("Mux"))
o.rmempty = true
o.default = "0"
o:depends("type", "v2ray")

o = s:option(Value, "concurrency", translate("Concurrency"))
o.datatype = "uinteger"
o.rmempty = true
o.default = "8"
o:depends("mux", "1")

o = s:option(Flag, "fast_open", translate("TCP Fast Open"))
o.rmempty = true
o:depends("type", "ssr")

return m
