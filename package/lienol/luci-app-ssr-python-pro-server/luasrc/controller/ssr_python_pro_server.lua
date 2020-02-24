-- Copyright 2018-2019 Lienol <lawlienol@gmail.com>
module("luci.controller.ssr_python_pro_server", package.seeall)
local http = require "luci.http"

function index()
    if not nixio.fs.access("/etc/config/ssr_python_pro_server") then return end
    entry({"admin", "vpn"}, firstchild(), "VPN", 45).dependent = false
    if nixio.fs.access("/usr/share/ssr_python_pro_server") then
        entry({"admin", "vpn", "ssr_python_pro_server"},
              cbi("ssr_python_pro_server/index"), _("SSR Python Server"), 2).dependent =
            true
    end

    entry({"admin", "vpn", "ssr_python_pro_server", "config"},
          cbi("ssr_python_pro_server/config")).leaf = true

    entry({"admin", "vpn", "ssr_python_pro_server", "status"},
          call("act_ssr_python_status")).leaf = true
    entry({"admin", "vpn", "ssr_python_pro_server", "users_status"},
          call("act_ssr_python_users_status")).leaf = true
    entry({"admin", "vpn", "ssr_python_pro_server", "get_total_traffic"},
          call("act_ssr_python_get_total_traffic")).leaf = true
    entry({"admin", "vpn", "ssr_python_pro_server", "get_link"},
          call("act_ssr_python_get_link")).leaf = true
    entry({"admin", "vpn", "ssr_python_pro_server", "clear_traffic"},
          call("act_ssr_python_clear_traffic")).leaf = true
    entry({"admin", "vpn", "ssr_python_pro_server", "clear_traffic_all_users"},
          call("act_ssr_python_clear_traffic_all_users")).leaf = true
end

local function http_write_json(content)
    http.prepare_content("application/json")
    http.write_json(content or {code = 1})
end

function act_ssr_python_status()
    local e = {}
    e.status = luci.sys.call(
                   "ps -w | grep -v grep | grep '/usr/share/ssr_python_pro_server/server.py' >/dev/null") ==
                   0
    http_write_json(e)
end

function act_ssr_python_users_status()
    local e = {}
    e.index = luci.http.formvalue("index")
    e.status = luci.sys.call("netstat -an | grep '" ..
                                 luci.http.formvalue("port") .. "' >/dev/null") ==
                   0
    http_write_json(e)
end

function act_ssr_python_get_total_traffic()
    local e = {}
    local result = nil
    local total_traffic_str = luci.sys.exec(
                                  "cd /usr/share/ssr_python_pro_server && ./mujson_mgr.py -l -I " ..
                                      luci.http.formvalue("section") ..
                                      " | sed -n 19p"):gsub("^%s*(.-)%s*$", "%1")
    local total_traffic = luci.sys.exec("echo " .. total_traffic_str ..
                                            " | awk '{print $3}'"):gsub(
                              "^%s*(.-)%s*$", "%1")
    if total_traffic == "" then total_traffic = 0 end
    local unit = luci.sys.exec("echo " .. total_traffic_str ..
                                   " | awk '{print $4}'"):gsub("^%s*(.-)%s*$",
                                                               "%1")
    result = string.format("%0.2f", total_traffic) .. unit
    e.result = result
    http_write_json(e)
end

function act_ssr_python_get_link()
    local e = {}
    local link = luci.sys.exec(
                     "cd /usr/share/ssr_python_pro_server && ./mujson_mgr.py -l -I " ..
                         luci.http.formvalue("section") .. " | sed -n 21p"):gsub(
                     "^%s*(.-)%s*$", "%1")
    if link ~= "" then e.link = link end
    http_write_json(e)
end

function act_ssr_python_clear_traffic()
    local e = {}
    e.status = luci.sys.call(
                   "cd /usr/share/ssr_python_pro_server && ./mujson_mgr.py -c -I '" ..
                       luci.http.formvalue("id") .. "' >/dev/null") == 0
    http_write_json(e)
end

function act_ssr_python_clear_traffic_all_users()
    local e = {}
    e.status = luci.sys.call(
                   "/usr/share/ssr_python_pro_server/sh/clear_traffic_all_users.sh >/dev/null") ==
                   0
    http_write_json(e)
end
