local i = require "luci.dispatcher"
local e = require "nixio.fs"
local e = require "luci.sys"
local e = luci.model.uci.cursor()
local o = "brook_server"

m = Map(o, translate("Brook Server"))

t = m:section(TypedSection, "global", translate("Global Settings"))
t.anonymous = true
t.addremove = false

e = t:option(Flag, "enable", translate("Enable"))
e.rmempty = false
t:append(Template("brook_server/brook"))

e = t:option(Value, "brook_path", translate("Brook Path"),
             translate(
                 "if you want to run from memory, change the path, such as /tmp/brook, Then save the application and update it manually."))
e.default = "/usr/bin/brook"
e.rmempty = false

t = m:section(TypedSection, "user", translate("Users Manager"))
t.anonymous = true
t.addremove = true
t.template = "cbi/tblsection"
t.extedit = i.build_url("admin", "vpn", o, "config", "%s")
function t.create(t, e)
    local e = TypedSection.create(t, e)
    luci.http.redirect(i.build_url("admin", "vpn", o, "config", e))
end
function t.remove(t, a)
    t.map.proceed = true
    t.map:del(a)
    luci.http.redirect(i.build_url("admin", "vpn", o))
end
e = t:option(Flag, "enable", translate("Enable"))
e.width = "5%"
e.rmempty = false
e = t:option(DummyValue, "status", translate("Status"))
e.template = "brook_server/users_status"
e.value = translate("Collecting data...")
e = t:option(DummyValue, "remarks", translate("Remarks"))
e.width = "20%"
e = t:option(DummyValue, "port", translate("Port"))
e.width = "20%"

e = t:option(DummyValue, "password", translate("Password"))
e.width = "30%"
e.cfgvalue = function(self, section)
    local e = m:get(section, "password") or ""
    local t = ""
    if type(e) == "table" then
        for a = 1, #e do t = t .. e[a] .. "," end
        t = string.sub(t, 0, #t - 1)
    else
        t = e
    end
    return t
end

m:append(Template("brook_server/log"))

m:append(Template("brook_server/users_list_status"))
return m

