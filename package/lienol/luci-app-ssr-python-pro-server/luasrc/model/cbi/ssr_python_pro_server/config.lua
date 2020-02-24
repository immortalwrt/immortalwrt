local i = "ssr_python_pro_server"
local d = require "luci.dispatcher"
local a, t, e

local methods = {
    "none", "table", "rc4", "rc4-md5", "aes-128-cfb", "aes-192-cfb",
    "aes-256-cfb", "aes-128-ctr", "aes-192-ctr", "aes-256-ctr", "bf-cfb",
    "cast5-cfb", "des-cfb", "rc2-cfb", "salsa20", "chacha20", "chacha20-ietf"
}
local protocols = {
    "origin", "verify_simple", "verify_deflate", "verify_sha1", "auth_simple",
    "auth_sha1", "auth_sha1_v2", "auth_sha1_v4", "auth_aes128_md5",
    "auth_aes128_sha1", "auth_chain_a", "auth_chain_b", "auth_chain_c",
    "auth_chain_d"
}
local obfss = {
    "plain", "http_simple", "http_post", "random_head", "tls_simple",
    "tls1.0_session_auth", "tls1.2_ticket_auth"
}

a = Map(i, "ShadowsocksR Python " .. translate("Server Config"))
a.redirect = d.build_url("admin", "vpn", "ssr_python_pro_server")

t = a:section(NamedSection, arg[1], "user", "")
t.addremove = false
t.dynamic = false

e = t:option(Flag, "enable", translate("Enable"))
e.default = "1"
e.rmempty = false

e = t:option(Value, "remarks", translate("Remarks"))
e.default = translate("Remarks")
e.rmempty = false

e = t:option(Value, "port", translate("Port"))
e.datatype = "port"
e.rmempty = false

e = t:option(Value, "password", translate("Password"))
e.password = true
e.rmempty = false

e = t:option(ListValue, "method", translate("Encrypt Method"))
for a, t in ipairs(methods) do e:value(t) end

e = t:option(ListValue, "protocol", translate("Protocol"))
for a, t in ipairs(protocols) do e:value(t) end

e = t:option(ListValue, "obfs", translate("Obfs"))
for a, t in ipairs(obfss) do e:value(t) end

e = t:option(Value, "device_limit", translate("Device Limit"), translate(
                 "Number of clients that can be linked at the same time (multi-port mode, each port is calculated independently), a minimum of 2 is recommended."))
e.default = "2"
e.rmempty = false

e = t:option(Value, "speed_limit_per_con", translate("Speed Limit Per Con"),
             translate(
                 "Single thread speed limit upper limit, multithreading is invalid. Zero means no speed limit. (unit: KB/S)"))
e.default = "0"
e.rmempty = false

e = t:option(Value, "speed_limit_per_user", translate("Speed Limit Per User"),
             translate(
                 "Total speed limit upper limit, single port overall speed limit. Zero means no speed limit. (unit: KB/S)"))
e.default = "0"
e.rmempty = false

e = t:option(Value, "forbidden_port", translate("Forbidden Port"), translate(
                 "For example, if port 25 is not allowed, the user will not be able to access the mail port 25 through the SSR agent. If 80,443 is disabled, the user will not be able to access the HTTP/HTTPS website normally. <br>blocked single port format: 25<br>blocked multiple port format: 23,465<br>blocked port format: 233-266<br>blocked multiple port format: 25,465,233-666"))

e = t:option(Value, "transfer_enable", translate("Available Total Flow"),
             translate(
                 "Maximum amount of total traffic available (GB, 1-838868), Zero means infinite."))
e.default = "0"
e.rmempty = false

return a
