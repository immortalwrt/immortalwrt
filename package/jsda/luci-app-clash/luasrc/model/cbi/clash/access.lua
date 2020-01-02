
local NXFS = require "nixio.fs"
local SYS  = require "luci.sys"
local HTTP = require "luci.http"
local DISP = require "luci.dispatcher"
local UTIL = require "luci.util"
local fs = require "luci.clash"
local uci = require "luci.model.uci".cursor()
local clash = "clash"
local http = luci.http


m = Map("clash")
s = m:section(TypedSection, "clash")
m.pageaction = false
s.anonymous = true
s.addremove=false

md = s:option(Flag, "proxylan", translate("Proxy Lan IP"))
md.default = 1
md.rmempty = false
md.description = translate("Only selected IPs will be proxied if enabled. fake-ip mode not supported")
md:depends("rejectlan", 0)


o = s:option(DynamicList, "lan_ac_ips", translate("Proxy Lan List"))
o.datatype = "ipaddr"
o.description = translate("Only selected IPs will be proxied")
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o:depends("proxylan", 1)



o = s:option(FileUpload, "",translate("Update GEOIP Database"))
o.description = translate("NB: Upload GEOIP Database file Country.mmdb")
.."<br />"
..translate("https://github.com/Dreamacro/maxmind-geoip/releases")
.."<br />"
..translate("https://static.clash.to/GeoIP2/GeoIP2-Country.tar.gz")

o.title = translate("Update GEOIP Database")
o.template = "clash/clash_upload"
um = s:option(DummyValue, "", nil)
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
			um.value = translate("File saved to") .. ' "/etc/clash/"'
			SYS.call("chmod + x /etc/clash/Country.mmdb")
			if luci.sys.call("pidof clash >/dev/null") == 0 then
			SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
			end
		end
	end
)

if luci.http.formvalue("upload") then
	local f = luci.http.formvalue("ulfile")
	if #f <= 0 then
		um.value = translate("No specify upload file.")
	end
end




md = s:option(Flag, "rejectlan", translate("Bypass Lan IP"))
md.default = 1
md.rmempty = false
md.description = translate("Selected IPs will not be proxied if enabled. fake-ip mode not supported")
md:depends("proxylan", 0)    


o = s:option(DynamicList, "lan_ips", translate("Bypass Lan List"))
o.datatype = "ipaddr"
o.description = translate("Selected IPs will not be proxied")
luci.ip.neighbors({ family = 4 }, function(entry)
       if entry.reachable then
               o:value(entry.dest:string())
       end
end)
o:depends("rejectlan", 1)





o = s:option(Button, "Apply")
o.title = translate("Save & Apply")
o.inputtitle = translate("Save & Apply")
o.inputstyle = "apply"
o.write = function()
  m.uci:commit("clash")
  if luci.sys.call("pidof clash >/dev/null") == 0 then
  SYS.call("/etc/init.d/clash restart >/dev/null 2>&1 &")
    luci.http.redirect(luci.dispatcher.build_url("admin", "services", "clash"))
  end
end

return m
