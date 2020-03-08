-- Copyright 2019 Lienol <lawlienol@gmail.com>
module("luci.controller.brook_server", package.seeall)
local http = require "luci.http"
local brook = require "luci.model.cbi.brook_server.api.brook"

function index()
    if not nixio.fs.access("/etc/config/brook_server") then return end
    entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
    entry({"admin", "vpn", "brook_server"}, cbi("brook_server/index"),
          _("Brook Server"), 3).dependent = true
    entry({"admin", "vpn", "brook_server", "config"}, cbi("brook_server/config")).leaf =
        true

    entry({"admin", "vpn", "brook_server", "users_status"},
          call("brook_users_status")).leaf = true
    entry({"admin", "vpn", "brook_server", "check"}, call("brook_check")).leaf =
        true
    entry({"admin", "vpn", "brook_server", "update"}, call("brook_update")).leaf =
        true
    entry({"admin", "vpn", "brook_server", "get_log"}, call("get_log")).leaf =
        true
    entry({"admin", "vpn", "brook_server", "clear_log"}, call("clear_log")).leaf =
        true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

function brook_users_status()
    local e = {}
    local index = luci.http.formvalue("index")
    e.index = index
    local protocol = luci.sys.exec("echo -n `uci get brook_server.@user[" ..
                                       index .. "].protocol`")
    local port = luci.sys.exec(
                     "echo -n `uci get brook_server.@user[" .. index ..
                         "].port`")

    local password = luci.sys.exec("echo -n `uci get brook_server.@user[" ..
                                       index .. "].password`")
    e.status = luci.sys.call(
                   "ps -w | grep -v grep | grep 'brook " .. protocol .. " -l :" ..
                       port .. " -p " .. password .. "' >/dev/null") == 0
    http_write_json(e)
end

function brook_check()
    local json = brook.to_check("")
    http_write_json(json)
end

function brook_update()
    local json = nil
    local task = http.formvalue("task")
    if task == "move" then
        json = brook.to_move(http.formvalue("file"))
    else
        json = brook.to_download(http.formvalue("url"))
    end

    http_write_json(json)
end

function get_log()
    luci.http.write(luci.sys.exec(
                        "[ -f '/var/log/brook_server/app.log' ] && cat /var/log/brook_server/app.log"))
end

function clear_log() luci.sys.call("echo '' > /var/log/brook_server/app.log") end

