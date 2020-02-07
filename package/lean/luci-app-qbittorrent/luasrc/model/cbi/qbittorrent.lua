local e=require"luci.model.uci".cursor()
local o=e:get_first("qbittorrent","qbittorrent","port") or 8080

local a=(luci.sys.call("pidof qbittorrent-nox > /dev/null")==0)

local t=""
if a then
	t="<br/><br/><input type=\"button\" value=\" " .. translate("Open Web Interface") .. " \" onclick=\"window.open('http://'+window.location.hostname+':" .. o .. "')\"/>"
end

m = Map("qbittorrent", translate("qBittorrent"), translate("qBittorrent is a cross-platform free and open-source BitTorrent client.").."<br/>"..translate("Default login username: admin, password: adminadmin.")..t)

m:section(SimpleSection).template="qbittorrent/qbittorrent_status"

s = m:section(TypedSection, "qbittorrent")
s.anonymous=true

enable = s:option(Flag, "enable", translate("Enable"))
enable.rmempty = false

port = s:option(Value,"port",translate("Port"),translate("WebUI listening port"))
port.default = "8080"
port.placeholder = "8080"
port.rmempty = false

profile_dir = s:option(Value,"profile_dir",translate("Profile Dir"),translate("Store configuration files in the Path"))
profile_dir.default = "/etc"
profile_dir.placeholder = "/etc"
profile_dir.rmempty = false

download_dir = s:option(Value,"download_dir",translate("Download Dir"),translate("Store download files in the Path"))
download_dir.default = "/mnt/download"
download_dir.placeholder = "/mnt/download"
download_dir.rmempty = false

return m
