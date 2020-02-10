
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local uci = luci.model.uci.cursor()
local fs = require "luci.clash"
local http = luci.http
local clash = "clash"

kk = Map(clash)
r = kk:section(TypedSection, "clash", translate("Auto Update Config"))
r.anonymous = true
kk.pageaction = false

o = r:option(Flag, "auto_update", translate("Auto Update"))
o.description = translate("Auto Update Server subscription")

o = r:option(ListValue, "auto_update_time", translate("Update time (every day)"))
for t = 0,23 do
o:value(t, t..":00")
end
o.default=0
o.description = translate("Daily Server subscription update time. Only update config in use")

o = r:option(Button, "Apply")
o.title = translate("Save & Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  kk.uci:commit("clash")
end


kr = Map(clash)
s = kr:section(TypedSection, "clash", translate("Subscription Config"))
s.anonymous = true
kr.pageaction = false

o = s:option(ListValue, "subcri", translate("Subcription Type"))
o.default = clash
o:value("clash", translate("clash"))
o:value("v2ssr2clash", translate("v2ssr2clash"))
o.description = translate("Select Subcription Type")

o = s:option(Value, "config_name")
o.title = translate("Config Name")
o.description = translate("Config Name. Do not use a config name that already exist")

o = s:option(Value, "subscribe_url_clash")
o.title = translate("Subcription Url")
o.description = translate("Clash Subscription Address")
o.rmempty = true
o:depends("subcri", 'clash')


o = s:option(Button,"update")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  SYS.call("sh /usr/share/clash/clash.sh >>/usr/share/clash/clash.txt 2>&1 &")
  SYS.call("sleep 1")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'clash')

o = s:option(Value, "subscribe_url")
o.title = translate("Subcription Url")
o.description = translate("V2/SSR Subscription Address, Only input your subscription address without any api conversion url")
o.rmempty = true
o:depends("subcri", 'v2ssr2clash')


o = s:option(Button,"updatee")
o.title = translate("Download Config")
o.inputtitle = translate("Download Config")
o.inputstyle = "reload"
o.write = function()
  kr.uci:commit("clash")
  SYS.call("cp /etc/config/clash /usr/share/clash/v2ssr/config.bak 2>/dev/null")
  SYS.call("sleep 1")
  luci.sys.call("bash /usr/share/clash/v2ssr.sh >>/usr/share/clash/clash.txt 2>&1 &")
  HTTP.redirect(DISP.build_url("admin", "services", "clash"))
end
o:depends("subcri", 'v2ssr2clash')





ko = Map(clash)
ko.reset = false
ko.submit = false
sul =ko:section(TypedSection, "clash", translate("Upload Config"))
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


return kr,kk,ko
