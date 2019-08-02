
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()

local http = luci.http

ful = Form("upload", nil)
ful.reset = false
ful.submit = false

sul =ful:section(SimpleSection, "", translate(""))
o = sul:option(FileUpload, "")
o.title = translate("Upload Config")
o.template = "clash/clash_upload"
o.description = translate("NB: Rename your config file to config.yaml before upload. file will save to /etc/clash")
um = sul:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd
dir = "/etc/clash/"
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
			um.value = translate("File saved to") .. ' "/etc/clash/' .. meta.file .. '"'
			os.execute("/etc/init.d/clash restart >/dev/null 2>&1 &")
		end
	end
)

if luci.http.formvalue("upload") then


	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end


m = Map("clash")
s = m:section(TypedSection, "clash")
s.anonymous = true
s.addremove=false


local conf = "/etc/clash/config.yaml"
sev = s:option(TextValue, "conf")
sev.readonly=true
--update_time = SYS.exec("ls -l --full-time /etc/clash/config.yaml|awk '{print $6,$7;}'")
--sev.description = update_time
--sev.description = translate("Changes to config file must be made from source")
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
  os.execute("rm -rf /etc/clash/config.yaml")
end


return ful , m
