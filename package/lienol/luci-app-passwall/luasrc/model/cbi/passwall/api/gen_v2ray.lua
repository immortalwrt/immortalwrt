local ucursor = require"luci.model.uci".cursor()
local sys = require "luci.sys"
local json = require "luci.jsonc"
local appname = "passwall"
local node_section = arg[1]
local proto = arg[2]
local redir_port = arg[3]
local socks_proxy_port = arg[4]
local node = ucursor:get_all(appname, node_section)
local inbounds = {}
local outbounds = {}
local network = proto
local routing = nil

local function gen_outbound(node, tag)
    local result = nil
    if node then
        local node_id = node[".name"]
        if tag == nil then
            tag = node_id
        end
        if node.type ~= "V2ray" then
            if node.type == "Socks" then
                node.v2ray_protocol = "socks"
                node.v2ray_transport = "tcp"
            else
                local node_type = (proto and proto ~= "nil") and proto or "socks"
                local new_port = sys.exec(string.format("echo -n $(/usr/share/%s/app.sh get_new_port auto tcp)", appname))
                node.port = new_port
                sys.call(string.format("/usr/share/%s/app.sh run_socks %s %s %s %s %s", 
                    appname,
                    node_id,
                    "127.0.0.1",
                    new_port,
                    string.format("/var/etc/%s/v2_%s_%s.json", appname, node_type, node_id),
                    "4")
                )
                node.v2ray_protocol = "socks"
                node.v2ray_transport = "tcp"
                node.address = "127.0.0.1"
            end
        end
        result = {
            tag = tag,
            protocol = node.v2ray_protocol or "vmess",
            mux = {
                enabled = (node.v2ray_mux == "1") and true or false,
                concurrency = (node.v2ray_mux_concurrency) and tonumber(node.v2ray_mux_concurrency) or 8
            },
            -- 底层传输配置
            streamSettings = (node.v2ray_protocol == "vmess") and {
                network = node.v2ray_transport,
                security = node.v2ray_stream_security,
                tlsSettings = (node.v2ray_stream_security == "tls") and {
                    serverName = node.tls_serverName,
                    allowInsecure = (node.tls_allowInsecure == "1") and true or false
                } or nil,
                tcpSettings = (node.v2ray_transport == "tcp" and
                    node.v2ray_protocol ~= "socks") and {
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
                    downlinkCapacity = tonumber(node.v2ray_mkcp_downlinkCapacity),
                    congestion = (node.v2ray_mkcp_congestion == "1") and true or false,
                    readBufferSize = tonumber(node.v2ray_mkcp_readBufferSize),
                    writeBufferSize = tonumber(node.v2ray_mkcp_writeBufferSize),
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
                servers = (node.v2ray_protocol == "socks") and {
                    {
                        address = node.address,
                        port = tonumber(node.port),
                        users = (node.username and node.password) and
                            {{user = node.username, pass = node.password}} or nil
                    }
                } or nil
            }
        }
    end
    return result
end

if socks_proxy_port ~= "nil" then
    table.insert(inbounds, {
        listen = "0.0.0.0",
        port = socks_proxy_port,
        protocol = "socks",
        settings = {auth = "noauth", udp = true, ip = "127.0.0.1"}
    })
    network = "tcp,udp"
end

if redir_port ~= "nil" then
    table.insert(inbounds, {
        port = redir_port,
        protocol = "dokodemo-door",
        settings = {network = proto, followRedirect = true},
        sniffing = {enabled = true, destOverride = {"http", "tls"}}
    })
    if proto == "tcp" and node.v2ray_tcp_socks == "1" then
        table.insert(inbounds, {
            listen = "0.0.0.0",
            port = tonumber(node.v2ray_tcp_socks_port),
            protocol = "socks",
            settings = {
                auth = node.v2ray_tcp_socks_auth,
                accounts = (node.v2ray_tcp_socks_auth == "password") and {
                    {
                        user = node.v2ray_tcp_socks_auth_username,
                        pass = node.v2ray_tcp_socks_auth_password
                    }
                } or nil,
                udp = true
            }
        })
    end
end

if node.v2ray_protocol == "_shunt" then
    local rules = {}

    ucursor:foreach(appname, "shunt_rules", function(e)
        local _node_id = node[e[".name"]] or nil
        if _node_id and _node_id ~= "nil" then
            local _node = ucursor:get_all(appname, _node_id)
            local _outbound = gen_outbound(_node, e[".name"])
            if _outbound then
                table.insert(outbounds, _outbound)
                if e.domain_list then
                    local _domain = {}
                    string.gsub(e.domain_list, '[^' .. "\r\n" .. ']+', function(w)
                        table.insert(_domain, w)
                    end)
                    table.insert(rules, {
                        type = "field",
                        outboundTag = e[".name"],
                        domain = _domain
                    })
                end
                if e.ip_list then
                    local _ip = {}
                    string.gsub(e.ip_list, '[^' .. "\r\n" .. ']+', function(w)
                        table.insert(_ip, w)
                    end)
                    table.insert(rules, {
                        type = "field",
                        outboundTag = e[".name"],
                        ip = _ip
                    })
                end
            end
        end
    end)
    
    local default_node_id = node.default_node or nil
    if default_node_id and default_node_id ~= "nil" then
        local default_node = ucursor:get_all(appname, default_node_id)
        local default_outbound = gen_outbound(default_node, "default")
        if default_outbound then
            table.insert(outbounds, default_outbound)
            local rule = {
                type = "field",
                outboundTag = "default",
                network = network
            }
            table.insert(rules, rule)
        end
    end

    routing = {domainStrategy = "IPOnDemand", rules = rules}

elseif node.v2ray_protocol == "_balancing" then
    if node.v2ray_balancing_node then
        local nodes = node.v2ray_balancing_node
        local length = #nodes
        for i = 1, length do
            local node = ucursor:get_all(appname, nodes[i])
            local outbound = gen_outbound(node)
            if outbound then table.insert(outbounds, outbound) end
        end
        routing = {
            domainStrategy = "IPOnDemand",
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

-- 额外传出连接
table.insert(outbounds, {protocol = "freedom", tag = "direct", settings = {keep = ""}})

local v2ray = {
    log = {
        -- error = "/var/log/v2ray.log",
        loglevel = "warning"
    },
    -- 传入连接
    inbounds = inbounds,
    -- 传出连接
    outbounds = outbounds,
    -- 路由
    routing = routing
}
print(json.stringify(v2ray, 1))
