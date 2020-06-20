local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require "luci.model.uci".cursor()
local s, o, krk, z, r
local clash = "clash"
local http = luci.http


krk = Map(clash)
s = krk:section(TypedSection, "clash", translate("Provider Config"))
s.anonymous = true
krk.pageaction = false

o = s:option(Flag, "provider_config", translate("Enable Create"))
o.default = 1
o.description = translate("Enable to create configuration")

o = s:option(Flag, "ppro", translate("Use Proxy Provider"))
o.description = translate("Use Proxy Provider")

o = s:option(Flag, "rulprp", translate("Use Rule Provider"))
o.description = translate("Use Rule Provider")

o = s:option(Flag, "rul", translate("Use Rules"))
o.description = translate("Use Rules")

o = s:option(Flag, "script", translate("Use Script"))
o.description = translate("Use Script")


o = s:option(Value, "name_tag")
o.title = translate("Config Name")
o.rmempty = true
o.description = translate("Give a name for your config")



local t = {
    {Creat_Config, Apply, Delete_Groups, Delete_ProxyPro, Delete_RulePro,Delete_Rules}
}

b = krk:section(Table, t)


o = b:option(Button,"Creat_Config")
o.inputtitle = translate("Create Config")
o.inputstyle = "apply"
o.write = function()
  krk.uci:commit("clash")
  luci.sys.call("bash /usr/share/clash/provider/provider.sh >/dev/null 2>&1 &")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
end

o = b:option(Button,"Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  krk.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end

o = b:option(Button,"Delete_ProxyPro")
o.inputtitle = translate("Delete Proxy Provider")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "proxyprovider", function(s) return true end)
  krk.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end

o = b:option(Button,"Delete_RulePro")
o.inputtitle = translate("Delete Rule Provider")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "ruleprovider", function(s) return true end)
  krk.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end


o = b:option(Button,"Delete_Rules")
o.inputtitle = translate("Delete Rules")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "rules", function(s) return true end)
  krk.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end

o = b:option(Button,"Delete_Groups")
o.inputtitle = translate("Delete Groups")
o.inputstyle = "reset"
o.write = function()
  krk.uci:delete_all("clash", "pgroups", function(s) return true end)
  krk.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end

-- [[ Groups Manage ]]--
x = krk:section(TypedSection, "pgroups", translate("Policy Groups"))
x.anonymous = true
x.addremove = true
x.sortable = true
x.template = "cbi/tblsection"
x.extedit = luci.dispatcher.build_url("admin/services/clash/pgroups/%s")
function x.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(x.extedit % sid)
		return
	end
end

o = x:option(DummyValue, "type", translate("Group Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


o = x:option(DummyValue, "name", translate("Group Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


-- [[ Proxy-Provider Manage ]]--
z = krk:section(TypedSection, "proxyprovider", translate("Proxy Provider"))
z.anonymous = true
z.addremove = true
z.sortable = true
z.template = "cbi/tblsection"
z.extedit = luci.dispatcher.build_url("admin/services/clash/proxyprovider/%s")
function z.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(z.extedit % sid)
		return
	end
end


o = z:option(DummyValue, "name", translate("Provider Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = z:option(DummyValue, "type", translate("Provider Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end





-- [[ Rule-Provider Manage ]]--
r = krk:section(TypedSection, "ruleprovider", translate("Rule Provider"))
r.anonymous = true
r.addremove = true
r.sortable = true
r.template = "cbi/tblsection"
r.extedit = luci.dispatcher.build_url("admin/services/clash/ruleprovider/%s")
function r.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(r.extedit % sid)
		return
	end
end


o = r:option(DummyValue, "name", translate("Provider Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = r:option(DummyValue, "type", translate("Provider Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = r:option(DummyValue, "behavior", translate("Provider Behavior"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


-- [[ Rule Manage ]]--
q = krk:section(TypedSection, "rules", translate("Rules"))
q.anonymous = true
q.addremove = true
q.sortable = true
q.template = "cbi/tblsection"
q.extedit = luci.dispatcher.build_url("admin/services/clash/rules/%s")
function q.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(q.extedit % sid)
		return
	end
end

o = q:option(DummyValue, "type", translate("Rule Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end

o = q:option(DummyValue, "rulename", translate("Description"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("-")
end


o = q:option(DummyValue, "rulegroups", translate("Groups"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


m = Map("clash")
y = m:section(TypedSection, "clash" , translate("Script"))
y.anonymous = true
y.addremove=false
m.pageaction = false

local script="/usr/share/clash/provider/script.yaml"
sev = y:option(TextValue, "scriptt")
sev.description = translate("NB: Set Clash Mode to Script if want to use")
sev.rows = 10
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(script) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(script, value:gsub("\r\n", "\n"))
end

o = y:option(Button,"Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "providers"))
end

return krk,m

