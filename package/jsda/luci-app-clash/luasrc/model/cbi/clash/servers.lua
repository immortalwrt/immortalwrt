
local clash = "clash"
local SYS  = require "luci.sys"
local uci = luci.model.uci.cursor()

k = Map(clash)
--k.pageaction = false
s = k:section(TypedSection, "clash")
s.anonymous = true

y = s:option(ListValue, "enable_servers", translate("Enable Create Config"))
y.default = 0
y:value("0", translate("disabled"))
y:value("1", translate("enabled"))
y.description = translate("Enable to create custom config.yaml. Note that this will overide any configuration you already have")


o = s:option(Button,"Delete_Severs")
o.title = translate("Delete Severs")
o.inputtitle = translate("Delete Severs")
o.description = translate("Perform this action to delete all servers")
o.inputstyle = "reset"
o.write = function()
  uci:delete_all("clash", "servers", function(s) return true end)
  luci.sys.call("uci commit clash") 
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "servers"))
end

o = s:option(Button,"Create_Severs")
o.title = translate("Create Config")
o.inputtitle = translate("Create Config")
o.description = translate("Perform this action to generate new configuration")
o:depends("enable_servers", "1")
o.inputstyle = "apply"
o.write = function()
  uci:set("clash", "enable_servers", "enable", 1)
  luci.sys.call("uci commit clash") 
  SYS.call("sh /usr/share/clash/proxy.sh 2>&1 &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "servers"))
end



s = k:section(TypedSection, "servers")
s.anonymous = true
s.addremove = true
s.sortable = false
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/clash/servers/%s")
function s.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(s.extedit % sid)
		return
	end
end

o = s:option(DummyValue, "type", translate("Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "name", translate("Alias"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "server", translate("Server Address"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = s:option(DummyValue, "port", translate("Server Port"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


return k
