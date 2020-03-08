#-- Copyright (C) 2018 dz <dingzhong110@gmail.com>
-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local server_run=0
local bridge_run=0
local client_run=0

local softethervpn5 = "softethervpn5"
-- html constants
font_blue = [[<font color="blue">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]

local fs = require "nixio.fs"
local sys = require "luci.sys"

m = SimpleForm("Version")
m.reset = false
m.submit = false

if luci.sys.call("pidof vpnserver >/dev/null") == 0 then
server_run=1
end	

if luci.sys.call("pidof vpnbridge >/dev/null") == 0 then
bridge_run=1
end	

if luci.sys.call("pidof vpnclient >/dev/null") == 0 then
client_run=1
end	

s=m:field(DummyValue,"server_run",translate("VPN Server")) 
s.rawhtml  = true
if server_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"bridge_run",translate("VPN Bridge")) 
s.rawhtml  = true
if bridge_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

s=m:field(DummyValue,"client_run",translate("VPN Client")) 
s.rawhtml  = true
if client_run == 1 then
s.value =font_blue .. bold_on .. translate("Running") .. bold_off .. font_off
else
s.value = translate("Not Running")
end

return m
