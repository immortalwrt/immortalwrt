local http = luci.http
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require("luci.model.uci").cursor()
local clash = "clash"

k = Map(clash)
k.reset = false
k.submit = false
sul =k:section(TypedSection, "clash", translate("Upload Config"))
sul.anonymous = true
sul.addremove=false
o = sul:option(FileUpload, "")
o.description = translate("NB: Only upload file with name .yaml.It recommended to rename each upload file name to avoid overwrite")
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


c = Map("clash")
c.template="clash/upconf"


m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false


local conf = string.sub(luci.sys.exec("uci get clash.config.config_path_up"), 1, -2)
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
for t,o in ipairs(fs.glob("/usr/share/clash/config/upload/*.yaml"))do
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
luci.sys.exec(string.format('uci set clash.config.config_path_up="/usr/share/clash/config/upload/%s"',e[t].name ))
luci.sys.exec('uci commit clash')
HTTP.redirect(luci.dispatcher.build_url("admin", "services", "clash", "config", "upconfig"))
end




btndl = tb:option(Button,"download",translate("Download")) 
btndl.template="clash/other_button"
btndl.render=function(e,t,a)
e.inputstyle="remove"
Button.render(e,t,a)
end
btndl.write = function (a,t)
	local sPath, sFile, fd, block
	sPath = "/usr/share/clash/config/upload/"..e[t].name
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
local a=fs.unlink("/usr/share/clash/config/upload/"..fs.basename(e[t].name))
luci.sys.exec(string.format('uci set clash.config.config_up_remove="%s"',e[t].name))
luci.sys.exec('uci commit clash')
luci.sys.exec('bash /usr/share/clash/uplist.sh 2>&1 &')
if a then table.remove(e,t)end
return a
end

return k,c,f,m
