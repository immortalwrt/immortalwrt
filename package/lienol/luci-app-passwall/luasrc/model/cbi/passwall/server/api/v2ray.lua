module("luci.model.cbi.passwall.server.api.v2ray", package.seeall)
local ucic = require"luci.model.uci".cursor()

function gen_config(user)
    local settings = nil
    local routing = nil
    local outbounds = {
        {protocol = "freedom"}, {protocol = "blackhole", tag = "blocked"}
    }

    if user.v2ray_protocol == "vmess" then
        if user.vmess_id then
            local clients = {}
            for i = 1, #user.vmess_id do
                clients[i] = {
                    id = user.vmess_id[i],
                    level = tonumber(user.vmess_level),
                    alterId = tonumber(user.vmess_alterId)
                }
            end
            settings = {clients = clients}
        end
    elseif user.v2ray_protocol == "socks" then
        settings = {
            auth = (user.username == nil and user.password == nil) and "noauth" or "password",
            accounts = {
                {
                    user = (user.username == nil) and "" or user.username,
                    pass = (user.password == nil) and "" or user.password
                }
            }
        }
    elseif user.v2ray_protocol == "http" then
        settings = {
            allowTransparent = false,
            accounts = {
                {
                    user = (user.username == nil) and "" or user.username,
                    pass = (user.password == nil) and "" or user.password
                }
            }
        }
    elseif user.v2ray_protocol == "shadowsocks" then
        settings = {
            method = user.v2ray_ss_encrypt_method,
            password = user.password,
            level = tonumber(user.vmess_level) or 1,
            network = user.v2ray_ss_network or "TCP,UDP",
            ota = (user.v2ray_ss_ota == '1') and true or false
        }
    end

    if user.accept_lan == nil or user.accept_lan == "0" then
        routing = {
            domainStrategy = "IPOnDemand",
            rules = {
                {
                    type = "field",
                    ip = {"10.0.0.0/8", "172.16.0.0/12", "192.168.0.0/16"},
                    outboundTag = "blocked"
                }
            }
        }
    end

    if user.transit_node and user.transit_node ~= "nil" then
        local node = ucic:get_all("passwall", user.transit_node)
        if node and node ~= "nil" and node.type and node.type == "V2ray" then
            local transit_node = {
                tag = "transit",
                protocol = node.v2ray_protocol or "vmess",
                mux = {
                    enabled = (node.v2ray_mux == "1") and true or false,
                    concurrency = (node.v2ray_mux_concurrency) and
                        tonumber(node.v2ray_mux_concurrency) or 8
                },
                -- 底层传输配置
                streamSettings = (node.v2ray_protocol == "vmess") and {
                    network = node.v2ray_transport,
                    security = node.v2ray_stream_security,
                    tlsSettings = (node.v2ray_stream_security == "tls") and {
                        serverName = node.tls_serverName,
                        allowInsecure = (node.tls_allowInsecure == "1") and true or
                            false
                    } or nil,
                    tcpSettings = (node.v2ray_transport == "tcp") and {
                        header = {
                            type = node.v2ray_tcp_guise,
                            request = {
                                path = node.v2ray_tcp_guise_http_path or {"/"},
                                headers = {
                                    Host = node.v2ray_tcp_guise_http_host or {}
                                }
                            } or {}
                        }
                    } or nil,
                    kcpSettings = (node.v2ray_transport == "mkcp") and {
                        mtu = tonumber(node.v2ray_mkcp_mtu),
                        tti = tonumber(node.v2ray_mkcp_tti),
                        uplinkCapacity = tonumber(node.v2ray_mkcp_uplinkCapacity),
                        downlinkCapacity = tonumber(
                            node.v2ray_mkcp_downlinkCapacity),
                        congestion = (node.v2ray_mkcp_congestion == "1") and
                            true or false,
                        readBufferSize = tonumber(node.v2ray_mkcp_readBufferSize),
                        writeBufferSize = tonumber(
                            node.v2ray_mkcp_writeBufferSize),
                        header = {type = node.v2ray_mkcp_guise}
                    } or nil,
                    wsSettings = (node.v2ray_transport == "ws") and {
                        path = node.v2ray_ws_path or "",
                        headers = (node.v2ray_ws_host ~= nil) and
                            {Host = node.v2ray_ws_host} or nil
                    } or nil,
                    httpSettings = (node.v2ray_transport == "h2") and
                        {path = node.v2ray_h2_path, host = node.v2ray_h2_host} or
                        nil,
                    dsSettings = (node.v2ray_transport == "ds") and
                        {path = node.v2ray_ds_path} or nil,
                    quicSettings = (node.v2ray_transport == "quic") and {
                        security = node.v2ray_quic_security,
                        key = node.v2ray_quic_key,
                        header = {type = node.v2ray_quic_guise}
                    } or nil
                } or nil,
                settings = {
                    vnext = (node.v2ray_protocol == "vmess") and {
                        {
                            address = node.address,
                            port = tonumber(node.port),
                            users = {
                                {
                                    id = node.v2ray_VMess_id,
                                    alterId = tonumber(node.v2ray_VMess_alterId),
                                    level = tonumber(node.v2ray_VMess_level),
                                    security = node.v2ray_security
                                }
                            }
                        }
                    } or nil,
                    servers = (node.v2ray_protocol == "http" or
                        node.v2ray_protocol == "socks" or node.v2ray_protocol == "shadowsocks") and {
                        {
                            address = node.address,
                            port = tonumber(node.port),
                            method = node.v2ray_ss_encrypt_method,
                            password = node.password or "",
                            ota = (node.v2ray_ss_ota == '1') and true or false,
                            users = (node.username and node.password) and
                                {
                                    {
                                        user = node.username or "",
                                        pass = node.password or ""
                                    }
                                } or nil
                        }
                    } or nil
                }
            }
            table.insert(outbounds, 1, transit_node)
        end
    end

    local config = {
        log = {
            -- error = "/var/log/v2ray.log",
            loglevel = "warning"
        },
        -- 传入连接
        inbounds = {
            {
                listen = (user.bind_local == "1") and "127.0.0.1" or nil,
                port = tonumber(user.port),
                protocol = user.v2ray_protocol,
                -- 底层传输配置
                settings = settings,
                streamSettings = (user.v2ray_protocol == "vmess") and {
                    network = user.v2ray_transport,
                    security = (user.tls_enable == '1') and "tls" or "none",
                    tlsSettings = (user.tls_enable == '1') and {
                        -- serverName = (user.tls_serverName),
                        allowInsecure = false,
                        disableSystemRoot = false,
                        certificates = {
                            {
                                certificateFile = user.tls_certificateFile,
                                keyFile = user.tls_keyFile
                            }
                        }
                    } or nil,
                    tcpSettings = (user.v2ray_transport == "tcp") and {
                        header = {
                            type = user.v2ray_tcp_guise,
                            request = {
                                path = user.v2ray_tcp_guise_http_path or {"/"},
                                headers = {
                                    Host = user.v2ray_tcp_guise_http_host or {}
                                }
                            } or {}
                        }
                    } or nil,
                    kcpSettings = (user.v2ray_transport == "mkcp") and {
                        mtu = tonumber(user.v2ray_mkcp_mtu),
                        tti = tonumber(user.v2ray_mkcp_tti),
                        uplinkCapacity = tonumber(user.v2ray_mkcp_uplinkCapacity),
                        downlinkCapacity = tonumber(user.v2ray_mkcp_downlinkCapacity),
                        congestion = (user.v2ray_mkcp_congestion == "1") and true or false,
                        readBufferSize = tonumber(user.v2ray_mkcp_readBufferSize),
                        writeBufferSize = tonumber(user.v2ray_mkcp_writeBufferSize),
                        header = {type = user.v2ray_mkcp_guise}
                    } or nil,
                    wsSettings = (user.v2ray_transport == "ws") and
                        {
                            headers = (user.v2ray_ws_host) and {Host = user.v2ray_ws_host} or
                                nil,
                            path = user.v2ray_ws_path
                        } or nil,
                    httpSettings = (user.v2ray_transport == "h2") and
                        {path = user.v2ray_h2_path, host = user.v2ray_h2_host} or nil,
                    dsSettings = (user.v2ray_transport == "ds") and
                        {path = user.v2ray_ds_path} or nil,
                    quicSettings = (user.v2ray_transport == "quic") and {
                        security = user.v2ray_quic_security,
                        key = user.v2ray_quic_key,
                        header = {type = user.v2ray_quic_guise}
                    } or nil
                } or nil
            }
        },
        -- 传出连接
        outbounds = outbounds,
        routing = routing
    }
    return config
end
