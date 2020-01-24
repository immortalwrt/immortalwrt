
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local fs = require "luci.clash"
local http = luci.http

c = Map("clash")
c.template="clash/cusconf"


m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false


local conf = string.sub(luci.sys.exec("uci get clash.config.config_path_cus"), 1, -2)
sev = s:option(TextValue, "conf")
--sev.readonly=true
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


local e,a={}
for t,o in ipairs(fs.glob("/usr/share/clash/config/custom/*"))do
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

function IsYamlFile(e)
e=e or""
local e=string.lower(string.sub(e,-5,-1))
return e==".yaml"
end

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
luci.sys.exec(string.format('uci set clash.config.config_path_cus="/usr/share/clash/config/custom/%s"',e[t].name ))
luci.sys.exec('uci commit clash')
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "cusconfig"))
end




btndl = tb:option(Button,"download",translate("Download")) 
btndl.template="clash/other_button"
btndl.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btndl.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/usr/share/clash/config/custom/"..e[t].name
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
local a=fs.unlink("/usr/share/clash/config/custom/"..fs.basename(e[t].name))
luci.sys.exec(string.format('uci set clash.config.config_cus_remove="%s"',e[t].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('/usr/share/clash/cuslist.sh 2>&1 &')

if a then table.remove(e,t)end
return a
end

return c,f,m
