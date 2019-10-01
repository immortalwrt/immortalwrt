local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require "luci.model.uci".cursor()
local m, s, o, k
local clash = "clash"
local http = luci.http


m = Map("clash")
s = m:section(TypedSection, "clash" , translate("Rule"))
s.anonymous = true
s.addremove=false


o = s:option(Value, "rule_url")
o.title = translate("Custom Rule Url")
o.description = translate("Insert your custom rule Url and click download")
o.rmempty = true

o = s:option(Button,"rule_update")
o.title = translate("Download Rule")
o.inputtitle = translate("Download Rule")
o.description = translate("Download Rule")
o.inputstyle = "reload"
o.write = function()
  uci:commit("clash")
  SYS.call("sh /usr/share/clash/rule.sh >>/tmp/clash.log 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash", "servers"))
end

local rule = "/usr/share/clash/custom_rule.yaml"
sev = s:option(TextValue, "rule")
sev.description = translate("NB: Attention to Proxy Group and Rule when making changes to this section")
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(rule) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(rule, value:gsub("\r\n", "\n"))
end

o = s:option(Button,"del_rule")
o.inputtitle = translate("Delete Rule")
o.write = function()
  SYS.call("rm -rf /usr/share/clash/custom_rule.yaml >/dev/null 2>&1 &")
end




kr = Map(clash)
s = kr:section(TypedSection, "clash", translate("Subscription Config"))
s.anonymous = true

y = s:option(Flag, "cus_servers", translate("Status"))
y.description = translate("enabled to create custom configuration")
y.default = 0
y.rmempty = false

o = s:option(ListValue, "loadservers", translate("Load Config From"))
o.default = sub
o:value("sub", translate("Subscription Config"))
o:value("upl", translate("Upload Config"))
o.description = translate("Select from which configuration custom server should be loaded from")


o = s:option(Flag, "auto_update", translate("Auto Update"))
o.description = translate("Auto Update Server subscription")

o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.description = translate("Daily Server subscription update time")



o = s:option(ListValue, "subcri", translate("Subcription Type"))
o.default = clash
o:value("clash", translate("clash"))
o:value("v2rayn2clash", translate("v2rayn2clash"))
--o:value("surge2clash", translate("surge2clash"))
o.description = translate("Select Subcription Type, enter only your subcription url without https://tgbot.lbyczf.com/*?")



md = s:option(Flag, "cusrule", translate("Enabled Custom Rule"))
md.default = 1
md.description = translate("Enabled Custom Rule")
md:depends("subcri", 'v2rayn2clash')


o = s:option(Value, "subscribe_url")
o.title = translate("Subcription Url")
o.description = translate("Server Subscription Address")
o.rmempty = true

o = s:option(Button,"update")
o.title = translate("Update Subcription")
o.inputtitle = translate("Update")
o.description = translate("Update Config")
o.inputstyle = "reload"
o.write = function()
  --os.execute("sed -i '/enable/d' /etc/config/clash")
  --SYS.call("rm -rf /tmp/clash.log >/dev/null 2>&1 &")
  SYS.call("sh /usr/share/clash/clash.sh >>/tmp/clash.log 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash", "servers"))
end




k = Map(clash)
k.reset = false
k.submit = false
sul =k:section(TypedSection, "clash", translate("Upload Config"))
sul.anonymous = true
sul.addremove=false
o = sul:option(FileUpload, "")
o.title = translate("  ")
o.template = "clash/clash_upload"
um = sul:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd
dir = "/usr/share/clash/config/upload/"
http.setfilehandler(
	function(meta, chunk, eof)
		if not fd then
			if not meta then return end

			if	meta and chunk then fd = nixio.open(dir .. meta.file, "w") end

			if not fd then
				um.value = translate("upload file error.")
				return
			end
		end
		if chunk and fd then
			fd:write(chunk)
		end
		if eof and fd then
			fd:close()
			fd = nil
			local clash_conf = "/usr/share/clash/config/upload/config.yml"
			if NXFS.access(clash_conf) then
				  SYS.call("mv /usr/share/clash/config/upload/config.yml /usr/share/clash/config/upload/config.yaml >/dev/null 2>&1 &")
			end
			
			um.value = translate("File saved to") .. ' "/usr/share/clash/config/upload/"'
			
		end
	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end








local t = {
    {Load_Config, Delete_Severs, Delete_Groups}
}
b = k:section(Table, t)

o = b:option(Button,"Load_Config")
o.inputtitle = translate("Load Servers")
o.inputstyle = "apply"
o.write = function()
  k.uci:delete_all("clash", "servers", function(s) return true end)
  k.uci:commit("clash")
  luci.sys.call("sh /usr/share/clash/get_proxy.sh 2>/dev/null &")
  SYS.call("sleep 2")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "servers"))
end

o = b:option(Button,"Delete_Severs")
o.inputtitle = translate("Delete Severs")
o.inputstyle = "reset"
o.write = function()
  k.uci:delete_all("clash", "servers", function(s) return true end)
  k.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "servers"))
end

o = b:option(Button,"Delete_Groups")
o.inputtitle = translate("Delete Groups")
o.inputstyle = "reset"
o.write = function()
  k.uci:delete_all("clash", "groups", function(s) return true end)
  k.uci:commit("clash")
  luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash", "servers"))
end



s = k:section(TypedSection, "servers", translate("Proxies"))
s.anonymous = true
s.addremove = true
s.sortable = true
s.template = "cbi/tblsection"
s.extedit = luci.dispatcher.build_url("admin/services/clash/servers-config/%s")
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

o = s:option(DummyValue, "server" ,translate("Latency"))
o.template="clash/ping"
o.width="10%"

r = k:section(TypedSection, "groups", translate("Policy Groups"))
r.anonymous = true
r.addremove = true
r.sortable = true
r.template = "clash/tblsection"
r.extedit = luci.dispatcher.build_url("admin/services/clash/groups/%s")
function r.create(...)
	local sid = TypedSection.create(...)
	if sid then
		luci.http.redirect(r.extedit % sid)
		return
	end
end

o = r:option(DummyValue, "type", translate("Group Type"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


o = r:option(DummyValue, "name", translate("Group Name"))
function o.cfgvalue(...)
	return Value.cfgvalue(...) or translate("None")
end


k:append(Template("clash/list"))

local apply = luci.http.formvalue("cbi.apply")
if apply then
	uci:commit("clash")
	SYS.call("sleep 1")
	SYS.call("sh /usr/share/clash/proxy.sh >/dev/null 2>&1 &")
end

return kr, k, m
