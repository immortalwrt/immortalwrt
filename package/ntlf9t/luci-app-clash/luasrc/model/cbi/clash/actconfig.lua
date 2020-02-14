
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local fs = require "luci.clash"
local http = luci.http




m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false


local conf = "/etc/clash/config.yaml"
sev = s:option(TextValue, "conf")
sev.readonly=true
sev.rows = 20
sev.wrap = "off"
sev.cfgvalue = function(self, section)
	return NXFS.readfile(conf) or ""
end
sev.write = function(self, section, value)
end


o = s:option(Button,"configrm")
o.inputtitle = translate("Delete Config")
o.write = function()
  os.execute("rm -rf /etc/clash/config.yaml 2>&1 &")
end

o = s:option(Button, "Download") 
o.inputtitle = translate("Download Config")
o.inputstyle = "apply"
o.write = function ()
	local sPath, sFile, fd, block
	sPath = "/etc/clash/config.yaml"
	sFile = NXFS.basename(sPath)
	if fs.isdirectory(sPath) then
		fd = io.popen('tar -C "%s" -cz .' % {sPath}, "r")
		sFile = sFile .. ".tar.gz"
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

return m
