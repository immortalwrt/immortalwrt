
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local fs = require "luci.clash"
local http = luci.http
local clash = "clash"


kr = Map(clash)
s = kr:section(TypedSection, "clash", translate("Subscription Config"))
s.anonymous = true
kr.pageaction = false

o = s:option(Flag, "auto_update", translate("Auto Update"))
o.description = translate("Auto Update Server subscription")

o = s:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.description = translate("Daily Server subscription update time. Only update config in use")



o = s:option(ListValue, "subcri", translate("Subcription Type"))
o.default = clash
o:value("clash", translate("clash"))
o:value("v2ssr2clash", translate("v2ssr2clash"))
o.description = translate("Select Subcription Type")

o = s:option(Value, "config_name")
o.title = translate("Config Name")
o.description = translate("Give a name for your config")

o = s:option(Value, "subscribe_url_clash")
o.title = translate("Subcription Url")
o.description = translate("Clash Subscription Address")
o.rmempty = true
o:depends("subcri", 'clash')

o = s:option(Value, "subscribe_url")
o.title = translate("Subcription Url")
o.description = translate("V2/SSR Subscription Address, Only input your subscription address without any api conversion url")
o.rmempty = true
o:depends("subcri", 'v2ssr2clash')

o = s:option(Button,"update")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  SYS.call("sh /usr/share/clash/clash.sh >>/tmp/clash.txt 2>&1 &")
  SYS.call("sleep 1")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'clash')

o = s:option(Button,"updatee")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  SYS.call("cp /etc/config/clash /usr/share/clash/v2ssr/config.bak 2>/dev/null")
  SYS.call("sleep 1")
  luci.sys.call("bash /usr/share/clash/v2ssr.sh >>/tmp/clash.txt 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'v2ssr2clash')


c = Map("clash")
c.template="clash/subconf"

m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false

local conf = string.sub(luci.sys.exec("uci get clash.config.config_path_sub"), 1, -2)
sev = s:option(TextValue, "conf")
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
sev.write = function(self, section, value)
	NXFS.writefile(conf, value:gsub("\r\n", "\n"))
end

o=s:option(Button,"apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "reload"
o.write = function()
  m.uci:commit("clash")
end

function IsYamlFile(e)
e=e or""
local e=string.lower(string.sub(e,-5,-1))
return e==".yaml"
end


local e,a={}
for t,o in ipairs(fs.glob("/usr/share/clash/config/sub/*"))do
a=fs.stat(o)
if a then
e[t]={}
e[t].name=fs.basename(o)
e[t].mtime=os.date("%Y-%m-%d %H:%M:%S",a.mtime)
e[t].size=tostring(a.size)
e[t].remove=0
e[t].enable=false
end
end

f=Form("config_list")
f.reset=false
f.submit=false
tb=f:section(Table,e)
nm=tb:option(DummyValue,"name",translate("File Name"))
mt=tb:option(DummyValue,"mtime",translate("Update Time"))
sz=tb:option(DummyValue,"size",translate("Size"))


btnis=tb:option(Button,"switch",translate("Use Config"))
btnis.template="clash/other_button"
btnis.render=function(o,t,a)
if not e[t]then return false end
if IsYamlFile(e[t].name)then
a.display=""
else
a.display="none"
end
o.inputstyle="apply"
Button.render(o,t,a)
end
btnis.write=function(a,t)
luci.sys.exec(string.format('uci set clash.config.config_path_sub="/usr/share/clash/config/sub/%s"',e[t].name ))
luci.sys.exec(string.format('uci set clash.config.config_update_name="%s"',e[t].name ))
luci.sys.exec('uci commit clash')
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "subconfig"))
end




btndl = tb:option(Button,"download",translate("Download")) 
btndl.template="clash/other_button"
btndl.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btndl.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/usr/share/clash/config/sub/"..e[t].name
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('yaml -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".yaml"
	else
		fd = nixio.open(sPath, "r")
	end
	if not fd then
		return
	end
	HTTP.header('Content-Disposition', 'attachment; filename="%s"' % {sFile})
	HTTP.prepare_content("application/octet-stream")
	while true do
		block = fd:read(nixio.const.buffersize)
		if (not block) or (#block ==0) then
			break
		else
			HTTP.write(block)
		end
	end
	fd:close()
	HTTP.close()
end


btnrm=tb:option(Button,"remove",translate("Remove"))
btnrm.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btnrm.write=function(a,t)
local a=fs.unlink("/usr/share/clash/config/sub/"..fs.basename(e[t].name))
luci.sys.exec(string.format('uci set clash.config.config_name_remove="%s"',e[t].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('bash /usr/share/clash/rmlist.sh 2>&1 &')
if a then table.remove(e,t)end
return a
end



return kr,c,f,m