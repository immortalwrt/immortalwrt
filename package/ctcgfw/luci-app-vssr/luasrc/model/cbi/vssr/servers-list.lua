-- Licensed to the public under the GNU General Public License v3.
local d = require "luci.dispatcher"
local fs = require "nixio.fs"
local sys = require "luci.sys"
local uci = require "luci.model.uci".cursor()
local m, s, o
local vssr = "vssr"


local uci = luci.model.uci.cursor()
local server_count = 0
uci:foreach("vssr", "servers", function(s)
  server_count = server_count + 1
end)



m = Map(vssr,  translate("Node List"))
m:section(SimpleSection).template  = "vssr/status1"


-- [[ Servers Manage ]]--
s = m:section(TypedSection, "servers")
s.anonymous = true
s.addremove = true
s.description = string.format(translate("Server Count") ..  ": %d", server_count)
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/vpn/vssr/servers/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end
o = s:option(DummyValue, "alias", translate("Alias"))
o.width="10%"
o = s:option(DummyValue, "type", translate("Type"))
o.width="15%"

o = s:option(DummyValue, "server", translate("Server Address"))
o.width="10%"

o = s:option(DummyValue, "server_port", translate("Server Port"))
o.width="10%"

o = s:option(DummyValue, "encrypt_method", translate("Encrypt Method"))
o.width="10%"

o = s:option(DummyValue, "protocol", translate("Protocol"))
o.width="10%"
o = s:option(DummyValue, "obfs", translate("Obfs"))
o.width="10%"

o = s:option(Flag, "switch_enable", translate("Enable Auto Switch"))
o.width="10%"

if nixio.fs.access("/usr/bin/kcptun-client") then

o = s:option(Flag, "kcp_enable", translate("KcpTun"))
o.width="10%"
end

o = s:option(DummyValue, "server_port", translate("Socket Connected"))
o.template="vssr/socket"
o.width="10%"

o = s:option(DummyValue,"server",translate("Ping Latency"))
o.template="vssr/ping"
o.width="10%"

m:append(Template("vssr/server_list"))


return m
