local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = require("luci.model.uci").cursor()
local m , r, k
local http = luci.http

font_green = [[<font color="green">]]
font_off = [[</font>]]
bold_on  = [[<strong>]]
bold_off = [[</strong>]]



ko = Map("clash")
ko.reset = false
ko.submit = false
sul =ko:section(TypedSection, "clash","")
sul.anonymous = true
sul.addremove=false
o = sul:option(FileUpload, "")
o.description = translate("NB: Upload only Dreamacro clash tun core - (https://github.com/Dreamacro/clash/releases/tag/TUN)")
o.title = translate("  ")
o.template = "clash/clash_upload"
um = sul:option(DummyValue, "", nil)
um.template = "clash/clash_dvalue"

local dir, fd
dir = "/etc/clash/dtun/"
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
			SYS.exec("chmod 755 /etc/clash/dtun/clash 2>&1 &")
			um.value = translate("File saved to") .. ' "/etc/clash/dtun/"'
			
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
m:section(SimpleSection).template  = "clash/update"
m.pageaction = false

k = Map("clash")
s = k:section(TypedSection, "clash")
s.anonymous = true
o = s:option(ListValue, "dcore", translate("Core Type"))
o.default = "clashcore"
o:value("1", translate("Clash"))
o:value("2", translate("Clashr"))
o:value("3", translate("Clash(ctun)"))
o.description = translate("Select core, clashr support ssr while clash does not.")


local cpu_model=SYS.exec("opkg status libc 2>/dev/null |grep 'Architecture' |awk -F ': ' '{print $2}' 2>/dev/null")
o = s:option(ListValue, "download_core", translate("Select Core"))
o.description = translate("CPU Model")..': '..font_green..bold_on..cpu_model..bold_off..font_off..' '
o:value("linux-386")
o:value("linux-amd64", translate("linux-amd64(x86-64)"))
o:value("linux-armv5")
o:value("linux-armv6")
o:value("linux-armv7")
o:value("linux-armv8")
o:value("linux-mips-hardfloat")
o:value("linux-mips-softfloat")
o:value("linux-mips64")
o:value("linux-mips64le")
o:value("linux-mipsle-softfloat")
o:value("linux-mipsle-hardfloat")


o=s:option(Button,"down_core")
o.inputtitle = translate("Save & Apply")
o.title = translate("Save & Apply")
o.inputstyle = "reload"
o.write = function()
  k.uci:commit("clash")
end

o = s:option(Button,"download")
o.title = translate("Download")
o.template = "clash/core_check"


return m, ko,k


