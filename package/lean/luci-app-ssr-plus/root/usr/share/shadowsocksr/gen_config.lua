local ucursor = require"luci.model.uci".cursor()
local json = require "luci.jsonc"
local server_section = arg[1]
local proto = arg[2]
local local_port = arg[3] or "0"
local socks_port = arg[4] or "0"
local server = ucursor:get_all("shadowsocksr", server_section)
local Xray = {
	log = {
		-- error = "/var/ssrplus.log",
		loglevel = "warning"
	},
	-- 传入连接
	inbound = (local_port ~= "0") and {
		port = tonumber(local_port),
		protocol = "dokodemo-door",
		settings = {network = proto, followRedirect = true},
		sniffing = {enabled = true, destOverride = {"http", "tls"}}
	} or nil,
	-- 开启 socks 代理
	inboundDetour = (proto == "tcp" and socks_port ~= "0") and {
		{
			protocol = "socks",
			port = tonumber(socks_port),
			settings = {auth = "noauth", udp = true}
		}
	} or nil,
	-- 传出连接
	outbound = {
		protocol = server.type,
		settings = {
			vnext = {
				{
					address = server.server,
					port = tonumber(server.server_port),
					users = {
						{
							id = server.vmess_id,
							alterId = (server.type == "vmess") and tonumber(server.alter_id) or nil,
							security = (server.type == "vmess") and server.security or nil,
							encryption = (server.type == "vless") and server.vless_encryption or nil,
							flow = (server.xtls == '1') and (server.vless_flow and server.vless_flow or "xtls-rprx-splice") or nil
						}
					}
				}
			}
		},
		-- 底层传输配置
		streamSettings = {
			network = server.transport,
			security = (server.xtls == '1') and "xtls" or (server.tls == '1') and "tls" or "none",
			tlsSettings = (server.tls == '1' and (server.insecure == "1" or server.tls_host)) and {
				allowInsecure = (server.insecure == "1") and true or nil,
				serverName = server.tls_host
			} or nil,
			xtlsSettings = (server.xtls == '1' and (server.insecure == "1" or server.tls_host)) and {
				allowInsecure = (server.insecure == "1") and true or nil,
				serverName = server.tls_host
			} or nil,
			tcpSettings = (server.transport == "tcp" and server.tcp_guise == "http") and {
				header = {
					type = server.tcp_guise,
					request = {
						path = {server.http_path} or {"/"},
						headers = {Host = {server.http_host} or {}}
					}
				}
			} or nil,
			kcpSettings = (server.transport == "kcp") and {
				mtu = tonumber(server.mtu),
				tti = tonumber(server.tti),
				uplinkCapacity = tonumber(server.uplink_capacity),
				downlinkCapacity = tonumber(server.downlink_capacity),
				congestion = (server.congestion == "1") and true or false,
				readBufferSize = tonumber(server.read_buffer_size),
				writeBufferSize = tonumber(server.write_buffer_size),
				header = {type = server.kcp_guise},
				seed = server.seed or nil
			} or nil,
			wsSettings = (server.transport == "ws") and (server.ws_path or server.ws_host or server.tls_host) and {
				path = server.ws_path,
				headers = (server.ws_host or server.tls_host) and {
					Host = server.ws_host or server.tls_host
				} or nil
			} or nil,
			httpSettings = (server.transport == "h2") and {
				path = server.h2_path or "",
				host = {server.h2_host} or nil
			} or nil,
			quicSettings = (server.transport == "quic") and {
				security = server.quic_security,
				key = server.quic_key,
				header = {type = server.quic_guise}
			} or nil
		},
		mux = (server.mux == "1" and server.xtls ~= "1") and {
			enabled = true,
			concurrency = tonumber(server.concurrency)
		} or nil
	} or nil
}
local trojan = {
	log_level = 3,
	run_type = (proto == "nat" or proto == "tcp") and "nat" or "client",
	local_addr = "0.0.0.0",
	local_port = tonumber(local_port),
	remote_addr = server.server,
	remote_port = tonumber(server.server_port),
	udp_timeout = 60,
	-- 传入连接
	password = {server.password},
	-- 传出连接
	ssl = {
		verify = (server.insecure == "0") and true or false,
		verify_hostname = (server.tls == "1") and true or false,
		cert = "",
		cipher = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-CHACHA20-POLY1305:ECDHE-RSA-CHACHA20-POLY1305:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384:ECDHE-ECDSA-AES256-SHA:ECDHE-ECDSA-AES128-SHA:ECDHE-RSA-AES128-SHA:ECDHE-RSA-AES256-SHA:DHE-RSA-AES128-SHA:DHE-RSA-AES256-SHA:AES128-SHA:AES256-SHA:DES-CBC3-SHA",
		cipher_tls13 = "TLS_AES_128_GCM_SHA256:TLS_CHACHA20_POLY1305_SHA256:TLS_AES_256_GCM_SHA384",
		sni = server.tls_host,
		alpn = {"h2", "http/1.1"},
		curve = "",
		reuse_session = true,
		session_ticket = false
	},
	tcp = {
		no_delay = true,
		keep_alive = true,
		reuse_port = true,
		fast_open = (server.fast_open == "1") and true or false,
		fast_open_qlen = 20
	}
}
local naiveproxy = {
	proxy = (server.username and server.password and server.server and server.server_port) and "https://" .. server.username .. ":" .. server.password .. "@" .. server.server .. ":" .. server.server_port,
	listen = (proto == "redir") and "redir" .. "://0.0.0.0:" .. tonumber(local_port) or "socks" .. "://0.0.0.0:" .. tonumber(local_port),
	concurrency = (socks_port ~= "0") and tonumber(socks_port) or "1"
}
local ss = {
	server = (server.kcp_enable == "1") and "127.0.0.1" or server.server,
	server_port = tonumber(server.server_port),
	local_address = "0.0.0.0",
	local_port = tonumber(local_port),
	password = server.password,
	method = server.encrypt_method,
	timeout = tonumber(server.timeout) or 60,
	fast_open = (server.fast_open == "1") and true or false,
	reuse_port = true
}
if server.type == "ss" then
	if server.plugin then
		ss.plugin = server.plugin
		ss.plugin_opts = (server.simple_obfs) and server.simple_obfs .. server.plugin_opts or (server.v2ray_plugin ~= "none") and server.v2ray_plugin .. server.plugin_opts or nil
	end
	print(json.stringify(ss, 1))
end
if server.type == "ssr" then
	ss.protocol = server.protocol
	ss.protocol_param = server.protocol_param
	ss.obfs = server.obfs
	ss.obfs_param = server.obfs_param
	print(json.stringify(ss, 1))
end
if server.type == "vless" or server.type == "vmess" then
	print(json.stringify(Xray, 1))
end
if server.type == "trojan" then
	print(json.stringify(trojan, 1))
end
if server.type == "naiveproxy" then
	print(json.stringify(naiveproxy, 1))
end
