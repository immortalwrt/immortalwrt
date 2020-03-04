-- Licensed to the public under the GNU General Public License v3.

local m, s, o
local vssr = "vssr"
local uci = luci.model.uci.cursor()
local server_table = {}
local gfwmode=0
local gfw_count=0
local ip_count=0

if nixio.fs.access("/etc/dnsmasq.ssr/gfw_list.conf") then
gfwmode=1		
end

local uci = luci.model.uci.cursor()
local server_count = 0
uci:foreach("vssr", "servers", function(s)
  server_count = server_count + 1
end)

local fs  = require "nixio.fs"
local sys = require "luci.sys"

if gfwmode==1 then 
 gfw_count = tonumber(sys.exec("cat /etc/dnsmasq.ssr/gfw_list.conf | wc -l"))/2

end


m = Map(vssr)
m:section(SimpleSection).template  = "vssr/status"
-- [[ 节点订阅 ]]--

s = m:section(TypedSection, "server_subscribe",  translate("Subscription"))
s.anonymous = true

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.rmempty = false
o.description = translate("Auto Update Server subscription, GFW list and CHN route")


o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
    o:value(t, t..":00")
end
o.default=2
o.rmempty = false

o = s:option(DynamicList, "subscribe_url", translate("Subscribe URL"))
o.rmempty = true

o = s:option(Flag, "proxy", translate("Through proxy update"))
o.rmempty = false
o.description = translate("Through proxy update list, Not Recommended ")


o = s:option(DummyValue, "", "")
o.rawhtml = true
o.template = "vssr/update_subscribe"

o = s:option(Button,"update",translate("Update All Subscribe Severs"),translate("No special needs, please click here to subscribe to update"))
o.inputstyle = "reload"
o.write = function()
  luci.sys.call("bash /usr/share/vssr/subscribe.sh >>/tmp/vssr.log 2>&1")
  luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "vssr", "servers"))
end


o = s:option(Button,"delete",translate("Delete All Subscribe Severs"))
o.inputstyle = "reset"
o.description = string.format(translate("Server Count") ..  ": %d", server_count)
o.write = function()
uci:delete_all("vssr", "servers", function(s) 
  if s["hashkey"] then
    return true
  else
    return false
  end
end)
uci:save("vssr")
luci.sys.call("uci commit vssr && /etc/init.d/vssr stop")
luci.http.redirect(luci.dispatcher.build_url("admin", "vpn", "vssr", "servers"))
return
end


m:section(SimpleSection).template  = "vssr/status2"

return m





