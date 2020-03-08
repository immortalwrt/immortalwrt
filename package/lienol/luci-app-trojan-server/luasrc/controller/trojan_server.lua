-- Copyright 2019 Lienol <lawlienol@gmail.com>
module("luci.controller.trojan_server", package.seeall)
local http = require "luci.http"

function index()
    if not nixio.fs.access("/etc/config/trojan_server") then return end
    entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
    entry({"admin", "vpn", "trojan_server"}, cbi("trojan_server/index"),
          _("Trojan Server"), 3).dependent = true
    entry({"admin", "vpn", "trojan_server", "config"},
          cbi("trojan_server/config")).leaf = true

    entry({"admin", "vpn", "trojan_server", "users_status"},
          call("trojan_users_status")).leaf = true
    entry({"admin", "vpn", "trojan_server", "get_log"}, call("get_log")).leaf =
        true
    entry({"admin", "vpn", "trojan_server", "clear_log"}, call("clear_log")).leaf =
        true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

function trojan_users_status()
    local e = {}
    e.index = luci.http.formvalue("index")
    e.status = luci.sys.call(
                   "ps -w| grep -v grep | grep '/var/etc/trojan_server/" ..
                       luci.http.formvalue("id") .. "' >/dev/null") == 0
    http_write_json(e)
end

function get_log()
    luci.http.write(luci.sys.exec(
                        "[ -f '/var/log/trojan_server/app.log' ] && cat /var/log/trojan_server/app.log"))
end

function clear_log() luci.sys.call("echo '' > /var/log/trojan_server/app.log") end

