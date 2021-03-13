module("luci.model.cbi.passwall.api.gen_xray", package.seeall)
local api = require "luci.model.cbi.passwall.api.api"

local myarg = {
    "-node", "-proto", "-redir_port", "-socks_proxy_port", "-http_proxy_port", "-dns_listen_port", "-dns_server", "-doh_url", "-doh_host", "-doh_socks_address", "-doh_socks_port", "-loglevel"
}

local var = api.get_args(arg, myarg)

local node_section = var["-node"]
local proto = var["-proto"]
local redir_port = var["-redir_port"]
local socks_proxy_port = var["-socks_proxy_port"]
local http_proxy_port = var["-http_proxy_port"]
local dns_listen_port = var["-dns_listen_port"]
local dns_server = var["-dns_server"]
local doh_url = var["-doh_url"]
local doh_host = var["-doh_host"]
local doh_socks_address = var["-doh_socks_address"]
local doh_socks_port = var["-doh_socks_port"]
local loglevel = var["-loglevel"] or "warning"
local network = proto
local new_port

local ucursor = require"luci.model.uci".cursor()
local sys = require "luci.sys"
local json = require "luci.jsonc"
local appname = api.appname
local dns = nil
local inbounds = {}
local outbounds = {}
local routing = nil

local function get_new_port()
    if new_port then
        new_port = tonumber(sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port %s tcp)", appname, new_port + 1)))
    else
        new_port = tonumber(sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port auto tcp)", appname)))
    end
    return new_port
end

function gen_outbound(node, tag, is_proxy, proxy_tag)
    local result = nil
    if node and node ~= "nil" then
        local node_id = node[".name"]
        if tag == nil then
            tag = node_id
        end

        if proxy_tag then
            node.proxySettings = {
                tag = proxy_tag,
                transportLayer = true
            }
        end

        if node.type == "Xray" or node.type == "V2ray" then
            is_proxy = nil
        end

        if node.type ~= "Xray" and node.type ~= "V2ray" then
            if node.type == "Socks" then
                node.protocol = "socks"
                node.transport = "tcp"
            else
                local node_type = proto or "socks"
                local relay_port = node.port
                new_port = get_new_port()
                node.port = new_port
                sys.call(string.format('/usr/share/%s/app.sh run_socks "%s" "%s" "%s" "%s" "%s" "%s" "%s" "%s"> /dev/null', appname,
                    new_port, --flag
                    node_id, --node
                    "127.0.0.1", --bind
                    new_port, --socks port
                    string.format("/var/etc/%s/v2_%s_%s_%s.json", appname, node_type, node_id, new_port), --config file
                    "0", --http port
                    "nil", -- http config file
                    (is_proxy and is_proxy == "1" and relay_port) and tostring(relay_port) or "" --relay port
                    )
                )
                node.protocol = "socks"
                node.transport = "tcp"
                node.address = "127.0.0.1"
            end
            node.stream_security = "none"
        else
            if node.tls and node.tls == "1" then
                node.stream_security = "tls"
                if node.xtls and node.xtls == "1" then
                    node.stream_security = "xtls"
                end
            end
        end

        result = {
            _flag_tag = node_id,
            _flag_is_proxy = (is_proxy and is_proxy == "1") and "1" or "0",
            tag = tag,
            proxySettings = node.proxySettings or nil,
            protocol = node.protocol,
            mux = (node.stream_security ~= "xtls") and {
                enabled = (node.mux == "1") and true or false,
                concurrency = (node.mux_concurrency) and tonumber(node.mux_concurrency) or 8
            } or nil,
            -- 底层传输配置
            streamSettings = (node.protocol == "vmess" or node.protocol == "vless" or node.protocol == "socks" or node.protocol == "shadowsocks" or node.protocol == "trojan") and {
                network = node.transport,
                security = node.stream_security,
                xtlsSettings = (node.stream_security == "xtls") and {
                    serverName = node.tls_serverName,
                    allowInsecure = (node.tls_allowInsecure == "1") and true or false
                } or nil,
                tlsSettings = (node.stream_security == "tls") and {
                    serverName = node.tls_serverName,
                    allowInsecure = (node.tls_allowInsecure == "1") and true or false
                } or nil,
                tcpSettings = (node.transport == "tcp" and node.protocol ~= "socks") and {
                    header = {
                        type = node.tcp_guise,
                        request = (node.tcp_guise == "http") and {
                            path = node.tcp_guise_http_path or {"/"},
                            headers = {
                                Host = node.tcp_guise_http_host or {}
                            }
                        } or nil
                    }
                } or nil,
                kcpSettings = (node.transport == "mkcp") and {
                    mtu = tonumber(node.mkcp_mtu),
                    tti = tonumber(node.mkcp_tti),
                    uplinkCapacity = tonumber(node.mkcp_uplinkCapacity),
                    downlinkCapacity = tonumber(node.mkcp_downlinkCapacity),
                    congestion = (node.mkcp_congestion == "1") and true or false,
                    readBufferSize = tonumber(node.mkcp_readBufferSize),
                    writeBufferSize = tonumber(node.mkcp_writeBufferSize),
                    seed = (node.mkcp_seed and node.mkcp_seed ~= "") and node.mkcp_seed or nil,
                    header = {type = node.mkcp_guise}
                } or nil,
                wsSettings = (node.transport == "ws") and {
                    path = node.ws_path or "",
                    headers = (node.ws_host ~= nil) and
                        {Host = node.ws_host} or nil
                } or nil,
                httpSettings = (node.transport == "h2") and
                    {path = node.h2_path, host = node.h2_host} or
                    nil,
                dsSettings = (node.transport == "ds") and
                    {path = node.ds_path} or nil,
                quicSettings = (node.transport == "quic") and {
                    security = node.quic_security,
                    key = node.quic_key,
                    header = {type = node.quic_guise}
                } or nil
            } or nil,
            settings = {
                vnext = (node.protocol == "vmess" or node.protocol == "vless") and {
                    {
                        address = node.address,
                        port = tonumber(node.port),
                        users = {
                            {
                                id = node.uuid,
                                alterId = tonumber(node.alter_id),
                                level = 0,
                                security = (node.protocol == "vmess") and node.security or nil,
                                encryption = node.encryption or "none",
                                flow = node.flow or nil
                            }
                        }
                    }
                } or nil,
                servers = (node.protocol == "socks" or node.protocol == "http" or node.protocol == "shadowsocks" or node.protocol == "trojan") and {
                    {
                        address = node.address,
                        port = tonumber(node.port),
                        method = node.method or nil,
                        flow = node.flow or nil,
                        password = node.password or "",
                        users = (node.username and node.password) and
                            {{user = node.username, pass = node.password}} or nil
                    }
                } or nil
            }
        }
    end
    return result
end

if node_section then
    local node = ucursor:get_all(appname, node_section)
    if socks_proxy_port then
        table.insert(inbounds, {
            listen = "0.0.0.0",
            port = tonumber(socks_proxy_port),
            protocol = "socks",
            settings = {auth = "noauth", udp = true}
        })
        network = "tcp,udp"
    end
    if http_proxy_port then
        table.insert(inbounds, {
            listen = "0.0.0.0",
            port = tonumber(http_proxy_port),
            protocol = "http",
            settings = {allowTransparent = false}
        })
    end

    if redir_port then
        table.insert(inbounds, {
            port = tonumber(redir_port),
            protocol = "dokodemo-door",
            settings = {network = proto, followRedirect = true},
            sniffing = {enabled = true, destOverride = {"http", "tls"}}
        })
        if proto == "tcp" and node.tcp_socks == "1" then
            table.insert(inbounds, {
                listen = "0.0.0.0",
                port = tonumber(node.tcp_socks_port),
                protocol = "socks",
                settings = {
                    auth = node.tcp_socks_auth,
                    accounts = (node.tcp_socks_auth == "password") and {
                        {
                            user = node.tcp_socks_auth_username,
                            pass = node.tcp_socks_auth_password
                        }
                    } or nil,
                    udp = true
                }
            })
        end
    end

    local up_trust_doh = ucursor:get(appname, "@global[0]", "up_trust_doh")
    if up_trust_doh then
        local t = {}
        string.gsub(up_trust_doh, '[^' .. "," .. ']+', function (w)
            table.insert(t, w)
        end)
        if #t > 1 then
            local host = sys.exec("echo -n $(echo " .. t[1] .. " | sed 's/https:\\/\\///g' | awk -F ':' '{print $1}' | awk -F '/' '{print $1}')")
            dns = {
                hosts = {
                    [host] = t[2]
                }
            }
        end
    end

    if node.protocol == "_shunt" then
        local rules = {}

        local default_node_id = node.default_node or "_direct"
        local default_outboundTag
        if default_node_id == "_direct" then
            default_outboundTag = "direct"
        elseif default_node_id == "_blackhole" then
            default_outboundTag = "blackhole"
        else
            local default_node = ucursor:get_all(appname, default_node_id)
            local main_node_id = node.main_node or "nil"
            local is_proxy = "0"
            local proxy_tag
            if main_node_id ~= "nil" then
                if main_node_id ~= default_node_id then
                    local main_node = ucursor:get_all(appname, main_node_id)
                    local main_node_outbound = gen_outbound(main_node, "main")
                    if main_node_outbound then
                        table.insert(outbounds, main_node_outbound)
                        is_proxy = "1"
                        proxy_tag = "main"
                        if default_node.type ~= "Xray" and default_node.type ~= "V2ray" then
                            proxy_tag = nil
                            new_port = get_new_port()
                            table.insert(inbounds, {
                                tag = "proxy_default",
                                listen = "127.0.0.1",
                                port = new_port,
                                protocol = "dokodemo-door",
                                settings = {network = "tcp,udp", address = default_node.address, port = tonumber(default_node.port)}
                            })
                            if default_node.tls_serverName == nil then
                                default_node.tls_serverName = default_node.address
                            end
                            default_node.address = "127.0.0.1"
                            default_node.port = new_port
                            table.insert(rules, 1, {
                                type = "field",
                                inboundTag = {"proxy_default"},
                                outboundTag = "main"
                            })
                        end
                    end
                end
            end
            local default_outbound = gen_outbound(default_node, "default", is_proxy, proxy_tag)
            if default_outbound then
                table.insert(outbounds, default_outbound)
                default_outboundTag = "default"
            end
        end

        ucursor:foreach(appname, "shunt_rules", function(e)
            local name = e[".name"]
            local _node_id = node[name] or "nil"
            local is_proxy = node[name .. "_proxy"] or "0"
            local outboundTag
            if _node_id == "_direct" then
                outboundTag = "direct"
            elseif _node_id == "_blackhole" then
                outboundTag = "blackhole"
            elseif _node_id == "_default" then
                outboundTag = "default"
            else
                if _node_id ~= "nil" then
                    local has_outbound
                    for index, value in ipairs(outbounds) do
                        if value["_flag_tag"] == _node_id and value["_flag_is_proxy"] == is_proxy then
                            has_outbound = api.clone(value)
                            break
                        end
                    end
                    if has_outbound then
                        has_outbound["tag"] = name
                        table.insert(outbounds, has_outbound)
                        outboundTag = name
                    else
                        local _node = ucursor:get_all(appname, _node_id)
                        if node.type ~= "Xray" and node.type ~= "V2ray" then
                            if is_proxy == "1" then
                                new_port = get_new_port()
                                table.insert(inbounds, {
                                    tag = "proxy_" .. name,
                                    listen = "127.0.0.1",
                                    port = new_port,
                                    protocol = "dokodemo-door",
                                    settings = {network = "tcp,udp", address = _node.address, port = tonumber(_node.port)}
                                })
                                if _node.tls_serverName == nil then
                                    _node.tls_serverName = _node.address
                                end
                                _node.address = "127.0.0.1"
                                _node.port = new_port
                                table.insert(rules, 1, {
                                    type = "field",
                                    inboundTag = {"proxy_" .. name},
                                    outboundTag = "default"
                                })
                            end
                        end
                        local _outbound = gen_outbound(_node, name, is_proxy, (is_proxy == "1" and "default" or nil))
                        if _outbound then
                            table.insert(outbounds, _outbound)
                            outboundTag = name
                        end
                    end
                end
            end
            if outboundTag then
                if outboundTag == "default" then 
                    outboundTag = default_outboundTag
                end
                local protocols = nil
                if e["protocol"] and e["protocol"] ~= "" then
                    protocols = {}
                    string.gsub(e["protocol"], '[^' .. " " .. ']+', function(w)
                        table.insert(protocols, w)
                    end)
                end
                if e.domain_list then
                    local _domain = {}
                    string.gsub(e.domain_list, '[^' .. "\r\n" .. ']+', function(w)
                        table.insert(_domain, w)
                    end)
                    table.insert(rules, {
                        type = "field",
                        outboundTag = outboundTag,
                        domain = _domain,
                        protocol = protocols
                    })
                end
                if e.ip_list then
                    local _ip = {}
                    string.gsub(e.ip_list, '[^' .. "\r\n" .. ']+', function(w)
                        table.insert(_ip, w)
                    end)
                    table.insert(rules, {
                        type = "field",
                        outboundTag = outboundTag,
                        ip = _ip,
                        protocol = protocols
                    })
                end
                if not e.domain_list and not e.ip_list and protocols then
                    table.insert(rules, {
                        type = "field",
                        outboundTag = outboundTag,
                        protocol = protocols
                    })
                end
            end
        end)

        if default_outboundTag then 
            table.insert(rules, {
                type = "field",
                outboundTag = default_outboundTag,
                network = network
            })
        end

        routing = {
            domainStrategy = node.domainStrategy or "AsIs",
            rules = rules
        }
    elseif node.protocol == "_balancing" then
        if node.balancing_node then
            local nodes = node.balancing_node
            local length = #nodes
            for i = 1, length do
                local node = ucursor:get_all(appname, nodes[i])
                local outbound = gen_outbound(node)
                if outbound then table.insert(outbounds, outbound) end
            end
            routing = {
                domainStrategy = node.domainStrategy or "AsIs",
                balancers = {{tag = "balancer", selector = nodes}},
                rules = {
                    {type = "field", network = "tcp,udp", balancerTag = "balancer"}
                }
            }
        end
    else
        local outbound = gen_outbound(node)
        if outbound then table.insert(outbounds, outbound) end
    end
end

if dns_server then
    local rules = {}

    dns = {
        tag = "dns-in1",
        servers = {
            dns_server
        }
    }
    if doh_url and doh_host then
        dns.hosts = {
            [doh_host] = dns_server
        }
        dns.servers = {
            doh_url
        }
    end

    if dns_listen_port then
        table.insert(inbounds, {
            listen = "127.0.0.1",
            port = tonumber(dns_listen_port),
            protocol = "dokodemo-door",
            tag = "dns-in",
            settings = {
                address = dns_server,
                port = 53,
                network = "udp"
            }
        })
    end

    table.insert(rules, {
        type = "field",
        inboundTag = {
            "dns-in"
        },
        outboundTag = "dns-out"
    })

    local outboundTag = "direct"
    if doh_socks_address and doh_socks_port then
        table.insert(outbounds, 1, {
            tag = "out",
            protocol = "socks",
            streamSettings = {
                network = "tcp",
                security = "none"
            },
            settings = {
                servers = {
                    {
                        address = doh_socks_address,
                        port = tonumber(doh_socks_port)
                    }
                }
            }
        })
        outboundTag = "out"
    end
    table.insert(rules, {
        type = "field",
        inboundTag = {
            "dns-in1"
        },
        outboundTag = outboundTag
    })
    
    routing = {
        domainStrategy = "IPOnDemand",
        rules = rules
    }
end

if inbounds or outbounds then
    table.insert(outbounds, {
        protocol = "freedom",
        tag = "direct",
        settings = {
            domainStrategy = "UseIPv4"
        },
        streamSettings = {
            sockopt = {
                mark = 255
            }
        }
    })
    table.insert(outbounds, {
        protocol = "blackhole",
        tag = "blackhole"
    })
    table.insert(outbounds, {
        protocol = "dns",
        tag = "dns-out"
    })

    local xray = {
        log = {
            -- error = string.format("/var/etc/%s/%s.log", appname, node[".name"]),
            loglevel = loglevel
        },
        -- DNS
        dns = dns,
        -- 传入连接
        inbounds = inbounds,
        -- 传出连接
        outbounds = outbounds,
        -- 路由
        routing = routing,
        -- 本地策略
        --[[
        policy = {
            levels = {
                [0] = {
                    handshake = 4,
                    connIdle = 300,
                    uplinkOnly = 2,
                    downlinkOnly = 5,
                    bufferSize = 10240,
                    statsUserUplink = false,
                    statsUserDownlink = false
                }
            },
            system = {
                statsInboundUplink = false,
                statsInboundDownlink = false
            }
        }
        ]]--
    }
    print(json.stringify(xray, 1))
end
